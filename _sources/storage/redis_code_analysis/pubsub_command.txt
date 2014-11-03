.. highlight:: c

PUBSUB 命令
=======================

:ref:`PUBSUB <redisref:pubsub>` 是 Redis 2.8 版本新增的命令，
用于对发布与订阅功能进行自省，
它的功能包括：

- ``PUBSUB CHANNELS`` 子命令查看服务器目前有多少个频道被订阅。

- ``PUBSUB NUMSUB`` 子命令查看某个频道有多少个订阅者（订阅这个频道的客户端）。

- ``PUBSUB NUMPAT`` 子命令查看某个模式有多少订阅者。

:ref:`PUBSUB` 的具体功能可以参考 :ref:`PUBSUB 命令的文档 <redisref:pubsub>` ，
本文只对 :ref:`PUBSUB` 命令的实现进行分析，
而不对 :ref:`PUBSUB` 的作用本身做详细的介绍。

PUBSUB CHANNELS
---------------------

首先来看 ``PUBSUB CHANNELS`` 子命令 —— 
这个命令的格式为 ``PUBSUB CHANNELS [pattern]`` ：

- 当不给定 ``pattern`` 参数的时候，
  命令返回当前服务器中被订阅的所有频道（的名字）。

- 而当给定 ``pattern`` 参数的时候，
  命令返回当前服务器中和 ``pattern`` 模式相匹配的所有频道（的名字）。

以下是 :ref:`PUBSUB` 命令关于 ``CHANNELS`` 子命令的代码：

::

        // PUBSUB CHANNELS [pattern] 子命令
        if (!strcasecmp(c->argv[1]->ptr,"channels") &&
            (c->argc == 2 || c->argc ==3))
        {
            /* PUBSUB CHANNELS [<pattern>] */
            // 检查命令请求是否给定了 pattern 参数
            // 如果没有给定的话，就设为 NULL
            sds pat = (c->argc == 2) ? NULL : c->argv[2]->ptr;

            // 创建 pubsub_channels 的字典迭代器
            // 该字典的键为频道，值为链表
            // 链表中保存了所有订阅键所对应的频道的客户端
            dictIterator *di = dictGetIterator(server.pubsub_channels);
            dictEntry *de;
            long mblen = 0;
            void *replylen;

            replylen = addDeferredMultiBulkLength(c);
            // 从迭代器中获取一个客户端
            while((de = dictNext(di)) != NULL) {

                // 从字典中取出客户端所订阅的频道
                robj *cobj = dictGetKey(de);
                sds channel = cobj->ptr;

                // 顺带一提
                // 因为 Redis 的字典实现只能遍历字典的值（客户端）
                // 所以这里才会有遍历字典值然后通过字典值取出字典键（频道）的蹩脚用法

                // 如果没有给定 pattern 参数，那么打印所有找到的频道
                // 如果给定了 pattern 参数，那么只打印和 pattern 相匹配的频道
                if (!pat || stringmatchlen(pat, sdslen(pat),
                                           channel, sdslen(channel),0))
                {
                    // 向客户端输出频道
                    addReplyBulk(c,cobj);
                    mblen++;
                }
            }
            // 释放字典迭代器
            dictReleaseIterator(di);
            setDeferredMultiBulkLength(c,replylen,mblen);
        }

理解这个命令的最重要的地方就是理解服务器的 ``pubsub_channels`` 字典，
如果不清楚这个字典的作用（或者忘记了的话），
请参考《Redis 设计与实现》的《\ `订阅频道 <http://www.redisbook.com/en/latest/feature/pubsub.html#id3>`_\ 》小节。


PUBSUB NUMSUB
------------------

``PUBSUB NUMSUB`` 子命令的格式为 ``PUBSUB NUMSUB [channel-1 channel-2 ... channel-n]`` ：
命令接收任意数量的频道名字，
并返回频道的订阅者数量（有多少个客户端在订阅这个频道）。

以下是 :ref:`PUBSUB` 命令关于 ``NUMSUB`` 子命令的代码：

::

        // PUBSUB NUMSUB [channel-1 channel-2 ... channel-N] 子命令
        } else if (!strcasecmp(c->argv[1]->ptr,"numsub") && c->argc >= 2) {
            /* PUBSUB NUMSUB [Channel_1 ... Channel_N] */
            int j;

            addReplyMultiBulkLen(c,(c->argc-2)*2);
            for (j = 2; j < c->argc; j++) {

                // c->argv[j] 也即是客户端输入的第 N 个频道名字
                // pubsub_channels 的字典为频道名字
                // 而值则是保存了 c->argv[j] 频道所有订阅者的链表
                // 而调用 dictFetchValue 也就是取出所有订阅给定频道的客户端
                list *l = dictFetchValue(server.pubsub_channels,c->argv[j]);

                addReplyBulk(c,c->argv[j]);
                // 向客户端返回链表的长度属性
                // 这个属性就是某个频道的订阅者数量
                // 例如：如果一个频道有三个订阅者，那么链表的长度就是 3
                // 而返回给客户端的数字也是三
                addReplyBulkLongLong(c,l ? listLength(l) : 0);
            }
        }

和 ``PUBSUB CHANNELS`` 命令一样，
理解 ``PUBSUB NUMSUB`` 命令最重要的就是理解服务器的 ``pubsub_channels`` 字典，
如果不理解这个字典的话，
请去《Redis 设计于实现》中看看关于 ``pubsub_channels`` 字典的部分。


PUBSUB NUMPAT
-----------------

``PUBSUB NUMPAT`` 命令返回服务器当前被订阅模式的数量，
以下是 :ref:`PUBSUB` 命令中关于 ``NUMPAT`` 子命令部分的代码：

::

        // PUBSUB NUMPAT 子命令
        } else if (!strcasecmp(c->argv[1]->ptr,"numpat") && c->argc == 2) {
            /* PUBSUB NUMPAT */

            // pubsub_patterns 链表保存了服务器中所有被订阅的模式
            // pubsub_patterns 的长度就是服务器中被订阅模式的数量
            addReplyLongLong(c,listLength(server.pubsub_patterns));
        }

理解 ``NUMPAT`` 子命令的关键就是理解服务器的 ``pubsub_patterns`` 链表，
这个链表保存了服务器中所有被订阅的模式。

如果不了解这个 ``pubsub_patterns`` 链表（或者忘记了）的话，
请参考《Redis 设计与实现》一书的《\ `订阅模式 <http://www.redisbook.com/en/latest/feature/pubsub.html#id7>`_\ 》小节。


PUBSUB 命令的完整代码
-------------------------

以下是带注释的 :ref:`PUBSUB` 命令的完整代码：

::

    /* PUBSUB command for Pub/Sub introspection. */
    void pubsubCommand(redisClient *c) {

        // PUBSUB CHANNELS [pattern] 子命令
        if (!strcasecmp(c->argv[1]->ptr,"channels") &&
            (c->argc == 2 || c->argc ==3))
        {
            /* PUBSUB CHANNELS [<pattern>] */
            // 检查命令请求是否给定了 pattern 参数
            // 如果没有给定的话，就设为 NULL
            sds pat = (c->argc == 2) ? NULL : c->argv[2]->ptr;

            // 创建 pubsub_channels 的字典迭代器
            // 该字典的键为频道，值为链表
            // 链表中保存了所有订阅键所对应的频道的客户端
            dictIterator *di = dictGetIterator(server.pubsub_channels);
            dictEntry *de;
            long mblen = 0;
            void *replylen;

            replylen = addDeferredMultiBulkLength(c);
            // 从迭代器中获取一个客户端
            while((de = dictNext(di)) != NULL) {

                // 从字典中取出客户端所订阅的频道
                robj *cobj = dictGetKey(de);
                sds channel = cobj->ptr;

                // 顺带一提
                // 因为 Redis 的字典实现只能遍历字典的值（客户端）
                // 所以这里才会有遍历字典值然后通过字典值取出字典键（频道）的蹩脚用法

                // 如果没有给定 pattern 参数，那么打印所有找到的频道
                // 如果给定了 pattern 参数，那么只打印和 pattern 相匹配的频道
                if (!pat || stringmatchlen(pat, sdslen(pat),
                                           channel, sdslen(channel),0))
                {
                    // 向客户端输出频道
                    addReplyBulk(c,cobj);
                    mblen++;
                }
            }
            // 释放字典迭代器
            dictReleaseIterator(di);
            setDeferredMultiBulkLength(c,replylen,mblen);

        // PUBSUB NUMSUB [channel-1 channel-2 ... channel-N] 子命令
        } else if (!strcasecmp(c->argv[1]->ptr,"numsub") && c->argc >= 2) {
            /* PUBSUB NUMSUB [Channel_1 ... Channel_N] */
            int j;

            addReplyMultiBulkLen(c,(c->argc-2)*2);
            for (j = 2; j < c->argc; j++) {

                // c->argv[j] 也即是客户端输入的第 N 个频道名字
                // pubsub_channels 的字典为频道名字
                // 而值则是保存了 c->argv[j] 频道所有订阅者的链表
                // 而调用 dictFetchValue 也就是取出所有订阅给定频道的客户端
                list *l = dictFetchValue(server.pubsub_channels,c->argv[j]);

                addReplyBulk(c,c->argv[j]);
                // 向客户端返回链表的长度属性
                // 这个属性就是某个频道的订阅者数量
                // 例如：如果一个频道有三个订阅者，那么链表的长度就是 3
                // 而返回给客户端的数字也是三
                addReplyBulkLongLong(c,l ? listLength(l) : 0);
            }

        // PUBSUB NUMPAT 子命令
        } else if (!strcasecmp(c->argv[1]->ptr,"numpat") && c->argc == 2) {
            /* PUBSUB NUMPAT */

            // pubsub_patterns 链表保存了服务器中所有被订阅的模式
            // pubsub_patterns 的长度就是服务器中被订阅模式的数量
            addReplyLongLong(c,listLength(server.pubsub_patterns));

        // 错误处理
        } else {
            addReplyErrorFormat(c,
                "Unknown PUBSUB subcommand or wrong number of arguments for '%s'",
                (char*)c->argv[1]->ptr);
        }
    }


EOF
------

以上就是本文的全部内容，
谢谢阅读！


| huangz
| 2013.7.11
