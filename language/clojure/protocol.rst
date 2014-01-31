.. highlight:: clojure

协议
======

Clojure 中的协议用于为类型定义一集操作。

协议中的每个操作用一个方法（method）来定义，这个定义只提供方法的签名，但是并不包含该方法的实现。

举个例子，以下名为 ``QueueOperation`` 的协议定义了队列类型常见的 ``push`` 和 ``pop`` 操作：

::

    (defprotocol QueueOperation
        "队列的推入和弹出操作。"
        (push [queue item] 
              "将元素 item 推入队列 queue 中，
              新元素的摆放位置由队列的类型（FIFO/LIFP）来决定。"
        )
        (pop [queue]
             "从队列 queue 中弹出一个元素，
             弹出元素由队列的类型来决定，
             如果队列为空，返回 nil 。"
        )
    )

因为一个协议可以被多种类型实现，而方法的分派（dispath）正是由方法的第一个参数的类型来决定的，因此协议中的方法必须至少带有一个参数（例子中是 ``queue`` ）。


实现协议
-----------

前一节说过，协议可以由不同的类型来实现。

在前面介绍记录的时候我们说过，记录拥有自己的类型，这里我们先来定义一个记录，再为这个类型实现给定的协议。

一个基本的 FIFO （先进先出）队列的定义如下：

::

    (defrecord FifoQueue [container])

我们可以用一个向量作为这个 FIFO 队列的容器，从而创建 ``FifoQueue`` 记录的实例：

::

    user=> (def q (->FifoQueue [1 2 3]))
    #'user/q

    user=> q
    #user.FifoQueue{:container [1 2 3]}

要正确地实现这个 ``FifoQueue`` ，程序必须在调用 ``push`` 时将新元素放到向量的末尾，比如 ``(push q 4)`` 应该会返回一个 ``:container`` 为 ``[1 2 3 4]`` 的记录。

另一方面， ``pop`` 操作被调用时，程序也需要将 ``:container`` 内的第一个元素弹出，比如说 ``(pop q)`` 应该会返回数字 ``3`` ，因为它是容器内的向量的最后一个元素。

以下就是 ``FifoQueue`` 类型对 ``QueueOperation`` 协议的实现：

::

    (extend-type FifoQueue  ; 如果在 repl 中测试，那么全名为 user.FifoQueue
        QueueOperation
            (push [queue item]
                (let [
                        vector (:container queue)
                        new-vector (conj vector item)
                     ]
                    (->FifoQueue new-vector)
                )
            )
            (pop [queue]
                (let [vector (:container queue)]
                    (first vector)
                )
            )
    )

以下代码测试了 ``FifoQueue`` 的实现：

::

    user=> (def q (->FifoQueue [1 2 3]))
    #'user/q

    user=> (push q 4)
    #user.FifoQueue{:container [1 2 3 4]}

    user=> (pop q)
    1
