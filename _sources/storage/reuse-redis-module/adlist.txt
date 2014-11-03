.. highlight:: c

双端链表
============


适用范围
--------------------

Redis 实现的是一个典型的双端链表，
这个链表具有以下特性：

1. 带有表头和表尾指针，可以在 :math:`O(1)` 复杂度内取得表头或者表尾节点

2. 带有节点数量变量，可以在 :math:`O(1)` 复杂度内取得链表的节点数量

3. 可以通过指定 ``dup`` 、 ``free`` 和 ``match`` 函数，适应多种类型的值（或结构）

4. 带有一个链表迭代器，通过这个迭代器，可以从表头向表尾或者从表尾向表头进行迭代。


准备工作
--------------------

1. 从 Redis 源码中复制 ``adlist.c`` 、 ``adlist.h`` 、 ``zmalloc.c`` 和 ``zmalloc.h`` 四个文件到目标文件夹。

2. 将 ``zmalloc.c`` 中的 ``#include "config.h"`` 一行删掉。

3. 添加 ``#include <stddef.h>`` 到 ``zamlloc.h`` ，解决 ``size_t`` 未定义的问题。


测试驱动程序
-------------------

以下是用作节点值的空对象：

.. literalinclude:: code/adlist/object.h

.. literalinclude:: code/adlist/object.c

以下程序进行了三项测试：

1. 创建一个空双端链表，并检查它的各项属性，然后释放它

2. 创建一个非空双端链表，然后用函数（宏）迭代节点，并检查它们的值

3. 创建一个非空双端链表，然后用迭代器对列表进行迭代，并检查各个节点的值

.. literalinclude:: code/adlist/main.c


完整源码
-------------------

测试程序的完整源码可以在 `这里 <https://github.com/huangz1990/blog/tree/master/storage/reuse-redis-module/code>`_ 的 ``adlist`` 文件夹下找到。
