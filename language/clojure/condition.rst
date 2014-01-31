.. highlight:: clojure

条件求值
===========

if
----

``if`` 接受三个单行表达式， ``test`` 、 ``then`` 和 ``else`` 。

如果 ``test`` 部分为真，那么就执行 ``then`` 部分；如果 ``test`` 部分为假，那么执行 ``else`` 部分。

::

    user=> (if true   
             "is true"
             "is false")
    "is true"

    user=> (if false 
             "is true"
             "is false")
    "is false"

``else`` 部分并不是必须的，如果省略 ``else`` 部分，那么条件为假时返回 ``nil`` 。

::

    user=> (if true "is true")
    "is true"

    user=> (if false "is true")
    nil


do
------

``do`` 可以包围任意数量的表达式，并按顺序求值这些表达式，然后返回最后一个表达式的值作为 ``do`` 的返回值，如果 ``do`` 里面没有表达式，那么返回 ``nil`` 。

::

    user=> (do 
             (prn "hello")
             (prn "moto"))
    "hello"
    "moto"
    nil

利用 ``do`` ，我们可以让 ``if`` 也能处理多条语句：

::

    user=> (if true
             (do
                (prn "hello")
                (prn "moto")))
    "hello"
    "moto"
    nil


when
--------

``when`` 和 ``if`` 不同，它只处理条件部分为真时的情况，而且 ``when`` 的体内可以包括多条语句，而不是像 ``if`` 那样，只能处理单条语句：

::

    user=> (when true
             (prn "hello")
             (prn "moto"))
    "hello"
    "moto"
    nil

当条件部分为假时， ``when`` 只是简单地返回 ``nil`` ：

::

    user=> (when false
             (prn "hello")
             (prn "moto"))
    nil

正如你所想象的， ``when`` 实际上就是 ``if`` 和 ``cond`` 组合构成得出的。   


cond
--------

``cond`` 接受任意数量的 ``test`` - ``exp`` 对，按顺序对各个 ``test`` 进行测试，并求值第一个为真 ``test`` 所对应的 ``exp`` 表达式；如果没有任何 ``test`` 为真，那么返回 ``nil`` 。

::

    (cond true "moto")  ; => "moto"

    (cond false "hello"
          true "moto")  ; => "moto"

    (cond false "hello"
          true "moto"
          true "google")    ; => "moto"

    (cond false "hello")    ; => nil


if-not 、 when-not
----------------------

``if-not`` 和 ``when-not`` 处理跟 ``if`` 和 ``when`` 相反的情况，它们只是 ``(if (not ...))`` 和 ``(when (not ...))`` 的一个快捷方式。

::

    user=> (if-not false "moto")
    "moto"

    user=> (when-not false "hello")
    "hello"


if-let 、 when-let
----------------------

``if-let`` 和 ``when-let`` 处理在条件判断前，先要创建局部变量的情况；它们分别是 ``(if (let ...))`` 和 ``(when (let ...))`` 的快捷方式：

::

    (if-let [test true]        
        "is true"
        "is false"
    )

    ; => "is true"

    (when-let [test true]
        "is true"
    )
    ; => "is true"

要注意的一点是， ``if-let`` 允许创建多个局部变量，而 ``when-let`` 只能创建一个局部变量（这是由 ``when`` 的性质决定的）。

.. for

.. recur

.. loop

.. while
