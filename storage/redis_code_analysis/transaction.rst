.. highlight:: c

事务
=======

通过对 Redis 源码中的 ``multi.c`` 文件进行分析，解释 Redis 事务（transaction）功能的实现原理。


Redis 的事务
--------------

在开始研究 ``multi.c`` 的源码之前，不妨先来回顾一下 Redis 的事务功能的用法。

Redis 的事务使用 `MULTI 命令 <http://redis.readthedocs.org/en/latest/transaction/multi.html>`_ 和 `EXEC 命令 <http://redis.readthedocs.org/en/latest/transaction/exec.html>`_ 包围，处在这两条命令之间的一条或多条命令，会以 FIFO 的方式运行：

::

    redis> MULTI                         # 标记事务开始
    OK

    redis> INCR user_id
    QUEUED

    redis> SET greeting "hello moto"
    QUEUED

    redis> GET replay
    QUEUED

    redis> EXEC                          # 标记事务结束，并执行事务
    1) (integer) 1
    2) OK
    3) "hello world"

需要注意的是，Redis 的事务和关系数据库的事务并不一样：Redis 的事务并不保证 `ACID <http://zh.wikipedia.org/wiki/ACID>`_ 性质。

也就是说，在 Redis 事务的执行过程中，因为服务器失败而造成数据不一致的情况是可能存在的，在后面对代码进行分析的时候，就会清晰地看到这一点。


MULTI 命令
---------------

每个 Redis 事务都以 ``MULTI`` 命令开始，而 ``MULTI`` 命令本身的实现则非常简单：

::

    void multiCommand(redisClient *c) {

        // MULTI 不可以嵌套使用
        if (c->flags & REDIS_MULTI) {
            addReplyError(c,"MULTI calls can not be nested");
            return;
        }

        c->flags |= REDIS_MULTI;    // 打开 FLAG
        addReply(c,shared.ok);
    }

``multiCommand`` 的主要动作就是对 ``redisClient`` 结构的 ``flags`` 进行检查和设置。

它首先检查 ``flags`` ，确保没有嵌套使用 ``MULTI`` 命令。

如果检查通过，那么就使用位或操作，将 ``REDIS_MULTI`` 这个 FLAG 打开。

最后向客户端返回 ``OK`` 。


命令的入队
----------------

当 ``REDIS_MULTI`` 这个 FLAG 被打开之后，
传入 Redis 客户端的命令就不会马上被执行（部分命令如 ``EXEC`` 除外），
这些未被执行的命令会被 ``queueMultiCommand`` 以 FIFO 的方式放入一个数组里，储存起来。

``redis.c`` 文件里的 ``processCommand`` 函数说明了这一点：

::

    int processCommand(redisClient *c) {
        // 其他代码 ...

        /* Exec the command */
        // 如果 REDIS_MULTI 被打开
        // 且要执行的命令不是 EXEC 、 DISCARD 、 MULTI 或 WATCH
        // 那么将这个命令入队
        if (c->flags & REDIS_MULTI &&
            c->cmd->proc != execCommand && c->cmd->proc != discardCommand &&
            c->cmd->proc != multiCommand && c->cmd->proc != watchCommand)
        {
            queueMultiCommand(c);       // 入队
            addReply(c,shared.queued);  // 返回已入队信息
        } else {
            call(c,REDIS_CALL_FULL);
        }
        return REDIS_OK;
    }

``queueMultiCommand`` 函数将要执行的命令、命令的参数个数以及命令的参数放进 ``multiCmd`` 结构中，并将这个结构保存到 ``redisClient.mstate.command`` 数组的末尾，从而形成一个保存了要执行的命令的 FIFO 队列：

::

    /* Add a new command into the MULTI commands queue */
    // 添加新命令到 MULTI 的执行队列中（FIFO）
    void queueMultiCommand(redisClient *c) {
        multiCmd *mc;
        int j;

        // 为新命令分配储存结构，并放到数组的末尾
        c->mstate.commands = zrealloc(c->mstate.commands,
                sizeof(multiCmd)*(c->mstate.count+1));

        // 设置新命令
        mc = c->mstate.commands+c->mstate.count;            // 指向储存新命令的结构体
        mc->cmd = c->cmd;                                   // 设置命令
        mc->argc = c->argc;                                 // 设置参数数量
        mc->argv = zmalloc(sizeof(robj*)*c->argc);          // 生成参数空间
        memcpy(mc->argv,c->argv,sizeof(robj*)*c->argc);     // 设置参数
        for (j = 0; j < c->argc; j++)
            incrRefCount(mc->argv[j]);

        // 更新命令数量的计数器
        c->mstate.count++;
    }

以下是 ``queueMultiCommand`` 函数用到的几个结构，放在 ``redis.h`` 文件中，定义都非常直观：

::

    typedef struct redisClient {
        // 其他属性 ...
        redisDb *db;                // 当前 DB
        int flags;                  // 标记事务状态，以及 WATCH 状态
        multiState mstate;          // 事务中的所有命令
        list *watched_keys;         // 这个客户端 WATCH 的所有 KEY
        // 其他属性 ...
    } redisClient;

    typedef struct multiState {
        multiCmd *commands;         // 保存事务中所有命令的数组
        int count;                  // 命令的数量
    } multiState;

    typedef struct multiCmd {
        robj **argv;                // 命令参数
        int argc;                   // 命令参数数量
        struct redisCommand *cmd;   // 命令
    } multiCmd;

回到文章开头的例子，在执行以下几个命令之后：

::

    redis> MULTI                         # 标记事务开始
    OK

    redis> INCR user_id
    QUEUED

    redis> SET greeting "hello moto"
    QUEUED

    redis> GET replay
    QUEUED

``redisClient.mstate`` 的值应该类似这个样子（用 JSON 结构来表示）：

.. code-block:: json

    redisClient.mstate = {
        'count': 3,
        'commands': [
            {
                'argv': ['user_id'],
                'argc': 1,
                'cmd': 'incrCommand',
            },
            {
                'argv': ['greeting', 'hello moto'],
                'argc': 2,
                'cmd': 'setCommand',
            },
            {
                'argv': ['replay'],
                'argc': 1,
                'cmd': 'getCommand',
            }
        ]
    }


执行事务
-------------

既然事务里已经有了等待执行的命令，那么此时不运行事务，更待何时？！

事务的执行由 ``execCommand`` 函数进行，它的定义如下：

::

    void execCommand(redisClient *c) {
        int j;
        robj **orig_argv;
        int orig_argc;
        struct redisCommand *orig_cmd;

        // 如果没执行过 MULTI ，报错
        if (!(c->flags & REDIS_MULTI)) {
            addReplyError(c,"EXEC without MULTI");
            return;
        }

        /* Check if we need to abort the EXEC if some WATCHed key was touched.
         * A failed EXEC will return a multi bulk nil object. */
        // 如果在执行事务之前，有监视中（WATCHED）的 key 被改变
        // 那么取消这个事务
        if (c->flags & REDIS_DIRTY_CAS) {
            freeClientMultiState(c);
            initClientMultiState(c);
            c->flags &= ~(REDIS_MULTI|REDIS_DIRTY_CAS);
            unwatchAllKeys(c);
            addReply(c,shared.nullmultibulk);
            return;
        }

        /* Replicate a MULTI request now that we are sure the block is executed.
         * This way we'll deliver the MULTI/..../EXEC block as a whole and
         * both the AOF and the replication link will have the same consistency
         * and atomicity guarantees. */
        // 为保证一致性和原子性
        // 如果处在 AOF 模式中，向 AOF 文件发送 MULTI
        // 如果处在复制模式中，向附属节点发送 MULTI
        execCommandReplicateMulti(c);

        /* Exec all the queued commands */
        // 开始执行所有事务中的命令（FIFO 方式）
        unwatchAllKeys(c); /* Unwatch ASAP otherwise we'll waste CPU cycles */

        // 备份所有参数和命令
        orig_argv = c->argv;
        orig_argc = c->argc;
        orig_cmd = c->cmd;
        addReplyMultiBulkLen(c,c->mstate.count);
        for (j = 0; j < c->mstate.count; j++) {
            c->argc = c->mstate.commands[j].argc;   // 取出参数数量
            c->argv = c->mstate.commands[j].argv;   // 取出参数
            c->cmd = c->mstate.commands[j].cmd;     // 取出要执行的命令
            call(c,REDIS_CALL_FULL);                // 执行命令

            /* Commands may alter argc/argv, restore mstate. */
            c->mstate.commands[j].argc = c->argc;
            c->mstate.commands[j].argv = c->argv;
            c->mstate.commands[j].cmd = c->cmd;
        }

        // 恢复所有参数和命令
        c->argv = orig_argv;
        c->argc = orig_argc;
        c->cmd = orig_cmd;

        // 重置事务状态
        freeClientMultiState(c);
        initClientMultiState(c);
        c->flags &= ~(REDIS_MULTI|REDIS_DIRTY_CAS);

        /* Make sure the EXEC command is always replicated / AOF, since we
         * always send the MULTI command (we can't know beforehand if the
         * next operations will contain at least a modification to the DB). */
        // 更新状态值，确保事务执行之后的状态为脏
        server.dirty++;
    }

``execCommand`` 函数的所有行为代码中都有注释，这里就不再赘述了。

需要提醒注意的是，在关键部分的 ``for`` 循环代码里，我们可以看见，
``execCommand`` 的主要作用只是一个个地执行储存在 ``redisClient.mstate`` 数组中的命令，
命令在执行之前并没有使用日志之类的保护机制，
这是为什么 Redis 的事务并不支持 ACID 这些性质的（其中一个）原因。

另外要注意的是，在 ``execCommand`` 的前半部分，调用了 ``execCommandReplicateMulti`` 函数，
如果有需要的话， Redis 就会向 AOF 文件和其他复制实例（replication）发送 ``MULTI`` 命令，
告诉它们：『哥要开始执行事务了，各单位请注意！』。

这样的话，如果事务在执行过程中失败，那么 AOF 文件和复制实例都会察觉到，
这时 Redis 实例会报错并退出，然后等待管理员使用 ``redis-check-aof`` 命令来进行数据修复，
具体请参考： [Redis 官方网站上的 Transaction 介绍](http://redis.io/topics/transactions) 。


取消事务
-----------

在一些情况下，我们也想在中途取消事务的执行。

`DISCARD <http://redis.readthedocs.org/en/latest/transaction/discard.html>`_ 命令就是用来中途取消事务的，
它的实现由 ``discardTransaction`` 和 ``discardCommand`` 两个函数实现：

::

    // 放弃执行事务
    void discardTransaction(redisClient *c) {
        freeClientMultiState(c);                        // 释放事务资源
        initClientMultiState(c);                        // 重置事务状态
        c->flags &= ~(REDIS_MULTI|REDIS_DIRTY_CAS);;    // 关闭 FLAG
        unwatchAllKeys(c);          // 取消对所有 key 的 WATCH
    }

    // 放弃执行事务（命令）
    void discardCommand(redisClient *c) {
        // 如果没有调用过 MULTI ，报错
        if (!(c->flags & REDIS_MULTI)) {
            addReplyError(c,"DISCARD without MULTI");
            return;
        }

        discardTransaction(c);
        addReply(c,shared.ok);
    }

其中 ``freeClientMultiState`` 和 ``initClientMultiState`` 两个函数用于重置 ``redisClient.mstate`` 数组，从而达到删除所有入队命令的作用：

::

    /* Client state initialization for MULTI/EXEC */
    // 初始化客户端状态，为执行事务作准备
    void initClientMultiState(redisClient *c) {
        c->mstate.commands = NULL;  // 清空命令数组
        c->mstate.count = 0;        // 清空命令计数器
    }

    /* Release all the resources associated with MULTI/EXEC state */
    // 释放所有事务资源
    void freeClientMultiState(redisClient *c) {
        int j;

        // 释放所有命令
        for (j = 0; j < c->mstate.count; j++) {
            int i;
            multiCmd *mc = c->mstate.commands+j;    // 将指针指向目标命令

            // 释放所有命令的参数，以及保存参数的数组
            for (i = 0; i < mc->argc; i++)
                decrRefCount(mc->argv[i]);
            zfree(mc->argv);
        }

        // 释放保存命令的数组
        zfree(c->mstate.commands);
    }


小结
------

对 Redis 事务的实现分析就到此结束了，希望这篇文章对你理解 Redis 的事务有所帮助。

跟之前一样，我将带有完整注释的代码放到了 GitHub 上，有兴趣了解全部细节的朋友可以参考源码： `https://github.com/huangz1990/reading_redis_source <https://github.com/huangz1990/reading_redis_source>`_ 。

最后，和 Redis 的事务有关的命令还有 ``WATCH`` 和 ``UNWATCH`` ，在下篇文章中，会继续探讨它们的实现方式。
