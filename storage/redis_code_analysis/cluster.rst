.. highlight:: c

集群（cluster）
=====================

本次注释包含 ``cluster.h`` 文件和 ``cluster.c`` 文件，
源文件来自 Redis 项目 ``unstable`` 分支的最新文件，
最后一次提交来自：

::

    commit e78938425536748e63932ccebb7248f6389db102
    Author: antirez <antirez@gmail.com>
    Date:   Mon Dec 23 12:48:39 2013 +0100
    
两个注释文件的代码都可以在 https://github.com/huangz1990/blog/blob/master/storage/redis_code_analysis/cluster 找到。

cluster.h
^^^^^^^^^^^^^^^^

.. literalinclude:: cluster/cluster.h

cluster.c
^^^^^^^^^^^^^^^^^

.. literalinclude:: cluster/cluster.c
