基本数据库操作
===================


查看数据库
--------------

使用 ``SHOT DATABASES`` 命令来查看服务器现有的数据库：

::

    MariaDB [(none)]> SHOW DATABASES;
    +--------------------+
    | Database           |
    +--------------------+
    | information_schema |
    | mysql              |
    | performance_schema |
    +--------------------+
    3 rows in set (0.02 sec)


使用数据库
-------------

使用 ``USE <db_name>`` 命令可以将目标数据库设置为操作对象：

::

    MariaDB [(none)]> USE mysql
    Reading table information for completion of table and column names
    You can turn off this feature to get a quicker startup with -A

    Database changed
    MariaDB [mysql]> 

注意 ``USE`` 是一条特殊的命令：

- 它不需要分号结尾，不过如果你喜欢的话，加上分号也是无害的；

- 它必须在一行之内结束，不能换行。

另外要注意的是客户端的提示符，
在没有指定数据库的时候，
提示符显示的是 ``MariaDB [(none)]>`` ，
而当我们指定了 ``mysql`` 数据库之后，
提示符变成了 ``MariaDB [mysql]>`` 。


创建数据库
-----------------

使用命令 ``CRATE DATABASE <db_name>`` 可以创建一个新的数据库：

::

    MariaDB [(none)]> CREATE DATABASE menagerie;
    Query OK, 1 row affected (0.00 sec)

    MariaDB [(none)]> SHOW DATABASES;
    +--------------------+
    | Database           |
    +--------------------+
    | information_schema |
    | menagerie          |  # <--- 新创建的数据库
    | mysql              |
    | performance_schema |
    +--------------------+
    4 rows in set (0.00 sec)

创建一个新的数据库并不会自动将这个新的数据库设置为目标数据库，
所以要使用这个新数据库的话，
还需要再执行一条 ``USE menagerie`` 命令。

另外，
在启动 MariaDB 的时候，
也可以使用：

::

    mysql -h <host> -u <user> -p <db_name>

的方式在启动客户端时指定数据库，
这和启动客户端之后再执行 ``USE <db_name>`` 的效果一样。
