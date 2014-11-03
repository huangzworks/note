.. highlight:: clojure

记录
======


类型的必要性
----------------

在 Clojure 中，通常用 Map 来表示键-值对的数据，比如说，以下 Map 可以用于表示一条 tweet 信息：

::

    user=> (def t {:user "huangz" :content "hello world"})
    #'user/t

在概念上，这个 Map 可以表示一条 tweet ，但是，从技术上来看，它只是一个 Map 而已：

::

    user=> (type t)
    clojure.lang.PersistentHashMap

在大多数时候，用 Map 来表示数据已经足够了，但是，在一些情况下，可能需要为一些数据创建新的类型，让它和其他结构相同的数据区别开来。

比如说，以下两个 Map 分别表示一个人和一只猫，但是，因为它们共享同样的结构，所以很容易会让人误以为是同类型的数据：

::

    user=> (def a-man {:name "peter" :age 25})
    #'user/a-man

    user=> (def a-cat {:name "mimi" :age 5})    
    #'user/a-cat

在这种情况下，为了避免混淆，就需要创建新的类型来表示特定的数据了。


记录的创建和使用
--------------------

Clojure 中的记录（record）可以用于创建带类型的键-值对数据结构，它的定义形式如下：

::

    (defrecord RecordName [key1 key2 ...])

为了和函数名和宏名区别开来，记录的名字通常以大驼峰的形式给出。

继续前面的例子，对于人和猫，可以给出定义以下记录：

::

    (defrecord Man [name age])

    (defrecord Cat [name age])

定义记录之后，可以用 ``->RecordName`` 的形式创建相应记录的实例：

::

    user=> (def a-real-man (->Man "peter" 25))
    #'user/a-real-man

    user=> (def a-real-cat (->Cat "mimi" 5))
    #'user/a-real-cat

    user=> a-real-man
    #user.Man{:name "peter", :age 25}

    user=> a-real-cat
    #user.Cat{:name "mimi", :age 5}

可以用 ``(:field record)`` 的形式取出记录的域：

::

    user=> (:name a-real-man)
    "peter"

    user=> (:age a-real-man)
    25

.. warning::

    Map 可以用 ``(:field map)`` 和 ``(map :field)`` 两种方式来取出域。

    而记录的域只能用 ``(:field record)`` 的形式来取出。

    以下是一个错误的例子：

    ::

        user=> (a-real-man :name)
        ClassCastException user.man cannot be cast to clojure.lang.IFn  user/eval47 (NO_SOURCE_FILE:13)

和前面 Map 表示的例子一样， ``Man`` 和 ``Cat`` 两个记录含有同样的两个域： ``name`` 和 ``age`` 。

但是，记录和用 Map 表示的数据不一样，它拥有自己的类型：

::

    user=> (type a-real-man)
    user.Man

    user=> (type a-real-cat)
    user.Cat

    user=> (= (type a-real-man) (type a-real-cat))
    false

这样就不会再混淆人和猫的数据了。
