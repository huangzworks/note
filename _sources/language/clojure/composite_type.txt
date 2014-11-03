.. highlight:: clojure

复合类型
=========

Clojure 中的复合类型被统称为 collection ，所有 collection 都是不可改（immutable）和持久化（persistent）的。

所有 collection 都支持 ``count`` 、 ``conj`` 和 ``seq`` 三种操作。 ``count`` 返回 collection 中元素的个数，而 ``conj`` 则将一个新元素添加到 collection 当中， ``seq`` 返回一个 collection 的序列，可以对这个序列进行多种操作；各种符合类型的序列表现稍有不同。

本章先介绍复合类型，序列在稍后的其他章节会详细介绍。


列表
-----

列表通常使用 ``list`` 函数来构建：

::

    user=> (def l (list 'a 'b 'c))
    #'user/l

    user=> l
    (a b c)

函数 ``peek`` 取出列表的头元素，而 ``pop`` 取出除头元素外的其他元素：

::

    user=> (peek l)
    a

    user=> (pop l)
    (b c)

``conj`` 将一个新元素添加到列表的头部：

::

    user=> (conj l 'alpha)
    (alpha a b c)


向量
-----

向量以下标来索引元素。

可以用 ``vector`` 函数来创建一个向量：

::

    user=> (def v (vector 'a 'b 'c))
    #'user/v

    user=> v    
    [a b c]

获取向量的元素可以使用 ``get`` 函数，下标从 ``0`` 开始：

::


    user=> (get v 0)
    a

    user=> (get v 1)
    b

如果 ``get`` 所传入的下标超出给定向量的下标，那么返回 ``nil`` ：

::

    user=> (get v 10086)
    nil

``nth`` 和 ``get`` 稍有不同，它在给定下标不存在时返回 ``nil`` ：

::

    user=> (nth v 10086)
    IndexOutOfBoundsException   clojure.lang.PersistentVector.arrayFor (PersistentVector.java:106)

要避免 ``nth`` 抛出异常，要显式地指定下标不存在时返回的默认值：

::

    user=> (nth v 10086 nil)
    nil

向量本身也可以作为函数来使用：

::

    user=> (v 0)
    a

    user=> (v 1)
    b

``assoc`` 函数可以用于修改向量的已存在元素，或者添加新元素到向量的最后：

::

    user=> (assoc v 3 'd)   ; 索引等于向量的长度时，执行添加操作
    [a b c d]

    user=> (assoc v 0 'A)   ; 索引小于向量的长度时，执行修改操作
    [A b c]


映射
-----

映射（Map）分为哈希映射（hash-map）和有序映射（sorted-map）两种，哈希映射是无序的，而有序映射则按 key 从小到大来排列。

``hash-map`` 函数用于创建一个哈希映射：

::

    user=> (def h (hash-map :k1 "value 1" :k2 "value 2"))
    #'user/h

    user=> h
    {:k1 "value 1", :k2 "value 2"}

映射本身可以作为函数来使用，获取给定 key 所对应的值，如果给定 key 不存在，那么返回 ``nil`` ：

::

    user=> (h :k1)
    "value 1"

    user=> (h :not-exists-key)
    nil

``get`` 函数也可以用于获取映射的值：

::

    user=> (get h :k2)
    "value 2"
    user=> (get h :not-exists-key)
    nil

``assoc`` 函数用于给映射增加一个或多个 key - value 对：

::

    user=> (assoc {:k1 "value 1"} :k2 "value 2")
    {:k2 "value 2", :k1 "value 1"}

    user=> (assoc {:k1 "value 1"} :k2 "value 2" :k3 "value 3")
    {:k3 "value 3", :k2 "value 2", :k1 "value 1"}

``dissoc`` 函数用于从映射中移除一个或多个 key - value 对：

::

    user=> (dissoc {:k1 "value 1" :k2 "value 2" :k3 "value 3"} :k1)
    {:k3 "value 3", :k2 "value 2"}

``sorted-map`` 和 ``sorted-map-by`` 两个函数用于创建有序映射。其中 ``sorted-map`` 使用默认的从小到大顺序来对 key 进行排序，而 ``sorted-map-by`` 则需要显式地传入对比操作符（comperator）：

::

    user=> (sorted-map 1 :a 2 :b 3 :c)
    {1 :a, 2 :b, 3 :c}

    user=> (sorted-map-by > 1 :a 2 :b 3 :c) ; 显式传入 > 号，将排序变为从大到小
    {3 :c, 2 :b, 1 :a}

映射还有 ``merge`` 、 ``zipmap`` 、 ``contains?`` 和 ``find`` 等常用操作。

还有一种数组映射（array map），这种映射按照创建的顺序将一个个 key - value 对包含在数组里，主要用于参数传递等映射规模非常小的情况：

::

    user=> {:a 1 :b 2 :c 3}     
    {:a 1, :c 3, :b 2}

    user=> (class {:a 1 :b 2 :c 3})
    clojure.lang.PersistentArrayMap


集合
-----

集合保证自身的每个元素都是唯一的。

集合分为 ``hash-set`` 和 ``sorted-set`` 两种，它们的关系类似于前面介绍的 ``hash-map`` 和 ``sorted-map`` ：前者是无序的，而后者按元素来排序。

``hash-set`` 可以通过集合的文法，或者显式调用 ``hash-set`` 来创建：

::

    user=> #{ :a :b :c }
    #{:a :c :b}

    user=> (class #{ :a :b :c })
    clojure.lang.PersistentHashSet

    user=> (hash-set :a :b :c)
    #{:a :c :b}

``conj`` 用于添加元素到集合， ``disj`` 则用于移除集合中的给定元素：

::

    user=> (conj #{:a} :b)
    #{:a :b}

    user=> (disj #{:a :b} :b)
    #{:a}

``get`` 函数在集合包含给定元素时返回元素本身，如果元素不存在，返回 ``nil`` ：

::

    user=> (def s #{:a :b :c})
    #'user/s

    user=> (get s :a)
    :a

    user=> (get s :not-exists-element)
    nil

获取元素的工作同样可以使用集合本身来进行：

::

    user=> (s :a)
    :a

    user=> (s :not-exists-element)
    nil

除以上介绍的操作外，集合还支持 ``union`` 、 ``difference`` 和 ``intersection`` 等常用操作。
