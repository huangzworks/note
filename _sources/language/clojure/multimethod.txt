.. highlight:: clojure

多态方法
===========

Clojure 通过多态方法（multimethod）来对运行时多态进行支持。

一个多态方法包含一个分派函数（dispatching function），以及一个或多个方法（method）。每个方法对应并处理一个分派值（dispatching value），分派值由分派函数产生。

如果一个分配值没有对应的方法，那么它的分配值为默认分派值（默认为 ``:default``\ ），如果多态方法没设置默认方法，那么抛出错误。


创建多态方法
--------------

``defmulti`` 用于定义多态方法，并设置分派函数。

::

    (defmulti complier :os)

以上代码定义了多态方法 ``complier`` ，并定义 ``:os`` 为它的分派函数。

``defmethod`` 创建并安装适用于给定分派值的新方法到给定的多态方法，以下代码定义了两个 ``complier`` 方法，它们的分派值分别是 ``::unix`` 和 ``::osx`` ：

::

    (defmethod complier ::unix [m]
        (get m :complier)
    )

    (defmethod complier ::oxs [m]
        (get m :complier)
    )

对于不同的数据，多态方法 ``complier`` 返回不同的值：

::

    user=> (complier {:os ::unix :complier "cc"})
    "cc"

    user=> (complier {:os ::osx :complier "gcc"})
    "gcc"


派生
-------

可以注意到，前面用于处理 ``::unix`` 和 ``::osx`` 的方法实现实际上是完全一样的，这意味着，我们应该可以通过重用，减少其中一个方法的定义 —— ``drive`` 函数就是用来完成这事的：它可以让某个分派值的方法派生（derive）另一个分派值的方法，从而达到实现重用的目的。

比如说，我们大可不必为 ``::unix`` 和 ``::osx`` 两个分派值都编写方法，而是以让分派值为 ``::osx`` 的方法派生分派值 ``::unix`` 的方法，达到重用实现的目的：

::

    (defmethod complier ::unix [m]
        (get m :complier)
    )

    (derive ::osx ::unix)

测试新的多态方法：

::

    user=> (complier {:os ::unix :complier "cc"})
    "cc"

    user=> (complier {:os ::osx :complier "gcc"})
    "gcc"

有一系列函数，比如 ``isa?`` 、 ``instance?`` 、 ``parents`` 、 ``ancestors`` 、 ``descendants`` 等函数，可以用于检查分派值之间的继承关系：

::

    user=> (isa? ::osx ::unix)
    true

    user=> (parents ::osx)
    #{:user/unix}

    user=> (parents ::unix)
    nil
    
    user=> (ancestors ::osx)
    #{:user/unix}

    user=> (descendants ::unix)
    #{:user/osx}



默认分派值
-----------

如果一个分派值没有对应的方法，并且该多态方法也没有设置默认方法，那么抛出一个错误：

::

    user=> (complier {:os ::linux :complier "gcc"})
    IllegalArgumentException No method in multimethod 'complier' for dispatch value: :user/linux  clojure.lang.MultiFn.getFn (MultiFn.java:121)

要防止这种错误，可以为多态方法设置默认方法。默认方法的设置非常简单，只要为分派值 ``:default`` 设置方法就可以了：

::

    (defmethod complier :default [m]
        (str "maybe is " (get m :complier))
    )   

现在，来试试默认方法：

::

    user=> (complier {:os ::linux :complier "gcc"})
    "maybe is gcc"

    user=> (complier {:os ::windows :complier "vc"})
    "maybe is vc"


解决冲突
-----------

有时候，一个方法可能同时派生了两个方法，在调用这个方法时，就会产生冲突，因为 Clojure 不知道应该使用哪个方法。

考虑以下例子，分派值为 ``::c`` 的方法分别派生了分派值 ``::a`` 和 ``::b`` 的两个方法：

::

    (defmulti msg :tag)

    (defmethod msg ::a [m]
        "this is a"
    )

    (defmethod msg ::b [m]
        "this is b"
    )

    (derive ::c ::a)
    (derive ::c ::b)

这时分派值 ``::c`` 的方法调用就会产生冲突：

::

    user=> (msg {:tag ::c})
    IllegalArgumentException Multiple methods in multimethod 'msg' match dispatch value: :user/c -> :user/b and :user/a, and neither is preferred  clojure.lang.MultiFn.findAndCacheBestMethod (MultiFn.java:136)

要解决这种冲突，需要通过 ``prefer-method`` 函数显式调整两个祖先方法的优先级，让它们的其中一个优先成为 ``::c`` 分派值的方法，从而解决方法冲突：

::

    (defmulti msg :tag)

    (defmethod msg ::a [m]
        "this is a"
    )

    (defmethod msg ::b [m]
        "this is b"
    )

    (derive ::c ::a)
    (derive ::c ::b)

    (prefer-method msg ::a ::b)     ; 新增

上面新增的代码表示，分派值 ``::a`` 的方法优先级高于分派值 ``::b`` 的方法，当出现二义性(ambiguous)时，就会优先使用分派值为 ``::a`` 的方法。

现在，再次调用 ``::c`` 分派值的方法，它就会使用 ``::a`` 分派值的方法：

::

    user=> (msg {:tag ::c})
    "this is a"
