.. highlight:: c

Lua 脚本
===========

通过对 Redis 源码中的 ``scripting.c`` 文件进行分析，解释 Lua 脚本功能的实现机制。


预备知识
----------

因为脚本功能的实现源码和命令关系密切，最好在阅读这篇文章之前先了解 Redis 的脚本功能是如何使用的，否则你可能无法看明白这里的一些实现决策是如何做出的。

`EVAL 命令的文档 <http://redis.readthedocs.org/en/latest/script/eval.html>`_ 是学习使用脚本功能的一个很好起点。


脚本功能的实现
------------------

Redis 脚本功能的实现代码放在源码的 ``src/scripting.c`` 文件中，主要分为三部分：

1) Lua 嵌入 Redis

2) ``EVAL`` 和 ``EVALSHA`` 命令的实现

3) ``SCRIPT`` 命令的实现

以下是这三个部分的详细说明。


Lua 嵌入 Redis
-------------------

要在 Redis 中执行 Lua 脚本，必须先将 Lua 嵌入到 Redis 服务器端中，并且初始化 Lua 脚本的相关环境。

在 ``scripting.c`` 中，以上工作是由 ``scriptingInit`` 函数完成的：

::

    void scriptingInit(void) {
        // Lua 魔法从这里发生...
    }

``scriptingInit`` 的工作包括以下：


1. 创建新的 Lua 环境
^^^^^^^^^^^^^^^^^^^^^

::

    lua_State *lua = lua_open();

``lua_open`` 是一个 Lua 5.1 提供的 C API ，它用于创造一个新的 Lua 环境（environment/state）。


2. 载入函数包
^^^^^^^^^^^^^^

Redis 的 Lua 环境中提供了好几个常用的包，比如 ``base`` 、 ``table`` 、 ``math`` 和 ``cjson`` 等，这些包都在创建环境之后通过 ``luaLoadLibraries`` 函数载入：

::

    luaLoadLibraries(lua);


3. 移除不能暴露给 Redis 环境的 Lua 函数
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

为了避免安全问题，一些带有特殊作用的函数，比如 ``openfile`` ，是不能暴露给执行 Redis 命令的 Lua 环境的，因此需要从 Lua 环境中移除这些函数：

::

    luaRemoveUnsupportedFunctions(lua);


4. 创建字典，用于保存脚本缓存
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

被 ``EVAL`` 执行过的脚本，或者被 ``SCRIPT LOAD`` 命令载入过的脚本，都会被保存到 Redis 的服务器中，方便将来直接使用 ``EVALSHA`` 调用。

所有的这些脚本都被缓存到一个字典中，根据脚本的 SHA1 校验和来进行索引。

``dictCreate`` 创建了字典，并将它传给 ``server.lua_scripts`` 属性：

::

    server.lua_scripts = dictCreate(&dbDictType,NULL);


5. 创建并设置 redis table
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

在 Redis 的 Lua 环境中，所有对 Redis 的工作都是通过调用 ``redis.xxx`` 这样的函数来完成的。

比如执行在 Lua 中执行 Redis 命令，可以调用 ``redis.call`` 或者 ``redis.pcall`` 来完成。

又或者，可以使用 ``redis.log`` 来记录日志。

这些 Lua 函数都是通过以下语句来注册：

::

    /* Register the redis commands table and fields */
    lua_newtable(lua);

    /* redis.call */
    lua_pushstring(lua,"call");
    lua_pushcfunction(lua,luaRedisCallCommand);
    lua_settable(lua,-3);

    /* redis.pcall */
    lua_pushstring(lua,"pcall");
    lua_pushcfunction(lua,luaRedisPCallCommand);
    lua_settable(lua,-3);

    /* redis.log and log levels. */
    lua_pushstring(lua,"log");
    lua_pushcfunction(lua,luaLogCommand);
    lua_settable(lua,-3);

    lua_pushstring(lua,"LOG_DEBUG");
    lua_pushnumber(lua,REDIS_DEBUG);
    lua_settable(lua,-3);

    lua_pushstring(lua,"LOG_VERBOSE");
    lua_pushnumber(lua,REDIS_VERBOSE);
    lua_settable(lua,-3);

    lua_pushstring(lua,"LOG_NOTICE");
    lua_pushnumber(lua,REDIS_NOTICE);
    lua_settable(lua,-3);

    lua_pushstring(lua,"LOG_WARNING");
    lua_pushnumber(lua,REDIS_WARNING);
    lua_settable(lua,-3);

    /* redis.sha1hex */
    lua_pushstring(lua, "sha1hex");
    lua_pushcfunction(lua, luaRedisSha1hexCommand);
    lua_settable(lua, -3);

    /* Finally set the table as 'redis' global var. */
    lua_setglobal(lua,"redis");


6. 覆盖 math table 中的 random 和 randomseed 函数
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

为了创建无副作用的脚本，Redis 使用修改过的 ``random`` 函数和 ``randomseed`` 函数，覆盖了原有的 ``math`` 包中的 ``random`` 和 ``randomseed`` 函数：

::

    /* Replace math.random and math.randomseed with our implementations. */
    lua_getglobal(lua,"math");

    lua_pushstring(lua,"random");
    lua_pushcfunction(lua,redis_math_random);
    lua_settable(lua,-3);

    lua_pushstring(lua,"randomseed");
    lua_pushcfunction(lua,redis_math_randomseed);
    lua_settable(lua,-3);

    lua_setglobal(lua,"math");


7. 创建辅助函数，用于排序
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

一些 Redis 命令，比如 ``SMEMBERS`` 和 ``KEYS`` ，返回的结果集是无序的。

在脚本功能中，这些命令被称为 non deterministic 命令。

为了避免这些 non deterministic 命令产生副作用（返回值结果无序）， Redis 使用一个辅助函数，用于对 non deterministic 命令的结果集进行排序，从而确保返回值无副作用：

::

    /* Add a helper funciton that we use to sort the multi bulk output of non
     * deterministic commands, when containing 'false' elements. */
    {
        char *compare_func =    "function __redis__compare_helper(a,b)\n"
                                            "  if a == false then a = '' end\n"
                                            "  if b == false then b = '' end\n"
                                            "  return a<b\n"
                                            "end\n";
        luaL_loadbuffer(lua,compare_func,strlen(compare_func),"@cmp_func_def");
        lua_pcall(lua,0,0,0);
    }


8. 创建客户端
^^^^^^^^^^^^^^^^^^

Redis 会创建一个客户端，用于处理 Lua 中执行的 Redis 命令。

这个客户端无须链接（connect）到服务器，因为它本身已经运行在服务器上了：

::

    /* Create the (non connected) client that we use to execute Redis commands
     * inside the Lua interpreter.
     * Note: there is no need to create it again when this function is called
     * by scriptingReset(). */
    if (server.lua_client == NULL) {
        server.lua_client = createClient(-1);
        server.lua_client->flags |= REDIS_LUA_CLIENT;
    }

另外需要提醒的一点是， Redis 从始到终都只是创建了一个 Lua 环境，以及一个 ``lua_client`` ，这就是一个 Redis 服务器端只能处理一个脚本的原因。


9. 对全局变量进行保护，避免遭到有意或无意的覆盖
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    scriptingEnableGlobalsProtection(lua);


10. 将 Lua 环境设置给 Redis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    server.lua = lua;

完成以上 10 个步骤之后，一个完整的 Lua 环境就被创建并且设置好了。

接下来，可以开始研究 ``EVAL`` 和 ``EVALSHA`` 这两个命令的实现，看看它们是如何配合 Lua 环境，一起完成对 Lua 脚本进行求值的任务的。


EVAL 和 EVALSHA 命令的实现
----------------------------

``EVAL`` 和 ``EVALSHA`` 分别通过 ``evalCommand`` 和 ``evalShaCommand`` 函数实现，而这两个函数都由 ``evalGenericCommand`` 函数实际实现，只是接受的参数有所不同。

::

    void evalCommand(redisClient *c) {
        evalGenericCommand(c,0);    // evalsha 参数为 0
    }

    void evalShaCommand(redisClient *c) {
        // 如果传给 EVALSHA 的 SHA1 值长度不对
        // 那么直接返回 noscripterr 错误
        if (sdslen(c->argv[1]->ptr) != 40) {
        /* We know that a match is not possible if the provided SHA is
         * not the right length. So we return an error ASAP, this way
         * evalGenericCommand() can be implemented without string length
         * sanity check */
            addReply(c, shared.noscripterr);
            return;
        }
        evalGenericCommand(c,1);    // evalsha 参数为 1
    }

``evalGenericCommand`` 函数完成了对脚本进行求值的任务：

::

    void evalGenericCommand(redisClient *c, int evalsha) {
        // ...
    }

以下是这个函数的一些主要工作：


1. 初始化 FLAG
^^^^^^^^^^^^^^^^^^

Redis 不允许脚本功能在执行一个 non deterministic 命令之后再继续执行一个写入功能，
另外，为了让一个纯读取（read only）的脚本在不打扰一个写入脚本的情况下进行读取，提升并发性，
Redis 使用了两个 FLAG 变量，用于检查所执行命令的属性。

在后面的相关函数实现里，会看见这两个 FLAG 的应用。

::

    /* We set this flag to zero to remember that so far no random command
     * was called. This way we can allow the user to call commands like
     * SRANDMEMBER or RANDOMKEY from Lua scripts as far as no write command
     * is called (otherwise the replication and AOF would end with non
     * deterministic sequences).
     *
     * Thanks to this flag we'll raise an error every time a write command
     * is called after a random command was used. */
    // 初始化 FLAG
    server.lua_random_dirty = 0;
    server.lua_write_dirty = 0;


2. 生成函数名
^^^^^^^^^^^^^^^^^

在 Lua 环境中，所有的脚本都被定义为一个函数，而每个函数都是以 ``f_`` + 脚本 SHA1 校验和的格式存在的。

举个例子，脚本 ``return redis.call('get','foo')`` 的校验和为 ``6b1bf486c81ceb7edf3c093f4c48582e38c0e791`` ，当这个脚本通过 ``evalGenericCommand`` 函数执行的时候，这个脚本会被放进一个 Lua 函数的函数体内里，而这个函数的名字就叫做 ``f_6b1bf486c81ceb7edf3c093f4c48582e38c0e791`` ，就像执行以下 Lua 代码一样：

.. code-block:: lua

    function f_6b1bf486c81ceb7edf3c093f4c48582e38c0e791()
        return redis.call('get', 'foo')
    end

生成函数名的工作由以下代码完成：

::

    /* We obtain the script SHA1, then check if this function is already
    * defined into the Lua state */
    funcname[0] = 'f';
    funcname[1] = '_';

    // 如果被调用的命令是 EVAL ，那么根据脚本产生一个 SHA1 值
    if (!evalsha) {
        /* Hash the code if this is an EVAL call */
        sha1hex(funcname+2,c->argv[1]->ptr,sdslen(c->argv[1]->ptr));
    // 如果被调用的命令是 EVALSHA ，那么直接使用参数中的 SHA1 值
    } else {
        /* We already have the SHA if it is a EVALSHA */
        int j;
        char *sha = c->argv[1]->ptr;

        for (j = 0; j < 40; j++)
            funcname[j+2] = tolower(sha[j]);
        funcname[42] = '\0';
    }


3. 寻找/创建函数
^^^^^^^^^^^^^^^^^^^^^

有了函数名之后，就可以根据函数明来进行查找了。

因为函数名由每个脚本的 SHA1 值来决定，而每个脚本的 SHA1 值都是唯一的（好吧，不是唯一，只是碰撞的机会非常非常非常小），如果同样的一个脚本曾经被运行过，那么它就会在 Lua 环境中定义，我们可以直接使用这个函数。

另一方面，如果给定脚本从来没被运行过，那么我们就定义这个函数到 Lua 环境：

::

    /* Try to lookup the Lua function */
    // 根据函数名，在 Lua 环境中查找函数
    lua_getglobal(lua, funcname);

    // 如果函数没找到。。。
    if (lua_isnil(lua,1)) {
        lua_pop(lua,1); /* remove the nil from the stack */

        /* Function not defined... let's define it if we have the
         * body of the funciton. If this is an EVALSHA call we can just
         * return an error. */
        // 如果函数没找到且这是一个 EVALSHA 命令调用
        // 那么产生一个错误
        if (evalsha) {
            addReply(c, shared.noscripterr);
            return;
        }

        // 如果函数没找到且这是一个 EVAL 命令调用
        // 那么创建这个函数
        if (luaCreateFunction(c,lua,funcname,c->argv[1]) == REDIS_ERR) return;

        /* Now the following is guaranteed to return non nil */
        lua_getglobal(lua, funcname);
        redisAssert(!lua_isnil(lua,1));
    }


4. 运行函数
^^^^^^^^^^^^^

有了函数之后，是时候运行它了。

运行函数通过 Lua API ``lua_pcall`` 来完成。

如果函数的执行过程中没有错误发生，那么 ``lua_pcall`` 返回 ``0`` ，否则它返回不等于 ``0`` 的其他值：

::

    /* At this point whatever this script was never seen before or if it was
     * already defined, we can call it. We have zero arguments and expect
     * a single return value. */
    // 执行函数
    if (lua_pcall(lua,0,1,0)) {
        // 函数执行出错时的处理语句 ...
    }


5. 对返回值做类型转换
^^^^^^^^^^^^^^^^^^^^^^^^^

如果函数成功执行了，那么就会产生返回值。

这些返回值是一个或一些 Lua 类型的值，而我们要在 Redis 中返回它们，因此需要将这些返回值从 Lua 类型转换成 Redis 类型，这个工作由 ``luaReplyToRedisReply`` 完成：

::

    if (lua_pcall(lua,0,1,0)) {
        // 函数执行出错时的处理语句 ...
    }
    // 其他函数执行成功时的处理语句 ...
    luaReplyToRedisReply(c,lua);
    // 其他函数执行成功时的处理语句 ...

好的，到了这一步， ``evalGenericCommand`` 剩下的就是诸如释放内存和返回值这类收尾工作了，以上就是使用 ``EVAL`` 或 ``EVALSHA`` 对一个脚本进行求值的主要流程。


诶，等等，先等等！
----------------------

就在我兴高采烈准备结束 ``evalGenericCommand`` 这一部分的时候，我忽然发现自己漏掉了什么东西。。。

文章的前一部分讲到， ``evalGenericCommand`` 函数进行了对脚本进行求值的实际工作，但是，在对 Lua 脚本进行求值的时候，还有一种特殊情况要处理，那就是： Lua 脚本中可能带有 ``redis.call`` 调用或者 ``redis.pcall`` 调用，这两个调用会反过来从 Lua 脚本中转到 Redis 里进行求值。

比如命令 ``EVAL "return redis.call('set','foo','bar')" 0`` 就会先在 Redis 里执行 ``EVAL`` 命令，然后在 Lua 里执行 ``redis.call`` 调用，然后又在 Lua 里对 Redis 执行命令 ``SET`` ，在执行完 ``SET`` 命令之后，Redis 将返回值转换成 Lua 值并返回给 Lua ，然后 Lua 又用 ``return`` 关键字，将函数的结果返回给 Redis ，作为 ``EVAL`` 调用的返回值。

嗯，虽然这听上去很复杂，但我最好讲讲这个，因为 ``redis.call`` （或者 ``redis.pcall`` ）的调用非常关键，如果我不好好解释清楚的话，可能就会有人找我来退款，给我发恐吓信，砸烂我家的玻璃。。。我可不想遇到这种破事！

之前讲过，在 ``scriptingInit`` 函数执行时，它会将一些函数注册到 ``redis`` 表中，并设置为全局变量，其中两个就是 ``redis.call`` 和 ``redis.pcall`` ：

::

    void scriptingInit(void) {
        // 其他代码 ...

        /* redis.call */
        lua_pushstring(lua,"call");
        lua_pushcfunction(lua,luaRedisCallCommand);
        lua_settable(lua,-3);

        /* redis.pcall */
        lua_pushstring(lua,"pcall");
        lua_pushcfunction(lua,luaRedisPCallCommand);
        lua_settable(lua,-3);

        // 其他代码 ...
    }

当 ``evalGenericCommand`` 在对脚本进行求值时，如果脚本带有 ``redis.call`` 调用或者 ``redis.pcall`` 调用，Lua 就用相应的 ``luaRedisCallCommand`` 或者 ``luaRedisPCallCommand`` 来处理这些调用：

::

    int luaRedisCallCommand(lua_State *lua) {
        return luaRedisGenericCommand(lua,1);
    }

    int luaRedisPCallCommand(lua_State *lua) {
        return luaRedisGenericCommand(lua,0);
    }

而这两个函数都调用同一个函数 ``luaRedisGenericCommand`` 。

``luaRedisGenericCommand`` 函数的定义比较长，这里不打算一段段地分析它的行为了，有兴趣的可以直接去看带注释的源码，以下是这个函数的大致工作：

1) 声明一些 Redis Object 实例，用于处理所执行的 Redis 命令

2) 对 ``call`` 或者 ``pcall`` 函数的参数进行检查，确保参数正确

3) 对调用的 Redis 命令进行检查，确保是可以在脚本环境中运行的命令（一些命令，比如 ``WATCH`` ，就无法在脚本环境中执行）

4) 如果要执行的 Redis 是写入型的（比如 ``SET`` ），那么检查它的执行环境是否合法

5) 查看内存是否足够运行指定命令，如果内存不够，尝试释放内存

6) 按需设置 ``server.lua_random_dirty`` 或 ``server.lua_write_dirty`` 这两个 FLAG

7) 执行命令

8) 取出命令的返回值，使用 ``redisProtocolToLuaType`` 函数将这些返回值从 Redis 类型转换成 Lua 类型

9) 如果执行的命令是 non deterministic 型的，那么对返回值进行一次排序

10) 清理环境

好的，以上就是 ``luaRedisGenericCommand`` 的执行过程了。


SCRIPT 命令的实现
-------------------

最后要说的就是 ``SCRIPT`` 命令了。

前文里说过，Redis 将一些执行过的 Lua 脚本以 ``f_`` + 脚本 SHA1 校验和的命名方式作为函数，保存在 ``server.lua_scripts`` 属性里，而 ``SCRIPT`` 的主要功能，就是对 ``server.lua_scripts`` 进行读写或删除等操作。

以下是 ``SCRIPT`` 命令的完整实现：

::

    void scriptCommand(redisClient *c) {
        if (c->argc == 2 && !strcasecmp(c->argv[1]->ptr,"flush")) {
            // 重置脚本环境
            scriptingReset();
            addReply(c,shared.ok);
            server.dirty++; /* Replicating this command is a good idea. */
        } else if (c->argc >= 2 && !strcasecmp(c->argv[1]->ptr,"exists")) {
            int j;

            // 遍历 server.lua_scripts 属性
            // 检查给定的一个或多个脚本是否存在
            addReplyMultiBulkLen(c, c->argc-2);
            for (j = 2; j < c->argc; j++) {
                if (dictFind(server.lua_scripts,c->argv[j]->ptr))
                    addReply(c,shared.cone);
                else
                    addReply(c,shared.czero);
            }
        } else if (c->argc == 3 && !strcasecmp(c->argv[1]->ptr,"load")) {
            char funcname[43];
            sds sha;

            // 根据给定脚本，生成 Lua 函数，并将它载入到 server.lua_scripts 当中
            funcname[0] = 'f';
            funcname[1] = '_';
            sha1hex(funcname+2,c->argv[2]->ptr,sdslen(c->argv[2]->ptr));
            sha = sdsnewlen(funcname+2,40);
            if (dictFind(server.lua_scripts,sha) == NULL) {
                if (luaCreateFunction(c,server.lua,funcname,c->argv[2])
                        == REDIS_ERR) {
                    sdsfree(sha);
                    return;
                }
            }
            addReplyBulkCBuffer(c,funcname+2,40);
            sdsfree(sha);
        } else if (c->argc == 2 && !strcasecmp(c->argv[1]->ptr,"kill")) {
            // 终止指定脚本的运行
            // 关于终止方式的细节，可以看 evalGenericCommand 函数是如何处理的
            if (server.lua_caller == NULL) {
                addReplyError(c,"No scripts in execution right now.");
            } else if (server.lua_write_dirty) {
                addReplyError(c, "Sorry the script already executed write commands against the dataset. You can either wait the script termination or kill the server in an hard way using the SHUTDOWN NOSAVE command.");
            } else {
                server.lua_kill = 1;
                addReply(c,shared.ok);
            }
        } else {
            addReplyError(c, "Unknown SCRIPT subcommand or wrong # of args.");
        }
    }


搞定！
--------

以上就是 Redis 的 Lua 脚本功能的实现分析了，因为篇幅的原因，这里主要介绍了几个主要函数的最重要的执行部分，一些错误处理或者其他细节功能的实现就被忽略掉了，我将带有完整注释的 ``scripting.c`` 放到了 GitHub 上，想了解所有细节的朋友可以到上面看看： `https://github.com/huangz1990/reading_redis_source <https://github.com/huangz1990/reading_redis_source>`_ 。
