.. highlight:: c

第 4 章：基本 TCP 套接字编程
================================

getsockname & getpeername
--------------------------------

一个用 ``getsockname`` 和 ``getpeername`` 写成的打印服务器和客户端 IP 和端口的程序。

客户端：

.. literalinclude:: code/chp4/test_getsockname_getpeername_client.c

运行客户端：

::

    $ ./client 127.0.0.1
    connect ok

服务器：

.. literalinclude:: code/chp4/test_getsockname_getpeername_server.c

运行服务器：

::

    $ ./server 
    Server ip is 0.0.0.0 , port is 52350 .
    Client ip is 127.0.0.1 , port is 60308 .

