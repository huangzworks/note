.. highlight:: clojure

引用类型
=========


通用设施
---------

所有引用类型都支持 watch 和 validator 两种机制，以下是对这两种机制的介绍。

watch
^^^^^^^

Watch 监视给定的引用类型，并在引用类型的值改变时，执行给定的函数。

``add-watch`` 负责关联 watch 函数和引用类型，每个传给 ``add-watch`` 的函数都接受四个参数：

1. ``key`` ：因为一个引用类型可以关联多个 watch 函数，所以需要使用一个 ``key`` 来标识各个不同的 watch
2. ``ref`` ：发生改变的引用类型本身
3. ``old-state`` ：引用类型的旧值
4. ``new-state`` ：引用类型的新值

而 ``add-watch`` 本身接受三个函数：

1. 被关联的引用类型
2. watch 函数的 key
3. watch 函数

::

    user=> (defn show-change [key ref old-state new-state]
             (println "Watcher Key " key)
             (println "Watcher Ref " ref)
             (println "Watcher Old State " old-state)
             (println "Watcher New State " new-state))
    #'user/show-change

    user=> (def a (atom {}))
    #'user/a

    user=> (add-watch a :show-info-when-value-change show-change)
    #<Atom@96d02d: {}>

    user=> (swap! a assoc :key "value")
    Watcher Key  :show-info-when-value-change
    Watcher Ref  #<Atom@96d02d: {:key value}>
    Watcher Old State  {}
    Watcher New State  {:key value}
    {:key "value"}

一个引用类型可以关注多个 watch 函数，它们在引用类型的值被改变时同步地运行（执行的顺序不明，API文档没提到）：

::

    user=> (def a (atom {}))
    #'user/a

    user=> (add-watch a :first-watcher (fn [key ref old new] (println "first watcher call")))
    #<Atom@1ba5790: {}>

    user=> (add-watch a :second-watcher (fn [key ref old new] (println "second watcher call")))
    #<Atom@1ba5790: {}>

    user=> (swap! a assoc :key "value")
    second watcher call
    first watcher call
    {:key "value"}

``remove-watch`` 函数用于移除关联的 watch 。

传给 ``remove-watch`` 的 ``key`` 参数需要和 ``add-watch`` 调用时指定的 ``key`` 一致：

::

    user=> (def a (atom {}))
    #'user/a

    user=> (add-watch a :watcher (fn [key ref old new] (println "watcher call")))
    #<Atom@11008e: {}>

    user=> (swap! a assoc :key "value")
    watcher call
    {:key "value"}

    user=> (remove-watch a :watcher)
    #<Atom@11008e: {:key "value"}>

    user=> (reset! a {})
    {}
