YUM
====

Yum 的定义：

    Yum is the The Fedora Project package manager 
    that is able to query for information about packages, 
    fetch packages from repositories, 
    install and uninstall packages using automatic dependency resolution, 
    and update an entire system to the latest available packages. 
    
    Yum performs automatic dependency resolution on packages you are updating, 
    installing or removing, and thus is able to automatically determine, 
    fetch and install all available dependent packages. 


包更新
-------

检查系统安装了的软件包是否有更新可用：

::

    ~]# yum check-update
    Loaded plugins: langpacks, presto, refresh-packagekit
    # 包名.架构                          可安装更新的版本号            更新包的来源
    PackageKit.x86_64                    0.6.14-2.fc15                 fedora
    PackageKit-command-not-found.x86_64  0.6.14-2.fc15                 fedora
    PackageKit-device-rebind.x86_64      0.6.14-2.fc15                 fedora
    PackageKit-glib.x86_64               0.6.14-2.fc15                 fedora
    PackageKit-gstreamer-plugin.x86_64   0.6.14-2.fc15                 fedora
    PackageKit-gtk-module.x86_64         0.6.14-2.fc15                 fedora
    PackageKit-gtk3-module.x86_64        0.6.14-2.fc15                 fedora
    PackageKit-yum.x86_64                0.6.14-2.fc15                 fedora
    PackageKit-yum-plugin.x86_64         0.6.14-2.fc15                 fedora
    gdb.x86_64                           7.2.90.20110429-36.fc15       fedora
    kernel.x86_64                        2.6.38.6-26.fc15              fedora
    rpm.x86_64                           4.9.0-6.fc15                  fedora
    rpm-libs.x86_64                      4.9.0-6.fc15                  fedora
    rpm-python.x86_64                    4.9.0-6.fc15                  fedora
    yum.noarch                           3.2.29-5.fc15                 fedora

更新单个包可以使用命令：

::

    yum update <package_name>

比如：

::

    yum update udev

如果被更新的包有相关联的包，
并且这些关联包也有更新可用，
那么这些关联包将被一并更新。

在一般情况下， 
yum 总会询问你是否确认要进行包更新：

::

    ~]# yum update udev
    Loaded plugins: langpacks, presto, refresh-packagekit

    # ...

    Total download size: 1.9 M
    Is this ok [y/N]:

而在更新包的时候给定 ``-y`` 参数就可以让更新自动确认，
无需再输入 ``y`` ：

::

    yum update <package_name> -y

要一次性更新所有包，可以直接输入：

::

    yum update


包搜索
--------

如果你只记得包的某些关键词，
但是忘记了包的具体名字，
那么可以使用：

::

    yum search term...

来搜索带有特定词语的包。

比如：

::

    ~]# yum search meld kompare
    Loaded plugins: langpacks, presto, refresh-packagekit
    ============================== N/S Matched: meld ===============================
    meld.noarch : Visual diff and merge tool
    python-meld3.x86_64 : HTML/XML templating system for Python

    ============================= N/S Matched: kompare =============================
    komparator.x86_64 : Kompare and merge two folders

      Name and summary matches only, use "search all" for everything.


列举包信息
-----------

使用命令 ``yum list`` 可以列出给定包、包组和 repo 的相关信息，
这个命令允许用户使用 Glob 表达式来过滤信息：

::

    yum list <glob_expression>

比如执行以下命令将列出所有以 ``hp`` 开头的包信息：

::


    ~]# yum list hp*

而以下命令将列出所有与 ``kernel`` 和 ``module`` 有关的包信息：

::

    ~]# yum list kernel module 

使用命令 ``yum list all`` 可以列出所有 *已安装* 以及 *可用* 的软件包：

::

    ~]# yum list all
    Loaded plugins: langpacks, presto, refresh-packagekit
    Installed Packages
    ConsoleKit.x86_64                       0.4.4-1.fc15                  @fedora
    ConsoleKit-libs.x86_64                  0.4.4-1.fc15                  @fedora
    ConsoleKit-x11.x86_64                   0.4.4-1.fc15                  @fedora
    GConf2.x86_64                           2.32.3-1.fc15                 @fedora
    GConf2-gtk.x86_64                       2.32.3-1.fc15                 @fedora
    ModemManager.x86_64                     0.4-7.git20110201.fc15        @fedora
    NetworkManager.x86_64                   1:0.8.998-4.git20110427.fc15  @fedora
    NetworkManager-glib.x86_64              1:0.8.998-4.git20110427.fc15  @fedora
    NetworkManager-gnome.x86_64             1:0.8.998-4.git20110427.fc15  @fedora
    NetworkManager-openconnect.x86_64       0.8.1-9.git20110419.fc15      @fedora
    [output truncated]

而使用 ``yum list installed`` 和 ``yum list available`` 则分别用于列出已安装的软件包和可用的软件包。

使用 ``yum grouplist`` 可以列出所有包组，
而 ``yum repolist`` 则可以列出所有已启用的（enabled） repo 的 ID 、名字和包数量。


查看包信息
-------------

使用 ``yum info <package_name>`` 可以查看给定包的详细信息：

::

    ~]# yum info abrt
    Loaded plugins: langpacks, presto, refresh-packagekit
    Installed Packages
    Name        : abrt
    Arch        : x86_64
    Version     : 2.0.1
    Release     : 2.fc15
    Size        : 806 k
    Repo        : installed
    From repo   : fedora
    Summary     : Automatic bug detection and reporting tool
    URL         : https://fedorahosted.org/abrt/
    License     : GPLv2+
    Description : abrt is a tool to help users to detect defects in applications and
                : to create a bug report with all informations needed by maintainer
                : to fix it. It uses plugin system to extend its functionality.


包安装
---------

使用命令 ``yum install <package_name>`` 可以安装给定的包，
比如以下命令将安装软件 ``sqlite`` ：

::

    yum install sqlite

通过给定多个包名就可以一次安装多个包：

::

    yum install sqlite mysql redis mongodb


包组安装
--------

以下是包组的定义：

::

    A package group is similar to a package: 
    it is not useful by itself, 
    but installing one pulls a group of dependent packages 
    that serve a common purpose. 

简单来说，
包组就是一个为了实现某些功能而组成的具有依赖关系的多个包。

使用 ``yum grouplist -v <glob_expression>`` 可以列出某个包组的信息，
其中列出了包组的名字和 ID ：

::

    ~]# yum grouplist -v kde\*

    # ...

    Available Groups:
        KDE Plasma Workspaces (kde-desktop-environment)
    Done

这里的 ``KDE Plasma Workspaces`` 就是包组名，
而 ``kde-desktop-environment`` 则是包组的 ID 。

只要知道了包组名或者包组 ID ，
就可以使用 ``yum groupinstall <group_name | group_id>`` 来安装给定的包组。

以下两个命令都可以安装 KDE 桌面包组：

::

    yum groupinstall "KDE Plasma Workspaces"

    yum groupinstall kde-desktop-environment


包移除和包组移除
-----------------

使用命令 ``yum remove <package_name>`` 可以移除给定的包，
而使用命令 ``yum groupremove <group_name>`` 则可以移除给定的包组。


查看 YUM 事务历史
------------------

查看 yum 执行的各项操作，
具体参考： http://docs.fedoraproject.org/en-US/Fedora/21/html/System_Administrators_Guide/sec-Yum-Transaction_History.html
