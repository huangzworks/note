指数补偿 —— Exponential backoff
===================================

指数补偿指的是，在执行事件时，通过反馈，逐渐降低某个过程的速率，从而最终找到一个合适的速率（来处理事件）。

指数补偿通常用于网络和传输协议，比如在进行网络连接时，如果第一次请求失败，那么可以等待 :math:`t_1` 之后重试，如果再次请求还是失败，那么等待 :math:`t_2` 之后重试。。。

重试可以一直继续下去，或者等待次数或等待时间超过特定值为止。

等待的时间 :math:`t_n` 可以是随机选择，也可以随着重试的次数而逐渐加大，诸如此类。

详细内容请参考维基百科的 `Exponential backoff <http://en.wikipedia.org/wiki/Exponential_backoff>`_ 词条。

应用
-------

《UNIX 环境高级编程，第二版》（APUE，2E） 16.4 节提供了一个带重试的 ``socket`` 连接程序，
如果连接失败，
那么程序就睡眠一段时间再尝试，
每失败一次睡眠的时间就延长一些：

.. code-block:: c

    #include "apue.h"
    #include <sys/socket.h>

    #define MAXSLEEP 128

    int
    connect_retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
    {
        int nsec;

        for (nsec = 1; nsec < MAXSLEEP; nsec <<=1) {
            if (connect(sockfd, addr, alen) == 0) {
                // connect accepted.
                return 0;
            }
            if (nsec <= MAXSLEEP/2)
                sleep(nsec);
        }

        return -1;
    }

