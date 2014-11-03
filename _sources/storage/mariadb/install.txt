安装
===========

访问 MariaDB 网站上的 Repository Configuration Tool ，根据你的系统配置选择最合适的安装方式： https://downloads.mariadb.org/mariadb/repositories 。

如果一切正常的话，那么安装成功之后应该可以看到 MariaDB 的版本号：

::

    $ mysql --version
    mysql  Ver 15.1 Distrib 10.0.9-MariaDB, for debian-linux-gnu (x86_64) using readline 5.1

.. note:: 为了兼容性考虑，在很多系统上面，安装好的 MariaDB 都继续沿用 ``mysql`` 作为程序名。

启动 MariaDB 试试：

::

    $ mysql -u root -p
    Enter password: 
    Welcome to the MariaDB monitor.  Commands end with ; or \g.
    Your MariaDB connection id is 34
    Server version: 10.0.9-MariaDB-1~wheezy-log mariadb.org binary distribution

    Copyright (c) 2000, 2014, Oracle, Monty Program Ab and others.

    Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

    MariaDB [(none)]> 

