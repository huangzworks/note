.. highlight:: c

第 3 章：文件 I/O
========================

本章先讨论 ``open`` 、 ``read`` 、 ``write`` 、 ``lseek`` 和 ``close`` 五个函数，
再介绍 ``dup`` 、 ``fcntl`` 、 ``sync`` 、 ``fsync`` 和 ``ioctl`` 函数。

其中，
``write`` 和 ``read`` 被称为不带缓冲的 I/O （unbuffered I/O），
因为每次执行这两个函数，都需要调用内核中的一个系统调用。


文件描述符
------------

对于内核而言，所有打开的文件都通过文件描述符引用。

文件描述符是一个非符整数。
当打开一个现有文件或者创建一个新文件时，内核向进程返回一个文件描述符。
当读或写一个文件时，使用 ``open`` 或 ``creat`` 返回的文件描述符标识该文件，并将其作为参数传送给 ``read`` 或者 ``write`` 函数。

按照惯例，UNIX 系统 SHELL 将描述符 ``0`` 、 ``1`` 、 ``2`` 分别和标准输入、标准输出和标准错误输出相关联。
在遵循 POSIX 规范的程序中，它们分别被定义于 ``unistd.h`` 中的常量 ``STDIN_FILENO`` 、 ``STDOUT_FILENO`` 和 ``STDERR_FILENO`` 所代替。


打开文件
-----------

函数 ``open`` 用于打开或创建一个文件：

::

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>

    int open(const char *pathname, int flags);
    int open(const char *pathname, int flags, mode_t mode);

``flags`` 参数的值可以是多个 ``fcntl.h`` 所定义的常量的二进制或。

以下三个参数的其中一个是必须的：

===============  ====================================
常量                作用
===============  ====================================
``O_RDONLY``        只读打开
``O_WRONLY``        只写打开
``O_RDWR``          读、写打开
===============  ====================================

以下这些参数则是可选的：

===============   ========================================================================================================
常量                作用
===============   ========================================================================================================
``O_APPEND``        每次写时都将内容追加到文件的尾端。

``O_CREAT``         如果指定文件不存在，那么创建并打开它。当使用这个参数时，需要指定 ``mode`` 参数的值（见下文）。

``O_EXCL``          在和 ``O_CREAT`` 一起使用时，确保文件是由本次调用所新创建的，如果文件已存在， ``open`` 执行失败。

``O_TRUNC``         如果打开目标是一个文件，并且允许进行写入，那么清除文件内原有的所有内容。

``O_SYNC``          当对文件进行写入时，阻塞直到写入内容已经被物理地保存到硬件为止。
===============   ========================================================================================================

更多可选参数请参考 ``open`` 命令的文档。

当 ``open`` 用于创建新文件时，文件的权限由 ``mode & ~umask`` 计算得出， ``mode`` 参数的值可以是以下多个常量的二进制或（定义于 ``sys/stat.h`` ）：

==========    =============================================================================
常量            意义
==========    =============================================================================
S_IRWXU         00700 user (file owner) has  read,  write  and  execute permission

S_IRUSR         00400 user has read permission

S_IWUSR         00200 user has write permission

S_IXUSR         00100 user has execute permission

S_IRWXG         00070 group has read, write and execute permission

S_IRGRP         00040 group has read permission

S_IWGRP         00020 group has write permission

S_IXGRP         00010 group has execute permission

S_IRWXO         00007 others have read, write and execute permission

S_IROTH         00004 others have read permission

S_IWOTH         00002 others have write permission

S_IXOTH         00001 others have execute permission
==========    =============================================================================


open 函数的 O_CREAT 和 O_EXCL 参数
---------------------------------------

设置了 ``O_CREAT`` 和 ``O_EXCL`` 参数的 ``open`` 函数都可以在文件不存在时创建并打开新文件，它们之间的区别在于：
如果文件已经存在， ``O_CREAT`` 直接打开（而不创建）文件， 而 ``O_CREAT | O_EXCL`` 则会报告错误。

以下是一个使用 ``O_CREAT`` 参数的 ``open`` 函数：

.. literalinclude:: code/3-open-create-file.c

当这个程序第一次调用时，它会创建并打开 ``test-3-open-create-file.txt`` 文件，
当程序第二次执行时，它只打开已存在的文件：

::

    $ ./3-open-create-file.out 

    $ ls -l test-3-open-create-file.txt 
    -rw-r--r-- 1 huangz huangz 0  1月 21 11:41 test-3-open-create-file.txt

    $ ./3-open-create-file.out 

    $ ls -l test-3-open-create-file.txt 
    -rw-r--r-- 1 huangz huangz 0  1月 21 11:41 test-3-open-create-file.txt

以下是一个使用 ``O_CREAT | O_EXCL`` 参数的 ``open`` 函数：

.. literalinclude:: code/3-open-create-with-excl.c

当这个程序第一次调用时，它会创建并打开 ``test-3-open-create-with-excl.txt`` 文件，
当第二次调用时， ``open`` 会返回一个错误，显示文件已存在：

::

    $ ./3-open-create-with-excl.out 

    $ ls -l test-3-open-create-with-excl.txt 
    -rw-r--r-- 1 huangz huangz 0  1月 21 12:10 test-3-open-create-with-excl.txt

    $ ./3-open-create-with-excl.out 
    open fail cause file already exists.


open 的返回值
------------------

当 ``open`` 成功打开文件时，
它的返回值总是当前最小的未使用描述符值。

比如说，如果标准输入、输出和出错占用了 ``0`` 、 ``1`` 和 ``2`` 三个描述符，
那么进程第一个打开的文件的描述符必定是 ``3`` 。

以下程序打印进程首个文件描述符的值：

.. literalinclude:: code/3-print-file-descriptor.c

执行结果：

::

    $ ./3-print-file-descriptor.out 
    fd = 3


用 creat 函数创建文件
--------------------------

``creat`` 函数提供了一种创建新的空白文件的快捷方式，
它等效于调用 ``open(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode)`` ：

::

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>

    int creat(const char *pathname, mode_t mode);

以下代码展示了 ``creat`` 的用法，它创建一个空白的 ``test-3-creat`` 文件：

.. literalinclude:: code/3-creat.c

执行结果：

::

    $ ./3-creat.out 
    creat file OK

    $ ls -l test-3-creat 
    -rw-r--r-- 1 huangz huangz 0  1月 21 15:29 test-3-creat

``creat`` 的一个缺点是，它以只写方式打开所创建的问题，
如果需要既创建文件，又读写文件，最好还是用 ``open`` 函数。

以下代码的执行效果类似于 ``creat`` ，但是打开的文件可以同时用于读和写： 
``open(pathname, O_RDWR | O_CREAT | O_TRUNC, mode);`` 。


关闭文件
-----------

调用 ``close`` 可以显式地关闭一个文件描述符：

::

    #include <unistd.h>

    int close(int fd);

当一个文件描述符被关闭时，和它相关联的所有记录锁都会被移除。

如果 ``fd`` 是指向一个底层打开文件描述（underlying open file description）的最后一个文件描述符，
那么和文件描述相关的资源会被释放。

如果一个文件被 ``unlink(2)`` 函数移除过，
并且 ``fd`` 是指向这个文件的最后一个引用，
那么这个文件会被删除。

和 ``write(2)`` 函数类似，
当一个文件描述符被关闭时，系统并不保证缓存中的文件内容会被保存到物理硬件中，
如果要保证这一点，请使用 ``fsnyc(2)`` 函数。

忘记检查 ``close`` 的返回值是一种常见的编程错误。

当一个进程终止时，内核会自动关闭进程打开的所有文件，很多程序都利用了这一功能而不显式地调用 ``close`` 。

以下示例代码打开并关闭一个文件：

.. literalinclude:: code/3-close.c

执行结果：

::

    $ ./3-close.out 
    creat file OK
    close file OK


文件写入
------------

要向一个打开的文件描述符写入内容，可以使用 ``write`` 函数：

::

    #include <unistd.h>

    ssize_t write(int fd, const void *buf, size_t count);

``write`` 将 ``buf`` 中的内容写入 ``count`` 字节到 ``fd`` 所指向的文件。

``ssize_t`` 类型的返回值说明成功写入了多少字节，这个值可能出现三种情况：

1) 返回值和 ``count`` 相等，写入成功。

2) 返回值小于 ``count`` ，写入部分成功，但还有一部分内容未写入：当写入中途程序发现底层空间不足，或者写入被信号打断时，就会出现这种情况。

3) 返回值等于 ``-1`` ，写入出错，没有任何内容被写入：当 ``fd`` 不合法或者 ``buf`` 不合法时，就会出现这种情况。具体的错误保存在 ``errno`` 变量中。

对于普通文件，写操作从当前偏移量开始，
如果文件是以 ``O_APPEND`` 模式打开，那么在每次写入之前，都会原子性执行以下两个步骤：

1. 将文件的偏移量设置为文件的末尾

2. 执行写操作

在一次成功写之后，成功写入的字节数量就是文件大小增加的字节数量。

.. note:: ``write`` 执行成功并不保证数据已经被保存到磁盘，如果要保证这一点，需要执行 ``fsync`` 。

以下代码展示了 ``write`` 的用法，它创建/打开一个文件，将一个字符串写入到文件，最后关闭文件：

.. literalinclude:: code/3-write.c

执行结果：

::

    $ ./3-write.out 

    $ cat test-3-write 
    hello moto


读文件
----------

``read`` 函数用于读取文件的内容：

::

    #include <unistd.h>

    ssize_t read(int fd, void *buf, size_t count);

``read`` 从 ``fd`` 指向的文件中读取 ``count`` 字节，并将内容保存到 ``buf`` 中。 
如果 ``count`` 大于 ``SSIZE_MAX`` ，那么结果将是未定义的。

``read`` 的返回值可能是以下三种情况：

1) 返回值等于 ``count`` ：读取成功。

2) 返回值小于 ``count`` ：读取部分成功，可能读取已经到遇到文件的末尾（EOF），或者读取被信号中断 —— 前一种情况可以通过再次调用 ``read`` ，然后检查返回值是否为 ``0`` 来判断；而后一种情况则可以通过检查 ``errno`` 是否为 ``EINTR`` 来判断。

3) 返回值为 ``-1`` ：读取出错，错误值保存在 ``errno`` 。

读取操作从文件的当前偏移量开始，在成功返回之后，文件的偏移量会加上实际读到的字节数。

以下代码打开一个新的空白文件，往里写入一段文字，然后重新打开并读取和打印文件的内容：

.. literalinclude:: code/3-read.c

执行结果：

::

    $ ./3-read.out 
    file content: hello moto


文件偏移量
----------------

每个打开的文件都有一个与之相关联的“当前文件偏移量”（current file offset）。
它通常是一个非负整数，用以度量从文件开始处计算的字节数。

当读、写操作进行时，它们从当前文件偏移量开始，并使偏移量增加所读/写的字节数。

当打开一个文件时，除非指定 ``O_APPEND`` 选项，否则偏移量被初始化为 ``0`` ，也就是文件第一个字节所在的位置。

.. note:: 设置一个文件的偏移量并不引发任何 I/O 操作，后面介绍文件表的时候会说到这一点。


设置文件偏移量
-----------------

除了根据读写操作进行移动外，偏移量还可以通过 ``lseek`` 函数显示地设定：

::

    #include <sys/types.h>
    #include <unistd.h>

    off_t lseek(int fd, off_t offset, int whence);

参数 ``offset`` 的解释与参数 ``whence`` 的值有关：

- 若 ``whence`` 是 ``SEEK_SET`` ，则将该文件的偏移量设置为距文件开始处 ``offset`` 个字节。也即是，以文件头为开始，设置绝对偏移量。

- 若 ``whence`` 是 ``SEEK_CUR`` ，则将该文件的偏移量加上 ``offset`` 的值， ``offset`` 可以是正数或负数。也即是，以当前偏移量为开始，设置相对偏移量。

- 若 ``whence`` 是 ``SEEK_END`` ，则将该文件的偏移量设置为文件的长度加上 ``offset`` 的值， ``offset`` 可以是正数或负数。也即是，以文件末尾为开始，设置绝对偏移量。

``lseek`` 成功执行时返回新的文件偏移量，设置失败则返回 ``-1`` ，并将错误信息保存到 ``errno`` 。

在前面演示 ``read`` 函数的时候，程序使用了先关闭、再打开文件的办法来将文件的偏移量重置为 ``0`` ，有了 ``lseek`` 函数，这个重新打开的步骤就没有必要了：

.. literalinclude:: code/3-lseek.c

执行结果：

::

    $ ./3-lseek.out 
    file conent: hello moto


获取当前偏移量
----------------

并没有一个专门的函数来取出文件的当前偏移量，
不过这一工作可以用 ``lseek`` 函数来完成。

以下代码展示了如何获取文件的当前偏移量：

::

    off_t current_offset = lseek(fd, 0, SEEK_CUR);

这个调用利用了 ``lseek`` 的返回值：在不对偏移量进行任何修改的情况下，返回的偏移量就是当前的偏移量。


文件空洞
-----------

``lseek`` 的偏移量并不一定要小于文件的大小，
当偏移量大于文件的大小时，对该文件的下一次写将加长该文件，并在文件中构成一个空洞（hole），这一点是允许的。
位于文件中但没有写过的字节都被读为 ``0`` 

空洞并不要求在磁盘上占用储存区，无须为它分配磁盘块。

以下程序创建一个带空洞的文件：

.. literalinclude:: code/3-hole.c

使用 ``od`` 命令，并以字符串模式打开程序创建的文件，可以看到文件中的空洞：

::

    ./3-hole.out 

    $ ls -l test-3-hole 
    -rw-r--r-- 1 huangz huangz 275  1月 22 13:27 test-3-hole

    $ od -c test-3-hole 
    0000000   a   b   c   d   e   f   g  \0  \0  \0  \0  \0  \0  \0  \0  \0
    0000020  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0
    *
    0000400  \0  \0  \0  \0  \0  \0  \0  \0   h   e   l   l   o       m   o
    0000420   t   o  \0
    0000423


SEEK_DATA 和 SEEK_HOLE
-----------------------------

从 Linux 3.1 开始， ``lseek`` 函数的 ``whence`` 参数还可以是 ``SEEK_DATA`` 或者 ``SEEK_HOLE`` ，
这两个参数的作用如下：

- ``lseek(fd, offset, SEEK_DATA)`` ：将偏移量移动到下一个包含数据的位置，该位置大于等于 ``offset`` 。如果 ``offset`` 已经指向数据，那么将偏移量设置为 ``offset`` 。

- ``lseek(fd, offset, SEEK_HOLE)`` ：将偏移量移动到下一个包含空洞的位置，该位置大于等于 ``offset`` 。如果 ``offset`` 已经指向空洞，那么将偏移量设置为 ``offset`` 。如果在 ``offset`` 之后没有任何空洞，那么偏移量设置为文件的最末尾。（文件末尾也被视为是一个空洞，因为它同样以 ``0`` 结尾。）

以下程序读入一个文件，并分别打印它的首个数据偏移量，以及它的首个空洞偏移量：

.. literalinclude:: code/3-seek-data-and-hole.c

执行结果：

::

    // 要使用 GNU 扩展来支持 SEEK_DATA 和 SEEK_HOLE

    $ gcc 3-seek-data-and-hole.c -D_GNU_SOURCE -o 3-seek-data-and-hole.out

    // 读入并分析一个带空洞的文件

    $ ./3-seek-data-and-hole.out test-3-hole 
    first data offset of file test-3-hole is 0
    first hole offset of file test-3-hole is 275

    // 被读入的带空洞文件

    $ od -c test-3-hole 
    0000000   a   b   c   d   e   f   g  \0  \0  \0  \0  \0  \0  \0  \0  \0
    0000020  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0
    *
    0000400  \0  \0  \0  \0  \0  \0  \0  \0   h   e   l   l   o       m   o
    0000420   t   o  \0
    0000423

注意，程序打印的首个空洞的位置是错误的， ``275`` 是文件的末尾，但并不是第一个空洞，
另外，程序的首个数据位置似乎是正确的，但这只是个巧合！

实际上， Linux 并不支持真正的 ``SEEK_DATA`` 和 ``SEEK_HOLE`` 选项：
``SEEK_DATA`` 总是将整个文件看作是数据，所以首个数据偏移量总是 ``0`` ，
而 ``SEEK_HOLE`` 总是指向文件末尾。

具体请参考 http://lists.freebsd.org/pipermail/freebsd-fs/2011-September/012340.html 。


文件共享
-----------------

.. note:: 以下说明是概念性的，和实际实现不一定匹配。

内核使用三种数据结构表示打开的文件：

1. 每个进程在进程表中都有一个记录项，记录项中包含一张\ **文件描述符表**\ （file descriptor table），每个描述符占用一项。与每个文件描述符相关联的是：

    a) 文件描述符标识（ ``close_on_exec`` ）

    b) 指向一个文件表项的指针

2. 内核为所有打开文件维持一张\ **打开文件表**\ （open file table ，简称文件表），每个文件表项包含：

    a) 文件状态标识（读、写、追加、同步和非阻塞等标识）

    b) 当前文件偏移量

    c) 指向该文件 v 节点表项的指针

3. 每个打开文件（或设备）都有一个\ **v 节点**\ （v-node ，虚拟节点），它包含了文件类型和对次文件进行各种操作的函数指针。对于大多数文件， v 节点还包含了该文件的 i 节点（i-node ，索引节点）， i 节点包含了文件的所有者、文件长度、文件所在的设备，文件在磁盘块上的位置指针，等等。

这些信息是在打开文件时从磁盘上读入内存的，所以所有关于文件的信息都是快速可供使用的。

以下图片展示了一个打开两个文件的进程：

.. image:: image/share-file-1.png

下图是另一个例子 —— 两个不同的进程打开了一个相同的文件，两个进程各自拥有自己所对应的文件表：

.. image:: image/share-file-2.png

当进行多进程编程时，多个父子进程打开相同的文件表也是可能的：

.. image:: image/share-file-3.png

在给出这些结构之后，现在可以对前面的操作做进一步说明：

- 在完成每个 ``write`` 之后，在文件表想中的当前文件偏移量即增加所写的字节数。如果当前文件偏移量超过了当前文件长度，那么 i 节点表项中的当前文件长度被更新为当前文件偏移量（也即是，该文件加长了）。

- 如果用 ``O_APPEND`` 标识打开了一个文件，则相应标识也被设置到文件表象的文件状态标志中。每次在带有 ``O_APPEND`` 的写操作执行时，文件表象中的当前文件偏移量就被更新为 i 节点表表项中的文件长度。这使得每次写的数据都添加到文件的末尾。

- ``lseek`` 操作修改文件表项中的当前文件偏移量，它并不执行任何 I/O 操作。


原子性的追加操作
--------------------

当多个进程打开同一个文件时，一些意料之外的情况就可能发生。

一个常见的错误时，一个进程的写入内容覆盖了另一个进程的写入内容。

假设有两个进程，它们对同一个文件执行以下代码：

::

    lseek(fd, 0, SEEK_END);

    write(fd, buf, BUF_SIZE);

初看上去，程序会先移动到文件末尾，然后对其写入 ``buf`` 中的内容。但是，考虑以下执行顺序：

+-------------------------------+-------------------------------------+
| 进程 A                        |     进程 B                          |
+===============================+=====================================+
| ``lseek(fd, 0, SEEK_END);``   |                                     |
+-------------------------------+-------------------------------------+
|                               |   ``lseek(fd, 0, SEEK_END);``       |
+-------------------------------+-------------------------------------+
|                               |   ``write(fd, buf, BUF_SIZE);``     |
+-------------------------------+-------------------------------------+
| ``write(fd, buf, BUF_SIZE);`` |                                     |
+-------------------------------+-------------------------------------+

当进程 A 执行完 ``lseek`` 之后，它将自己的文件表偏移量设置为旧的末端长度 ``old_end`` 。

这时进程 B 开始运行，它同样将自己的文件表偏移量设置为 ``old_end`` ，然后执行 ``write`` ，
当写操作执行完之后， v 节点表的当前长度被更新为 ``old_end`` 加上 ``write`` 写入的字节数。

然后进程 A 恢复执行，当它调用 ``write`` 时，进程 B 写入的内容就会被覆盖。

我们称这种情况为“在写操作中包含了竞争条件”。

要避免这种情况，我们需要使用 ``O_APPEND`` 来打开文件，确保每次更新表尾偏移量和写入会原子性地执行，
这样就不需要 ``lseek`` ，从而也就避免了竞争条件。

.. note:: 任何一个需要多个函数调用的操作都不可能是原子操作，因为在两个函数调用之间，内核有可能会临时挂起该进程。

.. tip:: 原子操作（atomic operation）值的是由多部组成的操作。如果该操作原子性地执行，则要么执行完所有步骤，要么一步也不执行，不可能只执行所有步骤的一个子集。


原子性的文件创建操作
-------------------------

在创建新文件时，也可能会引发竞争条件。

考虑以下代码：

::

    if ((open(file, O_WRONLY) == -1)  && (errno == ENOENT)) {
        // file not exists
        fd = creat(file, mode);
    }

以上代码尝试以只读模式打开一个文件，如果文件并不存在，那么它就创建一个新文件。

不过，这段代码也不是原子操作，考虑以下这个执行顺序：

+-----------------------------+--------------------------------------+
|   进程 A                    |   进程 B                             |
+=============================+======================================+
| ``open(file, O_RDONLY);``   |                                      |
+-----------------------------+--------------------------------------+
|                             |   ``open(file, O_RDONLY);``          |
+-----------------------------+--------------------------------------+
|                             |   ``fd = creat(file, mode);``        |
+-----------------------------+--------------------------------------+
|                             |   ``write(fd, buf, BUF_SIZE);``      |
+-----------------------------+--------------------------------------+
| ``fd = creat(file, mode);`` |                                      |
+-----------------------------+--------------------------------------+

当进程 A 检测到文件不存在之后， 进程 B 执行，创建一个同名的文件，并写入数据，
这时进程 A 回复写入，并创建同名的新文件，进程 B 写入的数据就会被清理掉。

要原子性地创建文件，可以调用 ``open`` 函数，配合 ``O_EXCL`` 和 ``O_CREAT`` 选项，
具体的示例可以参见前面对 ``O_EXCL`` 选项的讨论。


原子性读/写给定偏移量
--------------------------

``pread`` 和 ``pwrite`` 允许程序对文件的特定偏移量进行原子性的读/写：

::

    #include <unistd.h>

    ssize_t pread(int fd, void *buf, size_t count, off_t offset);

    ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

``fd`` 、 ``buf`` 和 ``count`` 参数的作用和 ``read`` / ``write`` 相同。

``offset`` 参数指定读/写开始的偏移量，因为没有类似于 ``lseek`` 的 ``whence`` 参数，所以 ``offset`` 总是以文件头为开始的绝对偏移量。

``pread`` 和 ``read`` 、 ``pwrite`` 和 ``write`` 的区别是：

1) ``pread`` 和 ``pwrite`` 移动到 ``offset`` 参数所指定的偏移量，然后开始执行读/写操作 —— 这一过程是原子性的，无法被中断。

2) ``pread`` 和 ``pwrite`` 执行之后并不修改文件的当前偏移量。比如说 ``read(fd, buf, 10)`` 调用之后，文件的当前偏移量最多会向后移动 ``10`` 字节，但是 ``pread(fd, buf, 10, offset)`` 调用之后，文件的当前偏移量还是和调用之前一样。

``pread`` 等同于原子性地执行的以下函数调用：

::

    off_t orig;

    orig = lseek(fd, 0, SEEK_CUR);      // save origin offset

    lseek(fd, offset, SEEK_SET);
    s = read(fd, buf, len);

    lseek(fd, orig, SEEK_SET);          // restore origin offset

``pwrite`` 的定义也是类似的。

``pwrite`` 和 ``pread`` 主要用于多进程编程时，多个父子进程共享同一个文件表，以及在多线程编程时，多个线程共享同一个文件描述符表时，执行不受其他进程/线程干扰的、无锁的、对特定 ``offset`` 上的读/写操作。

以下程序演示了 ``pread`` 的调用：

.. literalinclude:: code/3-pread.c

程序首先调用 ``read`` ，并打印调用之后的当前偏移量，
然后再调用 ``pread`` ，再打印调用之后的当前偏移量。

对比可以看出， ``read`` 修改了文件的当前偏移量，而 ``pread`` 则没有：

::

    $ ./3-pread.out 3-pread.c
    after read 10 bytes from file, offset move from 0 to 10
    after pread 10 bytes from file, offset move from 10 to 10


复制文件描述符
------------------

以下函数可以用于复制一个文件描述符：

::

    #include <unistd.h>

    int dup(int oldfd);
    int dup2(int oldfd, int newfd);

    #define _GNU_SOURCE             /* See feature_test_macros(7) */
    #include <fcntl.h>              /* Obtain O_* constant definitions */
    #include <unistd.h>

    int dup3(int oldfd, int newfd, int flags);

``dup`` 复制 ``oldfd`` ，并返回一个最小可用描述符。

``dup2`` 复制 ``oldfd`` 为 ``newfd`` ，但是：

- 如果 ``newfd`` 已存在，那么先关闭它。

- 如果 ``newfd == oldfd`` ，那么 ``dup2`` 不做任何事，也不关闭 ``newfd`` 。

以上两个函数创建出来的描述符都不共享 ``close-on-excl`` 标志，复制出的描述符都关闭了这个标志。

``dup3`` 作用和 ``dup2`` 类似，但是：

- 如果 ``oldfd == newfd`` ，那么引发错误 ``EINVAL`` 。

- ``dup3`` 可以通过指定参数 ``flags`` 为 ``O_CLOEXEC`` 而强制为复制出的描述符打开 ``close-on-exec`` 标志。

以上两个函数成功调用之后，
新创建的 ``newfd`` 和原来的 ``oldfd`` 通过共享文件描述符项，从而实现文件偏移量、以及文件状态标识的共享。
在逻辑上， ``newfd``  和 ``oldfd`` 是可互换的（interchangeably）。

以下代码演示了如何用三个不同的函数来复制标准输出：

.. literalinclude:: code/3-dup.c

执行结果：

::

    $ ./3-dup.out 
    file descriptor duplicated by dup, number = 3
    file descriptor duplicated by dup2 with newfd 123, number = 123
    file descriptor duplicated by dup3 with newfd 456, number = 456


保存文件到磁盘
-------------------

为了提高 I/O 性能，当 ``write`` 等函数执行之后，写入的内容通常只是写入到了一个内存中的缓存中，并没有真正地写入到磁盘，
在一段时间间隔之后，或者缓存写满之后，缓存中的数据才会被推入输出队列。

而输出队列又等到缓存到达队首时，才真正地将缓存写入到磁盘。

如果数据在等待写入的这段时间，机器因为发生故障而停机，那么就会发生数据丢失。

对于数据库这类、在写入之后必须第一时间将数据保存到磁盘中的程序来说，必须需要使用 ``fsync`` 或 ``fdatasync`` 函数：

::

    #include <unistd.h>

    int fsync(int fd);

    int fdatasync(int fd);

``fsync`` 将指定 ``fd`` 上的文件数据、以及文件的属性保存到磁盘中，并阻塞直到保存完成。

``fdatasync`` 和 ``fsync`` 的作用类似，但是它只保证数据本身被保存到磁盘，但并不同步更新文件的属性（metadata，比如文件的被修改时间，以及文件的大小，等等），对于无须确保文件属性被同步写入的程序，这个调用的性能比同步 ``fsync``  稍高一些。

还有一个条件更弱的调用 —— ``sync`` ，它只要求将所有修改过的块缓存区（包括文件数据和文件属性）排入到写队列，然后就返回，它并不等待实际写磁盘操作结束：

::

    #include <unistd.h>

    void sync(void);

    int syncfs(int fd);

``syncfs`` 函数用于只对 ``fd`` （而不是所有打开文件）执行 ``sync`` 所要求的动作。


TODO FCNTL 笔记
-----------------------

.. todo 介绍 fcntl 函数时说明 p61 的用法。


TODO IOCTL 笔记
-----------------------
