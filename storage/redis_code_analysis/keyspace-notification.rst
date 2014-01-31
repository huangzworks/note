.. highlight:: c

键空间通知
===================

键空间通知（keyspace notification）是仍在开发的 Redis 2.8 版本的新功能，
本文将对该功能的底层实现进行介绍。

本文假定你已经了解如何使用键空间通知功能，
如果你不具备这一前提条件，
请在阅读本文前先阅读 `键空间通知的文档 <http://redis.readthedocs.org/en/latest/topic/notification.html>`_ 。


选择通知的类型
----------------

通过对服务器的 ``notify-keyspace-events`` 选项进行修改，
用户可以选择开启或关闭键空间通知功能，
或者只接收特定类型的通知。

- 比如说，如果执行 ``CONFIG SET notify-keyspace-events Kh`` ，那么服务器将只发送和哈希类型有关的键空间通知。

- 又比如，如果执行 ``CONFIG SET notify-keyspace-events KElsh`` 命令的话，那么服务器将发送所有和列表、集合、哈希相关的键空间通知和键事件通知。

在通过 ``CONFIG SET`` 命令设置 ``notify-keyspace-events`` 选项的参数时，
``notify.c/keyspaceEventsStringToFlags`` 函数会对传入的字符串参数进行分析，
给出相应的 ``flags`` 值，
并将它赋值给服务器状态 ``server.notify_keyspace_events`` 属性：


::

    void configSetCommand(redisClient *c) {

    // ...

    } else if (!strcasecmp(c->argv[2]->ptr,"notify-keyspace-events")) {

        // 根据字符串参数，计算 flags 的值
        int flags = keyspaceEventsStringToFlags(o->ptr);
 
        // 传入的字符串带有不能识别的字符，出错
        if (flags == -1) goto badfmt;

        // 设置属性
        server.notify_keyspace_events = flags;
    }

    // ...

``keyspaceEventsStringToFlags`` 函数位于 ``notify.c`` 文件中，
它的完整定义如下：

::

    /* Turn a string representing notification classes into an integer
     * representing notification classes flags xored.
     *
     * The function returns -1 if the input contains characters not mapping to
     * any class. */
    int keyspaceEventsStringToFlags(char *classes) {
        char *p = classes;
        int c, flags = 0;

        while((c = *p++) != '\0') {
            switch(c) {
            case 'A': flags |= REDIS_NOTIFY_ALL; break;
            case 'g': flags |= REDIS_NOTIFY_GENERIC; break;
            case '$': flags |= REDIS_NOTIFY_STRING; break;
            case 'l': flags |= REDIS_NOTIFY_LIST; break;
            case 's': flags |= REDIS_NOTIFY_SET; break;
            case 'h': flags |= REDIS_NOTIFY_HASH; break;
            case 'z': flags |= REDIS_NOTIFY_ZSET; break;
            case 'x': flags |= REDIS_NOTIFY_EXPIRED; break;
            case 'e': flags |= REDIS_NOTIFY_EVICTED; break;
            case 'K': flags |= REDIS_NOTIFY_KEYSPACE; break;
            case 'E': flags |= REDIS_NOTIFY_KEYEVENT; break;
            default: return -1;
            }
        }
        return flags;
    }

``notify.c`` 中还定义了一个和 ``keyspaceEventsStringToFlags`` 相反的函数 —— ``keyspaceEventsFlagsToString`` ，
这个函数可以根据 ``flags`` 值还原设置这个 ``flags`` 所需的字符串。

当调用 ``CONFIG GET`` 命令取出选项的值时，
Reids 就会将当前的 ``server.notify_keyspace_events`` 的值传给这个函数，
让它将 ``flags`` 的字符串表示返回给用户：

::

    redis 127.0.0.1:6379> CONFIG SET notify-keyspace-events Kh
    OK

    redis 127.0.0.1:6379> CONFIG GET notify-keyspace-events
    1) "notify-keyspace-events"
    2) "hK"

    redis 127.0.0.1:6379> CONFIG SET notify-keyspace-events KElsh
    OK

    redis 127.0.0.1:6379> CONFIG GET notify-keyspace-events
    1) "notify-keyspace-events"
    2) "lshKE"

以下是 ``keyspaceEventsFlagsToString`` 函数的完整定义（同样为于 ``notify.c`` 中）：

::

    /* This function does exactly the revese of the function above: it gets
     * as input an integer with the xored flags and returns a string representing
     * the selected classes. The string returned is an sds string that needs to
     * be released with sdsfree(). */
    sds keyspaceEventsFlagsToString(int flags) {
        sds res;

        if ((flags & REDIS_NOTIFY_ALL) == REDIS_NOTIFY_ALL)
            return sdsnew("A");
        res = sdsempty();
        if (flags & REDIS_NOTIFY_GENERIC) res = sdscatlen(res,"g",1);
        if (flags & REDIS_NOTIFY_STRING) res = sdscatlen(res,"$",1);
        if (flags & REDIS_NOTIFY_LIST) res = sdscatlen(res,"l",1);
        if (flags & REDIS_NOTIFY_SET) res = sdscatlen(res,"s",1);
        if (flags & REDIS_NOTIFY_HASH) res = sdscatlen(res,"h",1);
        if (flags & REDIS_NOTIFY_ZSET) res = sdscatlen(res,"z",1);
        if (flags & REDIS_NOTIFY_EXPIRED) res = sdscatlen(res,"x",1);
        if (flags & REDIS_NOTIFY_EVICTED) res = sdscatlen(res,"e",1);
        if (flags & REDIS_NOTIFY_KEYSPACE) res = sdscatlen(res,"K",1);
        if (flags & REDIS_NOTIFY_KEYEVENT) res = sdscatlen(res,"E",1);
        return res;
    }


通知的发送
----------------

Redis 中每个对键进行了修改的命令，
都会命令的实现函数中调用 ``notify.c/notifyKeyspaceEvent`` 函数，
从而发送命令自己特有的通知。

比如说，
实现 `SET <http://redis.readthedocs.org/en/latest/string/set.html>`_ 、 `SETNX <http://redis.readthedocs.org/en/latest/string/setnx.html>`_ 、 `SETEX <http://redis.readthedocs.org/en/latest/string/setex.html>`_ 等命令的 ``t_string.c/setGenericCommand`` 函数就会在程序的末尾发送 ``"set"`` 通知，
如果命令执行的是 `SETEX <http://redis.readthedocs.org/en/latest/string/setex.html>`_ 命令的话，
那么除了 ``"set"`` 通知之外，
程序还需要发送 ``"expire"`` 通知：

::

    void setGenericCommand(
        redisClient *c,
        int flags,
        robj *key,
        robj *val,
        robj *expire,
        int unit,
        robj *ok_reply,
        robj *abort_reply
    ) {

        // ...

        // 发送设置事件通知
        notifyKeyspaceEvent(REDIS_NOTIFY_STRING,"set",key,c->db->id);

        // 如果执行的是 SETEX ，那么还需要发送过期时间设置通知
        if (expire) notifyKeyspaceEvent(REDIS_NOTIFY_GENERIC,
            "expire",key,c->db->id);

        // ...
    }

又比如，
实现 `SADD <http://redis.readthedocs.org/en/latest/set/sadd.html>`_ 命令的 ``t_set.c/saddCommand`` 函数，
就会在至少有一个元素被成功添加到集合时，
发送 ``"sadd"`` 通知：

::

    void saddCommand(redisClient *c) {

        // ...

        // 如果有至少一个元素被成功添加，那么执行以下程序
        if (added) {

            // ...

            // 发送添加元素通知
            notifyKeyspaceEvent(REDIS_NOTIFY_SET,"sadd",c->argv[1],c->db->id);
        }

        // ...
    }

尽管不同命令发送的通知可能各不相同，
但是这些通知都是由 ``notifyKeyspaceEvent`` 函数发送的，
这个函数的完整定义如下：

::

    /* The API provided to the rest of the Redis core is a simple function:
     *
     * notifyKeyspaceEvent(char *event, robj *key, int dbid);
     *
     * 'event' is a C string representing the event name.
     *
     * event 参数是一个字符串表示的事件名
     *
     * 'key' is a Redis object representing the key name.
     *
     * key 参数是一个 Redis 对象表示的键名
     *
     * 'dbid' is the database ID where the key lives.  
     *
     * dbid 参数为键所在的数据库
     */
    void notifyKeyspaceEvent(int type, char *event, robj *key, int dbid) {
        sds chan;
        robj *chanobj, *eventobj;
        int len = -1;
        char buf[24];

        /* If notifications for this class of events are off, return ASAP. */
        // 如果服务器配置为不发送 type 类型的通知，那么直接返回
        if (!(server.notify_keyspace_events & type)) return;

        // 事件的名字
        eventobj = createStringObject(event,strlen(event));

        /* __keyspace@<db>__:<key> <event> notifications. */
        // 发送键空间通知
        if (server.notify_keyspace_events & REDIS_NOTIFY_KEYSPACE) {

            // 构建频道对象
            chan = sdsnewlen("__keyspace@",11);
            len = ll2string(buf,sizeof(buf),dbid);
            chan = sdscatlen(chan, buf, len);
            chan = sdscatlen(chan, "__:", 3);
            chan = sdscatsds(chan, key->ptr);

            chanobj = createObject(REDIS_STRING, chan);

            // 通过 publish 命令发送通知
            pubsubPublishMessage(chanobj, eventobj);

            // 释放频道对象
            decrRefCount(chanobj);
        }

        /* __keyevente@<db>__:<event> <key> notifications. */
        // 发送键事件通知
        if (server.notify_keyspace_events & REDIS_NOTIFY_KEYEVENT) {

            // 构建频道对象
            chan = sdsnewlen("__keyevent@",11);
            // 如果在前面发送键空间通知的时候计算了 len ，那么它就不会是 -1
            // 这可以避免计算两次 buf 的长度
            if (len == -1) len = ll2string(buf,sizeof(buf),dbid);
            chan = sdscatlen(chan, buf, len);
            chan = sdscatlen(chan, "__:", 3);
            chan = sdscatsds(chan, eventobj->ptr);

            chanobj = createObject(REDIS_STRING, chan);

            // 通过 publish 命令发送通知
            pubsubPublishMessage(chanobj, key);

            // 释放频道对象
            decrRefCount(chanobj);
        }

        // 释放事件对象
        decrRefCount(eventobj);
    }

``notifyKeyspaceEvent`` 函数执行以下步骤：

1. 根据 ``server.notify_keyspace_events`` 的值，决定应该发送那些通知。

2. 构建通知的频道，以及通知的内容（事件）。

3. 通过 ``pubsubPublishMessage`` 函数，将事件从频道中发送出去。

``pubsubPublishMessage`` 函数是 `PUBLISH <http://redis.readthedocs.org/en/latest/pub_sub/publish.html>`_ 命令的实现函数，
调用它相当于调用 `PUBLISH <http://redis.readthedocs.org/en/latest/pub_sub/publish.html>`_ 命令。

当通知被 ``pubsubPublishMessage`` 函数发送出去之后，
之后的处理就交给订阅与发布模块来处理了，
``notifyKeyspaceEvent`` 的使命就此完成。


实例
------------

文章上一节对键空间事件的底层实现进行了介绍，
为了更透彻地理解键空间事件的具体运作方式，
让我们来看一个实际的例子。

首先，
启动 Redis 服务器，
将服务器的 ``notify-keyspace-events`` 选项的参数设置为 ``Es`` ，
让服务器只发送和集合有关的键事件通知：

::

    redis 127.0.0.1:6379> CONFIG SET notify-keyspace-events Es
    OK

接着，
让客户端订阅模式 ``__keyevent*`` ，
接收所有键事件通知：

::

    # 0 号终端

    redis 127.0.0.1:6379> PSUBSCRIBE __keyevent*
    Reading messages... (press Ctrl-C to quit)
    1) "psubscribe"
    2) "_keyevent*"
    3) (integer) 1

然后，
开启另一个终端，
启动 Redis 客户端，
发送以下命令：

::

    # 1 号终端

    redis 127.0.0.1:6379> SADD favorite-fruits orange
    (integer) 1

命令发送完毕之后，
切换回 ``0`` 号终端，
可以看到，
客户端接收到了新的键事件通知：

::

    # 0 号终端

    1) "pmessage"
    2) "__keyevent*"            # 被匹配的模式
    3) "__keyevent@0__:sadd"    # 消息的来源频道
    4) "favorite-fruits"        # 执行 SADD 命令的键

以下是这个通知的完整发送步骤：

1. Redis 接收并处理输入的 `SADD <http://redis.readthedocs.org/en/latest/set/sadd.html>`_ 命令，并执行前面说过的 ``saddCommand`` 函数。

2. ``saddCommand`` 函数执行将新元素添加到集合的动作，然后以 ``"sadd"`` 为通知内容，调用 ``notifyKeyspaceEvent`` 函数：

  ::

      void saddCommand(redisClient *c) {

          // ...

          // 如果有至少一个元素被成功添加，那么执行以下程序
          if (added) {

              // ...

              // 发送添加元素通知
              notifyKeyspaceEvent(REDIS_NOTIFY_SET,"sadd",c->argv[1],c->db->id);
          }

          // ...
      }

3. ``notifyKeyspaceEvent`` 首先检查 ``server.notify_keyspace_events`` 属性，确保键空间事件功能已开启，并且 ``"sadd"`` 是服务器允许发送的通知之一：

  ::

      // 如果服务器配置为不发送 type 类型的通知，那么直接返回
      if (!(server.notify_keyspace_events & type)) return;


4. 接着 ``notifyKeyspaceEvent`` 检查是否需要发送键空间通知：

  ::

      /* __keyspace@<db>__:<key> <event> notifications. */
      // 发送键空间通知
      if (server.notify_keyspace_events & REDIS_NOTIFY_KEYSPACE) {

          // ...

      }

  因为前面我们设置 ``keyspace-notify-events`` 选项的时候，并未包含字符 ``'K'`` ，所以这一检测的结果为假， ``notifyKeyspaceEvent`` 不会发送键空间通知。

5. 最后 ``notifyKeyspaceEvent`` 检查是否需要发送键事件通知：

  ::


      /* __keyevente@<db>__:<event> <key> notifications. */
      // 发送键事件通知
      if (server.notify_keyspace_events & REDIS_NOTIFY_KEYEVENT) {

          // ...

      }

  因为我们提供给服务器的 ``keyspace-notify-event`` 选项包含字符 ``'E'`` ，所以这个检测为真。
  
  程序将构建一条内容为 ``"favorite-fruits"`` 的事件通知，并调用 ``pubsubPublishMessage`` 函数，将通知发送到 ``__keyevent@0__:sadd`` 频道。

6. 至此， ``notifyKeyspaceEvent`` 执行完毕，整个发送步骤完成。


总结
-----------

以上介绍的三个函数就是 ``notify.c`` 中的所有内容了 —— 
通过使用内建的订阅与发布功能为基础，
Redis 只用了不到 150 行代码就实现了键空间通知功能，
这可以说是 Redis 又一次高效地重用代码来实现新功能的例子，
其中的模块化思想非常值得我们学习。


扩展阅读
------------

如果有兴趣弄清楚订阅与发布模块是如何分发信息的，
可以参考 `订阅与发布模块的源码分析文章 <http://www.huangz.me/en/latest/storage/redis_code_analysis/pubsub.html>`_ ，或者《\ `Redis 设计与实现 <http://redisbook.com>`_\ 》中的《\ `订阅与发布 <http://www.redisbook.com/en/latest/feature/pubsub.html>`_\ 》章节。


| huangz
| 2013.4.18
