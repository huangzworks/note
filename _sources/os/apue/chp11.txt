.. highlight:: c

第 11 章：线程
==================

线程是一个进程中的控制和执行单元（unit），
传统的 UNIX 进程可以看成是只有一个控制线程：
在同一时刻，它只能做一件事。

通过在进程中创建多个线程，
可以并发地完成多个不同的任务。

进程的所有信息对该进程的所有线程都是共享的，
包括可执行的程序文本、程序的全局内存和堆内存、栈以及文件描述符。

多线程的用途和好处包括：

- 使用同步代码，实现异步效果。

- 实现简单的进程间通讯和数据共享。

- 将多个任务并行化，提高效率，减少阻塞。比如一个编辑器可以用一个线程接受用户的输入，另一个线程写入数据，第三个线程进行备份，等等。

当然，多线程也带来了一些难题，
其中最重要的就是因为共享数据而带来的数据一致性问题，
以及线程之间的同步，等等。

.. note::

    多线程和多处理器并没有直接关系，
    即使使用的是单处理器，也可以从多线程中获益。


创建线程
------------

``pthread_create`` 函数用于创建新线程：

::

    #include <pthread.h>

    // 创建成功返回 0 ，否则返回错误编号
    int pthread_create(pthread_t *thread,               // 保存新线程的 id
                       const pthread_attr_t *attr,      // 定制线程的属性
                       void *(*start_routine) (void *), // 线程要执行的函数
                       void *arg);                      // 传给函数的参数

和创建进程类似，
系统并不保证新线程和原有线程之间那个会先执行，
所以线程程序一定要有相应的同步措施，
并且线程所执行的程序不能依赖于特定的执行顺序。

另外，
线程相关函数出错时，
通常以返回错误代码而不是设置 ``errno`` 的方式来报告错误，
这样更简洁一些。

``pthread_self`` 函数用于返回线程的 id ，同一个进程的线程 id 是唯一的：

::

    #include <pthread.h>

    pthread_t pthread_self(void);

以下是书本提供的，创建新线程并打印线程 id 的示例代码：

.. literalinclude:: code/11-1.c

以下是代码的执行结果（记得编译的时候要带上 ``-lpthread`` ）：

::

    $ ./11-1.out 
    main thread: pid 3729 tid 3075602112 (0xb751f6c0)
    new thread:  pid 3729 tid 3075599168 (0xb751eb40)


终止线程
----------

线程有以下两种外部/自然终止方式：

- 进程中的任一线程调用了 ``exit`` 、 ``_Exit`` 或者 ``_exit`` ，那么整个进程终止。

- 如果信号的默认动作是终止进程，那么把该信号发送到某个线程会导致整个进程终止。

还有以下三种线程主动终止的方式：

- 线程执行完毕，从调用例程中返回，终止它的控制流，但不影响整个进程。

- 线程被同一进程的其他线程取消。

- 线程调用 ``pthread_exit`` 。

除非某个线程是进程的最后一个线程，
否则，
当一个线程终止时，
它所共享的进程资源不会被释放，
``atexit`` 也不会被调用。

``pthread_exit`` 的签名如下：

::

    #include <pthread.h>

    void pthread_exit(void *retval);

``retval`` 用于向使用 ``pthread_join`` 等待这个线程的另一个线程返回值。

``pthread_join`` 将调用线程阻塞，直到指定的线程终止为止，它的签名如下：

::

    #include <pthread.h>

    // 成功返回 0 ，否则返回错误代码
    int pthread_join(pthread_t thread, void **retval);

``thread`` 是等待的目标线程， ``retval`` 用于保存目标线程的返回值。

以下是书本提供的，测试线程退出的例程：

.. literalinclude:: code/11-2.c

执行结果：

::

    $ ./11-2.out 
    thread 1 returning
    thread 2 exiting
    thread 1 exit code 0
    thread 2 exit code 2


参考资料
------------

https://computing.llnl.gov/tutorials/pthreads

一个教程，很规范地列举了关于线程、mutex、条件变量等知识，以及 API 的使用方式。
