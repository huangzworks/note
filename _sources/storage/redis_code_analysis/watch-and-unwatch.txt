.. highlight:: c

WATCH 和 UNWATCH
======================

对 Redis 的 ``WATCH`` 命令和 ``UNWATCH`` 命令的源码进行分析，
了解 Redis 事务中的乐观锁实现机制。

和之前介绍的 `Redis 事务命令 <http://huangz.me/redis-transaction-implement/>`_ 一样， ``WATCH`` 和 ``UNWATCH`` 命令的源码也放在 ``multi.c`` 文件中。


WATCH 命令
-----------------

``WATCH`` 的主要作用是在事务中构建乐观锁。

以下是使用 ``WATCH`` 时最常见的两种情形：

第一种可能是：
客户端对某个或某些 KEY 进行 ``WATCH`` ，
如果在调用 ``EXEC`` 命令之前，没有任何 ``WATCH`` 监视的 KEY 被改动，
那么事务可以开始执行。

就像这样：

::

    redis> WATCH number
    OK
    redis> MULTI
    OK
    redis> SET number 10086
    QUEUED
    redis> EXEC
    1) OK

另一种可能的情况是：
在对 KEY 进行 ``WATCH`` 之后，客户端执行 ``EXEC`` 之前，
有其他客户端对被 ``WATCH`` 的 KEY 进行了改动，
那么事务的执行就会被取消。

就像这样：

::

    redis> WATCH number
    OK
    redis> MULTI
    OK
    redis> SET number 123456
    QUEUED
    redis> EXEC
    (nil)

Redis 向客户端返回 ``nil`` ，
表示事务执行失败，
没有任何命令被执行。


与 WATCH 有关的数据结构
------------------------------------

为了实用性和操作的方便性，
Redis 将被 ``WATCH`` 的 KEY 的信息分别保存到两个数据结构中。

其中一个是 ``redis.h`` 文件中的 ``redisClient`` 结构，
它的 ``watched_keys`` 是一个链表，
所有被监视的 KEY 都被保存在这个链表里：

::

    typedef struct redisClient {
        // 其他属性 ...
        redisDb *db;                // 当前 DB
        list *watched_keys;         // 这个客户端 WATCH 的所有 KEY
        // 其他属性 ...
    } redisClient;

``watched_keys`` 链表中的每个节点都是一个 ``watchedKey`` 结构：

::

    typedef struct watchedKey {
        robj *key;      // 被 WATCH 的 KEY
        redisDb *db;    // 被 WATCH 的 KEY 所在的 DB
    } watchedKey;

以前面执行的 ``WATCH`` 命令为例子，在执行 ``EXEC`` 的时候， ``watched_keys`` 的值应该类似于这样（用 JSON 结构表示）：

.. code-block:: json

    redisClient.watched_keys = [
        // 链表
        {   // watchedKey struct
            'key': 'number',
            'db': 0
        }
    ]

另一方面，除了 ``redisClient`` 之外，Redis 还将被 ``WATCH`` 的 KEY 添加到 ``redisDb`` 结构中（这个结构也在 ``redis.h`` 文件里）：

::

    typedef struct redisDb {
        // 其他属性 ...
        dict *watched_keys;
    } redisDb;

在 Redis 中，每个数据库实例都使用一个 ``redisDb`` 来表示，
而每个 ``redisDb`` 都使用一个字典 ``watched_keys`` ，来保存被 ``WATCH`` KEY 的信息。

这些字典使用被 ``WATCH`` 的 KEY 作为关键字，而字典的值则是一个链表，链表中保存了所有 ``WATCH`` 这个数据库的这个 KEY 的所有客户端。

还是用前面执行的那个 ``WATCH`` 作为例子，假设我们使用的客户端代号为 ``client_123`` ，那么 ``redisDb.watched_keys`` 这个属性的值应该类似于这样（用 JSON 结构表示）：

.. code-block:: json

    redisDb.watched_keys = {
        // 被 WATCH 的 KEY : 所有客户端
        'number' : ['client_123']
    }


WATCH 数据结构的实现决策
-----------------------------

上一节讲到，Redis 将 ``WATCH`` KEY 的信息分别放到了两个地方，
一个是 ``redisClient.watched_keys`` 链表，
另一个是 ``redisDb.watched_keys`` 字典。

Redis 将 ``WATCH`` 放在这两个不同的地方，
分别用于实现两种不同的操作：

对于客户端来说， ``redisClient.watched_keys`` 将所有被监视的 KEY 放在一个链表中，这样 Redis 就可以很方便地设置或清除当前客户端所 ``WATCH`` 的 KEY 。

举个例子，如果某个客户端同时对 ``member`` 、 ``salary`` 和 ``infomation`` 三个 KEY 进行 ``WATCH`` ，当要撤销 ``WATCH`` 的时候（可能是由于事务成功了，也可能是事务失败了，也可能是用户手动执行 `UNWATCH <http://redis.readthedocs.org/en/latest/transaction/unwatch.html>`_ ，谁知道呢），Redis 就可以通过遍历列表，将这个客户端 ``WATCH`` 的这个三个 KEY 都删除。

另一方面，对于数据库而言，这个数据库的某个 KEY ，可能会被多个客户端同时 ``WATCH`` ，因此，在 ``redisDb.watched_keys`` 这个字典里，使用 KEY 作为字典的键，所有 ``WATCH`` 这个 KEY 的客户端的链表作为字典的值，这样就可以在其中一个客户端成功对这个 KEY 进行修改之后，让所有其他客户端都失败。

举个例子，假设 ``client_123`` 、 ``client_456`` 和 ``client_789`` 三个客户端都同时对 ``number`` 这个 KEY 进行 ``WATCH`` ，那么这三个客户端都会被放进链表中，形成一个类似这样的结构（用 JSON 表示）：

.. code-block:: json

    {
        'number': ['client_123', 'client_456', 'client_789']
    }

之后，如果 ``client_123`` 成功修改了 ``number`` 这个 KEY ，那么 Redis 就可以（很方便地）通过遍历链表，对 ``client_456`` 和 ``client_789`` 发起通知，让它们的事务失败（因为 ``number`` 已经被 ``client_123`` 修改了）。

虽然将 ``WATCH`` KEY 的信息复制到两个地方咋看上去很奇怪，
但实际上这是一个对不同操作进行衡量之后做出的实现决策。


WATCH 命令的实现
---------------------

``WATCH`` 命令由 ``watchCommand`` 函数实现，
它对用户输入的所有 KEY 调用 ``watchForKey`` 函数，
然后返回 ``OK`` ：

::

    void watchCommand(redisClient *c) {
        int j;

        if (c->flags & REDIS_MULTI) {
            addReplyError(c,"WATCH inside MULTI is not allowed");
            return;
        }
        for (j = 1; j < c->argc; j++)
            watchForKey(c,c->argv[j]);
        addReply(c,shared.ok);
    }

``watchForKey`` 函数需要完成三件事：

1) 检查给定 KEY 是否已经被被 ``WATCH`` 过：也即是，检查 KEY 是否已经存在于 ``redisClient.watched_keys`` 这个链表中。

如果 KEY 没有被 ``WATCH`` 过，那么执行以下两个步骤：

2) 将 KEY 信息添加到当前数据库的 ``redisDb.watched_keys`` 属性中。

3) 将 KEY 信息添加到当前客户端的 ``redisClient.watched_keys`` 属性中。

``watchForKey`` 函数的完整定义如下：

::

    /* Watch for the specified key */
    // WATCH 某个 KEY
    void watchForKey(redisClient *c, robj *key) {
        list *clients = NULL;
        listIter li;
        listNode *ln;
        watchedKey *wk;

        /* Check if we are already watching for this key */
        // 所有被 WATCHED 的 KEY 都被放在 redisClient.watched_keys 链表中
        // 遍历这个链表，查看这个 KEY 是否已经处于监视状态（WATCHED）
        listRewind(c->watched_keys,&li);
        while((ln = listNext(&li))) {
            wk = listNodeValue(ln);
            if (wk->db == c->db && equalStringObjects(key,wk->key))
                return; /* Key already watched */
        }

        /* This key is not already watched in this DB. Let's add it */
        // 如果 KEY 还没有被 WATCH 过，那么对它进行 WATCH
        clients = dictFetchValue(c->db->watched_keys,key);
        if (!clients) { 
            // 如果 clients 链表不存在
            // 说明这个客户端是第一个监视这个 DB 的这个 KEY 的客户端
            // 那么创建 clients 链表，并将它添加到 c->db->watched_keys 字典中
            clients = listCreate();
            dictAdd(c->db->watched_keys,key,clients);
            incrRefCount(key);
        }
        // 将客户端添加到 clients 链表
        listAddNodeTail(clients,c); 

        /* Add the new key to the lits of keys watched by this client */
        // 除了 c->db->watched_keys 之外
        // 还要将被 WATCH 的 KEY 添加到 c->watched_keys
        wk = zmalloc(sizeof(*wk));
        wk->key = key;
        wk->db = c->db;
        incrRefCount(key);
        listAddNodeTail(c->watched_keys,wk);
    }


UNWATCH 命令的实现
----------------------

``UNWATCH`` 命令的实现就是 ``WATCH`` 命令的反向操作：它从 ``redisDb.watched_keys`` 和 ``redisClient.watched_keys`` 中清除当前客户端的所有 ``WATCH`` KEY 的信息。

::

    /* Unwatch all the keys watched by this client. To clean the EXEC dirty
     * flag is up to the caller. */
    // 撤销对这个客户端的所有 WATCH
    // 清除 EXEC dirty FLAG 的任务由调用者完成
    void unwatchAllKeys(redisClient *c) {
        listIter li;
        listNode *ln;

        // 没有 WATCHED KEY ，直接返回
        if (listLength(c->watched_keys) == 0) return;

        listRewind(c->watched_keys,&li);
        while((ln = listNext(&li))) {
            list *clients;
            watchedKey *wk;

            /* Lookup the watched key -> clients list and remove the client
             * from the list */
            // 将当前客户端从监视 KEY 的链表中移除
            wk = listNodeValue(ln);
            clients = dictFetchValue(wk->db->watched_keys, wk->key);
            redisAssertWithInfo(c,NULL,clients != NULL);
            listDelNode(clients,listSearchKey(clients,c));

            /* Kill the entry at all if this was the only client */
            // 如果监视 KEY 的只有这个客户端
            // 那么将链表从字典中删除
            if (listLength(clients) == 0)
                dictDelete(wk->db->watched_keys, wk->key);

            /* Remove this watched key from the client->watched list */
            // 还需要将 KEY 从 client->watched_keys 链表中移除
            listDelNode(c->watched_keys,ln);
            decrRefCount(wk->key);
            zfree(wk);
        }
    }


touchWatchedKey 和 touchWatchedKeysOnFlush 函数
-----------------------------------------------------

还有另外两个需要一提的函数，
那就是 ``touchWatchedKey`` 函数和 ``touchWatchedKeysOnFlush`` 函数。

``touchWatchedKey`` 接受一个 ``redisDb`` 实例和一个 KEY ，然后将所有 ``WATCHE`` 这个 KEY 的客户端的 ``REDIS_DIRTY_CAS`` 状态都打开。

这样就可以在多个客户端同时 ``WATCHE`` 一个 KEY 的情况下，方便地让多个客户端的事务执行失败，在文章的前面也举过这样一个例子。

::

    // 打开所有 WATCH 给定 KEY 的客户端的 REDIS_DIRTY_CAS 状态
    // 使得接下来的 EXEC 执行失败
    void touchWatchedKey(redisDb *db, robj *key) {
        list *clients;
        listIter li;
        listNode *ln;

        if (dictSize(db->watched_keys) == 0) return;
        clients = dictFetchValue(db->watched_keys, key);
        if (!clients) return;

        /* Mark all the clients watching this key as REDIS_DIRTY_CAS */
        /* Check if we are already watching for this key */
        listRewind(clients,&li);
        while((ln = listNext(&li))) {
            redisClient *c = listNodeValue(ln);

            c->flags |= REDIS_DIRTY_CAS;    // 打开 FLAG
        }
    }

``touchWatchedKeysOnFlush`` 的工作和 ``touchWatchedKey`` 类似：它们都负责打开 ``REDIS_DIRTY_CAS`` 状态，但 ``touchWatchedKeysOnFlush`` 仅用于 ``FLUSH`` 或者 ``FLUSHALL`` 命令执行之后。


总结
-------

关于 ``WATCH`` 和 ``UNWATCH`` 命令的分析就到此结束了。

在 Redis 2.6 版本以前， ``WATCH`` 命令是在事务中完成 CAS （check-and-set）动作的唯一方式。

从 Redis 2.6 开始，使用脚本可以更简单方便地完成事务工作。 `Redis 官网上也说 <http://redis.io/topics/transactions>`_ 未来可能会废弃 ``MULTI`` 、 ``EXEC`` 和 ``WATCH`` 等命令，所有事务工作都交由脚本完成，这应该是一个好方向。

最后，和往常一样，带注释的完整代码可以在 GITHUB 查看： `github.com/huangz1990/reading_redis_source <https://github.com/huangz1990/reading_redis_source>`_ 。
