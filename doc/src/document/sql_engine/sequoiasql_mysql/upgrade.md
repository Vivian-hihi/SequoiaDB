##MySQL 实例组件手工升级指导##

假如升级前 MySQL 实例组件中已添加端口号为 3306，数据库实例路径为 /opt/sequoiasql/mysql/database/3306，那么升级的步骤如下：

1. 如果是从 3.2 版本升级到更高版本的 MySQL，直接安装新版本的 MySQL 即可。执行安装命令需要指定 installmode 参数，详细的参数介绍见命令的 help 信息；

  ```lang-bash
  # ./sequoiadb-x.x.x-linux_x86_64-installer.run --installmode upgrade
  ```

2. 如果是从 3.2 版本以下的版本升级到 3.2 版本以及更高版本的 MySQL，则按照以下的步骤执行；

3. 卸载之前版本的 MySQL，保留数据目录；

4. 安装 3.2 版本的 MySQL；

5. 切换目录和用户；

  ```lang-bash
  # cd /opt/sequoiasql/mysql
  # su sdbadmin
  ```

6. 将旧的数据目录 database/3306 进行备份；

  ```lang-bash
  # mv database/3306 database/3306_bk
  ```

7. 添加实例名为 mysqld3306 的数据库实例；

  ```lang-bash
  # bin/sdb_sql_ctl addinst mysqld3306 -p 3306 -D database/3306
  ```

8. 停止实例；

  ```lang-bash
  # bin/sdb_sql_ctl stop mysqld3306
  ```

9. 备份新新的实例数据目录下的 database/3306/auto.cnf；

  ```lang-bash
  # mv database/3306/auto.cnf database/3306/auto.cnf_bk
  ```

10. 将备份数据拷贝到新的实例目录下，并将上一步操作中备份的配置文件还原；

  ```lang-bash
  # cp -r database/3306_bk/* database/3306
  # mv database/3306/auto.cnf_bk database/3306/auto.cnf
  ```

11. 将之前安装路径下 my.cnf 端口号为 3306 的数据库实例的配置项合入 database/3306/auto.cnf；

12. 启动实例。

  ```lang-bash
  # bin/sdb_sql_ctl start mysqld3306
  ```
