.. highlight:: c

整数集合
=============


适用范围
-----------------------

整数集合提供了这样一个抽象：

1. 集合可以保存 ``int_16`` 、 ``int_32t`` 和 ``int_64`` 三种不同长度的整数。

2. 对集合保存值所使用的字长是由程序自动调整的，这个过程被称为“升级”。

3. 所有整数在集合中都是独一无二的，各个整数以从小到达的顺序在集合中排序，所以程序在集合中查找元素的复杂度为 :math:`O(\log N)` 。

整数集合的详细信息和操作 API 可以参考 `《Redis 设计与实现》 <http://www.redisbook.com>`_ 的相关章节。


准备步骤
-----------------------

1. 从 Redis 源码中复制 ``endianconv.c`` 、 ``endianconv.h`` 、 ``intset.c`` 、 ``intset.h`` 、 ``zmalloc.c`` 和 ``zmalloc.h`` 到目标文件夹。

2. 将 ``#include "config.h"`` 从 ``zmalloc.c`` 中删除。

3. 将 ``#include <stddef.h>`` 加入到 ``intset.h`` ，解决 ``size_t`` 未定义的问题。


测试驱动程序
-----------------------

以下程序对整数集合进行了三项测试：

1. 创建并删除一个空集合

2. 对集合进行添加、删除和查找操作

3. 检查整数集合的升级状态

.. literalinclude:: code/intset/main.c


完成源码
------------------------

测试程序的完整源码可以在 `这里 <https://github.com/huangz1990/blog/tree/master/storage/reuse-redis-module/code>`_ 的 ``intset`` 文件夹找到。
