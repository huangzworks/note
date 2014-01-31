.. highlight:: clojure

命名空间
================

命名空间（namespace）保存了从符号到 Var 或 Class 的映射。

在默认情况下，用 ``def`` 或其变种创建的任何 Var 都被添加到当前命名空间中，不过，通过使用 ``require`` 、 ``refer`` 或者 ``use`` 等函数，也可以引用其他命名空间的符号。


带前缀和不带前缀的名字
-------------------------

如果在命名空间里可以直接调用给定的名字而不必携带任何前缀，那么我们称这个名字为“不带前缀的名字”（unqualified name）。

如果调用名字时需要显式地自定它的命名空间或者包名，那么称这个名字为“带前缀的名字”（qualified name）。

::

    user=> map
    #<core$map clojure.core$map@1b50639>

    user=> (require 'clojure.set)
    nil

    user=> clojure.set/intersection
    #<set$intersection clojure.set$intersection@1ba7dab>

在上面的代码中， ``map`` 就是不带前缀的，而 ``intersection`` 则是带前缀的。


创建命名空间
--------------

宏 ``ns`` 用于创建一个新的命名空间，它默认载入所有 ``java.lang`` 中的类和 ``clojure.core`` 中的所有函数。

::

    user=> (ns my-new-ns)
    nil

    my-new-ns=> String
    java.lang.String

    my-new-ns=> map
    #<core$map clojure.core$map@47c1bf>

``ns`` 通常用在程序源码的开头，用于定义命名空间，以及指定要载入的模块：

::

    (ns foo.bar
        (:refer-clojure :exclude [ancestors printf])
        (:require (clojure.contrib sql combinatorics))
        (:use (my.lib this that))
        (:import (java.util Date Timer Random)
                 (java.sql Connection Statement)))

对于 REPL 环境，创建命名空间最好使用 ``in-ns`` ，它只载入 ``java.lang`` 中的所有类，这样就可以最大限度地避免函数重名：

::

    my-new-ns=> (in-ns 'hello-moto)
    #<Namespace hello-moto>

    hello-moto=> String
    java.lang.String

    hello-moto=> map    ; 不包含 clojure.core
    CompilerException java.lang.RuntimeException: Unable to resolve symbol: map in this context, compiling:(NO_SOURCE_PATH:0) 


require
---------

载入包，但是并不将包中的符号加入到当前命名空间的映射当中。

因此，要使用被载入的包里面的符号，需要使用完整符号名(fully qualified name)，如 ``package_name/symbol_name`` 。

::

    user=> (require 'clojure.set)
    nil

    ; 使用 fully qualified 格式引用包中的名字
    user=> (clojure.set/intersection #{1 2 3} #{3 2 1})
    #{1 2 3}

    ; 证实 clojure.set 里的名字没有加入到当前映射当中
    user=> (intersection #{1 2 3} #{3 2 1}) 
    CompilerException java.lang.RuntimeException: Unable to resolve symbol: intersection in this context, compiling:(NO_SOURCE_PATH:5) 

选项 ``:as`` 可以为包指定一个别名：

::

    user=> (require '[clojure.set :as s]) 
    nil

    user=> (s/intersection #{1 2 3} #{3 2 1})
    #{1 2 3}

``require`` 还提供了以下选项：

- ``:reload`` 强制重载所有已经载入的包。
- ``:reload-all`` 强制重载所有已经载入的包，无论是间接还是直接使用 ``use`` 或 ``require`` 载入。
- ``:verbose`` 打印载入包的相关信息。

::

    user=> (require '[clojure.set :verbose true])
    (clojure.core/load "/clojure/set")
    nil

    user=> (require '[clojure.set :reload true :verbose true])
    (clojure.core/load "/clojure/set")
    nil


refer 
------

将包中的符号加入到当前命名空间当中，被指定的包必须先使用 ``require`` 函数载入。
 
被载入的符号可以用 unqualified name 格式使用。

::

    ; 试图载入没有被 require 的命名空间，失败
    user=> (refer 'clojure.set)
    Exception No namespace: clojure.set  clojure.core/refer (core.clj:3761)

    ; 先 require 进指定的命名空间
    user=> (require 'clojure.set)
    nil

    user=> (clojure.set/intersection #{1 2 3} #{3 2 1})
    #{1 2 3}

    ; 再进行 refer 
    user=> (refer 'clojure.set)
    nil

    ; 现在可以用 unqualified 格式直接使用 clojure.set 中的名字了
    user=> (intersection #{1 2 3} #{3 2 1})
    #{1 2 3}

``refer`` 提供了以下选项：

- ``:exclude`` ：一个符号列表，包含不加入到映射的符号名。
- ``:only`` ：一个符号列表，包含要被加入到映射的符号名。
- ``:rename`` ：一个符号 map ，指定要改名的符号名。

::

    user=> (require 'clojure.set)
    nil

    ; 载入 intersection 符号并将它改名成 inter
    user=> (refer 'clojure.set :only '[intersection] :rename {'intersection 'inter})
    nil

    user=> (inter #{1 2 3} #{3 2 1})
    #{1 2 3}


use
------
 
载入包和包中所有公开（public）的符号，之后可以使用 fully qualified name 或 unqualified 两种格式来引用被载入的符号。
  
效果类似于同时使用 ``require`` 函数和 ``refer`` 函数。

::

    user=> (use 'clojure.set)
    nil

    user=> (intersection #{1 2 3} #{3 2 1})
    #{1 2 3}

    user=> (clojure.set/intersection #{1 2 3} #{3 2 1})
    #{1 2 3}

``require`` 函数和 ``refer`` 函数的选项都适用于 ``use`` 函数。


import
---------

``import`` 载入给定的 JAVA 类：

::

    user=> (import java.util.Date)
    java.util.Date

    user=> (str (Date.))    ; 创建一个 Date 实例并打印它
    "Thu Oct 11 14:30:31 CST 2012"

