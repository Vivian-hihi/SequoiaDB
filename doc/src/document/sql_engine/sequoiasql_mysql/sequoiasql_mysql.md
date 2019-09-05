##描述##

  MySQL 实例组件的服务 sequoiasql-mysql 以守护进程 sequoiasql-mysql-daemon 的形式存在的。 该服务在启动时，会自动拉起相关的实例，在实例进程异常退出时，也会自动拉起实例。

一个安装对应一个 sequoiasql-mysql 服务，一台机器上存在多个安装时系统服务名为 sequoiasql-mysql[i]，i 为小于 50 的数值或者为空。 

##数据库服务相关参数说明##

- 以root用户登录目标机器

1.  启动服务，拉起守护进程 sequoiasql-mysql-daemon 和相关数据库实例进程；

    ```lang-bash
    # service sequoiasql-mysql start
    ```

2.  停止服务，停止守护进程 sequoiasql-mysql-daemon 和相关数据库实例进程；

    ```lang-bash
    # service sequoiasql-mysql stop
    ```

3.  查看服务状态，系统提示 running 表示服务正在运行；

    ```lang-bash
    # service sequoiasql-mysql status
    ```

4.  重启服务，重启守护进程 sequoiasql-mysql-daemon 和相关数据库实例进程；

    ```lang-bash
    # service sequoiasql-mysql restart
    ```  
    