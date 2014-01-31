.. highlight:: clojure

基本类型
==========

整数
-----

只要有需要的话，整数会在多个类型之间自动转换，以满足长度需求：

::

    user=> (class 10086)          
    java.lang.Long

    user=> (class 10086000000000000000000000000000000)
    clojure.lang.BigInt


有限精度浮点数
----------------

Clojure 默认使用有限精度的浮点数，类型为 ``Double`` ：

::

    user=> (class 3.14)
    java.lang.Double


无限精度浮点数
----------------

无限精度浮点数需要在数值的最后加上后缀 ``M`` ：

::

    user=> (class 3.144444444444444444444444444444444444444444444444444444444444444444444444M)
    java.math.BigDecimal

无限精度浮点数的类型为 ``BigDecimal`` 。


有理数
-------

为了进一步加强对高精度计算的支持， Clojure 提供了有理数类型：

::

    user=> (class (/ 1 3))
    clojure.lang.Ratio

在进行高精度计算时，使用有理数会比使用无限精度浮点数得到更精确的结果。


字符
-----

单个字符用 ``\`` 作为前缀来表示：

::

    user=> \a
    \a

    user=> (class \a)
    java.lang.Character


字符串
-------

字符串使用 ``"`` 符号包围：

::

    user=> "i'm a string"
    "i'm a string"

    user=> (class "i'm a string")
    java.lang.String


关键字
------

一个关键字总是指向它自身，因为这个原因，它可以用作是比字符串更高效的比对手段：

::

    user=> :keyword
    :keyword

    user=> (class :keyword)
    clojure.lang.Keyword

    user=> (identical? :keyword :keyword)   ; 对象比对
    true

    user=> (= :keyword :keyword)            ; 值比对
    true


符号
-----

符号在求值时，返回的是它在当前上下文中所指向的值：

::

    user=> (def number 10086)   ; 定义一个变量
    #'user/number

    user=> 'number  ; 一个符号值
    number

    user=> (eval 'number)   ; 求值符号值
    10086


nil
------

``nil`` 是一个特殊值，它表示“空”，如果一个函数不返回任何值，那么它返回 ``nil`` 。

::

    user=> nil
    nil

    user=> (class nil)
    nil

    user=> (prn)      

    nil


布尔
------

布尔值包含 ``true`` 和 ``false`` 两个值：

::

    user=> true
    true

    user=> false
    false

    user=> (class true)
    java.lang.Boolean

    user=> (class false)
    java.lang.Boolean

在 Clojure 中，除了 ``nil`` 和 ``false`` 之外，其他任何值都被看作是 ``true`` ，这和 C 语言和其他一些函数式语言稍有不同，需要注意：

::

    user=> (false? nil)
    false

    user=> (false? false)
    true

    user=> (false? 0)
    false

    user=> (false? "")
    false

    user=> (false? (list))
    false


函数
-----

函数也是 Clojure 中的第一类对象，因此它也是其中一种基本类型值：

::

    user=> (fn [] "hello moto")
    #<user$eval94$fn__95 user$eval94$fn__95@155bb17>

稍后的章节会介绍更多和函数相关的内容。


元数据
-------

元数据（metadata）是一个映射，可以添加到一个符号或者一个 collection 当中，用作编译提示，或者给开发者提供额外的信息。

``with-meta`` 用于为对象添加元数据：

::

    user=> (def s (with-meta 'a-symbol {:author "huangz"}))
    #'user/s

    user=> s
    a-symbol

以上代码为符号 ``s`` 添加了包含 ``:author`` 信息的元数据。

``meta`` 函数用于取出对象的元数据：

::

    user=> (meta s)
    {:author "huangz"}


正则表达式
------------

正则表达式可以通过 reader 宏 ``#"pattern"`` 在读入期创建，或者使用 ``re-pattern`` 函数在运行时创建，这两种方式都会创建一个 ``java.util.regexp.Pattern`` 对象。

::

    user=> (class #"pattern")
    java.util.regex.Pattern

    user=> (re-seq #"[0-9]+" "abs123def345ghi567")
    ("123" "345" "567")

    user=> (re-seq #"\w+" "one-two/three")
    ("one" "two" "three")

``re-seq`` 函数惰性地返回字符串中的所有匹配。
