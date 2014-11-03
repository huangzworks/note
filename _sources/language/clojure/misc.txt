.. highlight:: clojure

杂项
=========


文件载入
----------

对一些小程序进行原型测试的时候，通常要在 REPL 中进行一些重复的输入，一个更好的办法是将程序保存进某个文件，然后在 REPL 中通过载入文件来避免重复输入。

``load-file`` 函数读入并求值指定文件。

假如现在有文件 ``greet.clj`` ，内容如下：

::

    ;;; greet.clj

    (defn greeting []
        (str "hello"))

    (greeting)

之后可以使用 ``load-file`` 对它进行读入和求值：

::

    user=> (load-file "greet.clj")
    "hello"


延迟求值
----------

Clojure 提供了一组原语，用于实现延迟求值。

``delay`` 延缓对一个表达式的求值，直到对它调用 ``force`` 为止:

::

    user=> (def d (delay (+ 1 1)))
    #'user/d

    user=> d
    #<Delay@14fa3ef: :pending>

    user=> (class d)
    clojure.lang.Delay

    user=> (force d)
    2

    user=> d
    #<Delay@14fa3ef: 2>

    user=> (class d)
    clojure.lang.Delay


异常的捕捉与处理
------------------

Clojure 的异常捕捉和处理沿用了 JAVA 的 ``try/catch/finally`` 套路，只是外观上略有不同，它的格式为：

::

    (try 
        main-expr
        (catch exception-class exception-instance catch-expr*)
        (finally finally-expr*)
    )

如果在 ``main-expr`` 执行的过程中，没有异常发生，那么返回 ``main-expr`` 的求值结果作为整个 ``try`` 表达式的值。

反之，如果有异常产生，那么程序就会将这个异常和所提供的 ``catch`` 语句进行匹配，看所抛出异常的类型是否为 ``exception-class`` 。

如果找到一个匹配正确的 ``catch`` 语句的话，那么就将这个异常实例和局部变量 ``exception-instance`` 绑定，这个局部变量可以在 ``catch-expr*`` 内使用。

当有异常发生时，被匹配 ``catch`` 语句的 ``catch-expr*`` 表达式的值就是整个 ``try`` 语句的返回值；如果没有提供 ``catch-expr*`` ，那么返回 ``nil`` 。

``catch`` 语句可以有多个，它们按从先到后的顺序被一个个匹配，如果所有语句都匹配不成功，那么程序向外部传递异常。

``finally`` 是可选的，最多只能有一个 ``finally`` 语句：它包围一些表达式，用它们的副作用来执行抛出异常之后要完成的动作，比如关闭数据库，等等。

举个例子，以下代码试图使用 ``Integer.`` 来将 ``"not a integer"`` 转换为整数，但这个字符串并不是合法的输入，于是引发异常：

::

    user=> (Integer. "not a integer")
    ; NumberFormatException For input string: "not a integer"  
    ; java.lang.NumberFormatException.forInputString (NumberFormatException.java:65)

以下代码会捕捉异常，并打印出一个字符串格式的异常信息：

::

    user=> 
    (try
        (Integer. "not a integer")
        (catch java.lang.NumberFormatException err (str err))
    )
    "java.lang.NumberFormatException: For input string: \"not a integer\""
