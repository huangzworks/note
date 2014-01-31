源码的编译和使用
=========================

APUE 的源码本身带有一些错误，需要修改之后才能正常编译。

以下方法在 Ubuntu 12.10 上测试通过。


步骤
------

1. 到 www.apuebook.com 下载源码

2. ``tar`` 解包， ``cd apue.2e`` 

3. 编辑 ``Make.defines.linux`` ，修改变量 ``WKDIR`` ，指向你放置 apue 源码的位置，我的是 ``/home/huangz/code/apue.2e`` ，所以设置为 ``WKDIR=/home/huangz/code/apue.2e`` 

4. 编辑 ``include/apue.h`` ，增加一个常量 ``ARG_MAX`` ： ``#defines ARG_MAX 4096`` ， ``threadctl/getenv1.c`` 和 ``threadctl/getenv3.c`` 要用到这个常量； ``4096`` 只是参考值，可以视情况按需修改。

5. 编辑 ``threadctl/getenv1.c`` 增加一行： ``#include "apue.h"``

6. 编辑 ``threadctl/getenv3.c`` 增加一行： ``#include "apue.h"`` 

7. 编辑 ``threads/badexit2.c`` ，修改第 31 行，将 ``pthread_self()`` 的返回值转换为 ``int`` 类型： ``printf("thread 2: ID is %d\n", (int)pthread_self());`` 。

8. 编辑 ``std/linux.mk`` ，将两个 ``nawk`` 替换为 ``gawk`` 。

9. ``make`` 

10. 将相应的文件复制到库位置里： ``sudo cp include/apue.h /usr/include`` ，以及 ``sudo cp lib/libapue.a /usr/lib`` 。

至此，所有步骤执行完成了。别忘了在编译的时候，要让编辑器链接 ``apue`` 库：

::

    gcc xxxx.c -lapue


参考资料
-----------

http://hi.baidu.com/crazyboymx/blog/item/d3520bde28d79172d0164eb9.html
