.. highlight:: c

第 3 章：套接字编程简介
============================== 

IP echo 程序
--------------

用 ``inet_pton`` 和 ``inet_ntop`` 写的一个程序，
接收一个 ``ddd.ddd.ddd.ddd`` 格式的 IP 地址，
然后将它转换成 ``sockaddr_in`` 结构，
然后再从结构中转换出一个 IP 地址，
并打印出来。

如果一切正常的话，
打印的 IP 应该和输入的 IP 一样：

.. literalinclude:: code/chp3/ip_echo.c

运行：

::

    $ ./a.out 123.123.123.123
    The ip you input is: 123.123.123.123

