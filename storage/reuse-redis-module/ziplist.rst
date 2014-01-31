.. highlight:: c

压缩列表
=====================


适用范围
---------------------

压缩列表(ziplist)是一系列特殊编码的内存块，
它可以用节约内存的方式保存一系列字符串和整数值，
具体的信息请参考 `《Redis 设计与实现》 <http://www.redisbook.com>`_ 的相关章节。


准备工作
---------------------

1. 将以下文件复制到目标文件夹：

   - ``endianconv.c`` 
   
   - ``endianconv.h`` 
   
   - ``fmacros.h`` 
   
   - ``util.c`` 
   
   - ``util.h``  
   
   - ``ziplist.c``  
   
   - ``ziplist.h``  
   
   - ``zmalloc.c``  
   
   - ``zmalloc.h``

2. 编辑 ``zmallo.c`` ，将其中的 ``#include "config.h"`` 语句删去。


测试驱动程序
---------------------

以下程序对 ``ziplist`` 模块进行了测试，
包括创建并释放一个空白的 ``ziplist`` ，
以及对 ``ziplist`` 进行添加、删除和查找操作：

.. literalinclude:: code/ziplist/main.c


完整源码
---------------------

完整的测试代码可以在 `这里 <https://github.com/huangz1990/blog/tree/master/storage/reuse-redis-module/code>`_ 的 ``ziplist`` 文件夹找到。
