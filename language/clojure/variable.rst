.. highlight:: clojure

定义变量
==========

Cljure 中的变量被称为 Var ：每个 Var 以一个符号命名，并持有一个值。

Var 的定义默认被所有线程所共享的，在某个线程里更改一个 Var 并不会改变这个 Var 原有的定义（这一定义被称为根绑定，root binding）。


全局变量
----------

``def`` 特殊形式（special form）用于创建一个全局变量（global var）。

::

    user=> (def x 10086)
    #'user/x

    user=> x
    10086

还可以给全局变量添加文档：

::

    user=> (def x "just a value, nothing else." 10086)
    #'user/x

    user=> (doc x)
    -------------------------
    user/x
      just a value, nothing else.
      nil


私有变量
-----------

私有变量可以通过元数据的 ``private`` 关键字来声明：

::

    user=> (def ^{:private true} v 123)
    #'user/v

    user=> (meta #'v)   ; 查看变量的私有变量
    {:ns #<Namespace user>, :name v, :private true, :line 36, :file "NO_SOURCE_PATH"}

如果一个变量是私有的，那么其他命名空间就不可以在外部中引用这个变量。


局部变量
---------------

局部变量由 ``let`` 创建：

::

    user=> (let [v 123]
             v)
    123

``let`` 中的多个声明按从先到后的顺序求值，因此，靠后的声明可以引用前面的声明：

::

    user=> (let [a 1            ; 1
                 b (+ a 1)      ; 2
                 c (+ a b)]     ; 3
             c)
    3

当多个 ``let`` 嵌套时，嵌套最深的变量绑定先被使用：

::

    user=> (let [a 123]
             (let [a 10086]
                a))
    10086

在这种情况下，我们说，嵌套内部的 ``a`` 遮蔽（shadowed）了嵌套外部的 ``a`` 。


动态变量
-----------

默认情况下，Clojure 使用静态作用域来处理变量，使用 ``dynamic`` 元数据关键字，以及 ``binding`` 宏，可以将变量改为动态作用域：

::

    user=> (def ^:dynamic x 1)
    #'user/x

    user=> (def ^:dynamic y 2)
    #'user/y

    user=> (+ x y)
    3

    user=> (binding [x 123 y 123]   ; 使用动态绑定覆盖原来的绑定
             (+ x y))
    246

    user=> (+ x y)
    3


单次赋值
---------

``defonce`` 提供了 ``def`` 的一个变种：当前仅当给定符号还没有绑定时，它才为这个符号设置绑定：

::

    user=> (defonce s "a string")
    #'user/s

    user=> s
    "a string"

    user=> (defonce s "another string")     ; 设置无效
    nil

    user=> s
    "a string"
