.. highlight:: c

第 1 章：简介
==================

基本的时间服务器和客户端构建。

服务器代码：

.. literalinclude:: code/chp1/daytimetcpsrv.c

运行服务器：

::

    $ sudo ./daytimetcpsrv 
    time server start working!

客户端代码：

.. literalinclude:: code/chp1/daytimetcpcli.c

运行客户端：

::

    $ ./daytimetcpcli 127.0.0.1
    Tue Jun 25 11:52:54 2013
