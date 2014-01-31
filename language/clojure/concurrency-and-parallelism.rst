.. highlight:: clojure

并发与并行
=============


延时计算
-------------

使用 ``delay`` 包裹一个给定的表达式，并产生一个延时对象。

当使用 ``deref`` 或者 ``@`` 对这个延时对象进行解引用时，被包裹的表达式才会被求值：

::

    user=> (def d (delay (println "Running...") :done!))
    #'user/d

    user=> d                    ; 未求值的 delay
    #<Delay@4f6e03: :pending>

    user=> @d                   ; 解引用，等同于调用 (deref d)
    Running...              
    :done!

    user=> @d                   ; 值被求值一次之后就会被缓存
    :done!

因为并不对被包裹的表达式进行求值，所以对 ``delay`` 语句的调用总是立即被返回。

使用 ``realized?`` 可以检查一个 ``delay`` 是否已经被解引用过：

::

    user=> (def dd (delay :done!))
    #'user/dd

    user=> (realized? dd)
    false

    user=> @dd 
    :done!

    user=> (realized? dd)
    true


并发线程
--------------

``future`` 将给定的表达式放到一个线程里执行，执行的结果使用解引用取出：

::

    user=> (def f (future 10086))            
    #'user/f

    user=> f
    #<core$future_call$reify__6110@4d6d4e: 10086>

    user=> @f
    10086

解引用是否阻塞，取决于所给定的表达式在新线程里是否已经运行完毕。

比如以下代码就会阻塞几秒钟，因为它创建了一个会阻塞的表达式，并且\ **立即**\ 对它进行解引用：

::

    user=> @(future (Thread/sleep 5000) 10086)  ; ... 需要等待 5 秒
    10086

不论被包裹表达式是否阻塞，对 ``future`` 的调用总是立即返回的，调用者的线程并不会因为 ``future`` 所包裹的表达式而阻塞。

.. note::

    为了进一步优化效率，被 ``future`` 包裹的表达式会被放到一个线程池里执行，而不是直接创建新线程。


数据流变量
------------

``promise`` 声明某个变量为一个数据流变量，表示这个变量『会在将来的某个时候拥有一个值』。

带 ``promise`` 的变量的值通过 ``deliver`` 来设置。

::

    user=> (def p (promise))
    #'user/p

    user=> p
    #<core$promise$reify__6153@638273: :pending>

    user=> (deliver p 10086)
    #<core$promise$reify__6153@638273: 10086>

    user=> @p
    10086

当对一个未有值的 ``promise`` 变量进行求值时，当前线程会被阻塞，直到有其他线程对这个变量进行 ``deliver`` 为止。


并行
-----------------

使用线程，将计算任务分配到多个内核上并行执行，从而加快计算速度。

pmap
^^^^^^

``map`` 函数的并行版本。

::

    ; 串行运行， 4 个元素，每个等待 3 秒，共等待 12 秒
    user=> (time 
             (dorun 
               (map (fn [x] (Thread/sleep 3000)) 
                    (range 4))))
    "Elapsed time: 12000.767484 msecs"
    nil

    ; 并行运行， 4 个元素，每个等待 3 秒，共等待 3 秒
    user=> (time 
             (dorun 
               (pmap (fn [x] (Thread/sleep 3000))
                     (range 4))))
    "Elapsed time: 3002.602211 msecs"
    nil

``pmap`` 使用 ``future`` 实现，所以会产生一些和线程有关的消耗，如果要处理的操作并不耗时，就不要使用 ``pmap`` ，否则反而会影响性能。


pcalls
^^^^^^^

并行运行多个无参数函数，并以惰性序列的形式返回它们的值。

::

    ; 并行运行 3 个等待 3 秒的线程，共等待 3 秒
    user=> (pcalls 
             #(Thread/sleep 3000) 
             #(Thread/sleep 3000)
             #(Thread/sleep 3000))
    (nil nil nil)


pvalues
^^^^^^^^^

并行对多个表达式进行求值，并以惰性序列的形式返回它们的值。

::

    ; 以下多个表达式的最大求值时间为 3 秒
    user=> (pvalues 
             (Thread/sleep 3000)
             10086
             (Thread/sleep 3000)
             "hello moto")
    (nil 10086 nil "hello moto")

.. note::

    因为 ``pcalls`` 和 ``pvalues`` 的返回值都是惰性序列，因此，如果有一个非常耗时的表达式阻塞在其他一些表达式的前面，那么就算这些表达式已经计算完了，它们也不能被返回。

    以下是这样一个实例，在序列前面的三个元素，可以立即被返回，但是，后面的三个元素只有等待 ``3`` 秒之后，才会被返回，尽管它们早就在并发线程里被求值完了：

    ::

       user=> (for [i (pvalues 1 2 3 (Thread/sleep 3000) 4 5 6)] (println i))
       (1
       2
       nil 3    ; 打印出这里之后，会停滞 3 秒
       nil nil
       nil 4
       nil 5
       nil 6
       nil nil)
