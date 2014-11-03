.. highlight:: c

字典
=========


适用范围
--------------

Redis 的字典实现具有以下特色：

1. 可以通过设置不同的类型特定函数，让字典的键和值包含不同的类型，并为键使用不同的哈希算法。

2. 字典可以自动进行扩展或收缩，并且由此而带来的 rehash 是渐进式地进行的。

字典的相关设计和操作 API 可以参考 `《Redis 设计与实现》 <http://www.redisbook.com>`_ 的相关章节。


准备工作
--------------

1. 将 ``dict.h`` 、 ``dict.c`` 、 ``fmacros.h`` 、 ``zmalloc.h`` 和 ``zmalloc.c`` 文件复制到目标文件夹。

2. 将 ``zmalloc.c`` 中的 ``#include "config.h"`` 注释掉。


类型特定函数
--------------

Redis 字典允许使用者为字典的键和值设置不同的类型特定函数，
从而使字典可以包含各种不同类型的键和值，
这些函数可以通过 ``dictType`` 类型定义：

::

    // 摘录自 dict.h

    /*
     * 特定于类型的一簇处理函数
     */
    typedef struct dictType {

        // 计算键的哈希值函数
        unsigned int (*hashFunction)(const void *key);

        // 复制键的函数
        void *(*keyDup)(void *privdata, const void *key);

        // 复制值的函数
        void *(*valDup)(void *privdata, const void *obj);

        // 对比两个键的函数
        int (*keyCompare)(void *privdata, const void *key1, const void *key2);

        // 键的释构函数
        void (*keyDestructor)(void *privdata, void *key);

        // 值的释构函数
        void (*valDestructor)(void *privdata, void *obj);

    } dictType;

以下 ``dictType`` 定义了一个非常简单的，
键和值都是整数的字典类型：

.. literalinclude:: code/dict/int_dict_type.h

.. literalinclude:: code/dict/int_dict_type.c


测试驱动程序
---------------------

以下测试驱动程序使用了上面提到的整数字典类型，
并对字典的创建、键值对添加和删除等操作进行了测试。

.. literalinclude:: code/dict/main.c


完整源码
--------------------

测试程序的完整源码可以在 `这里 <https://github.com/huangz1990/blog/tree/master/storage/reuse-redis-module/code>`_ 的 ``dict`` 文件夹下找到。
