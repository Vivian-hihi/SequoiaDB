##MySQL 实例组件手工升级指导##

假如升级前 MySQL 实例组件中已添加端口号为 3306，数据库实例路径为 /opt/sequoiasql/mysql/database/3306，那么升级至版本 3.2 的步骤如下：

1. 卸载之前版本的 MySQL，保留数据目录

2. 安装 3.2 版本的 SequoiaSQL-MySQL

3. 切换目录和用户

  ```lang-bash
  # cd /opt/sequoiasql/mysql/database
  # su sdbadmin
  ```

4. 将旧的数据目录 3306 进行备份

  ```lang-bash
  # mv 3306 3306_bk
  ```

5. 添加实例名为 mysqld3306 的数据库实例

  ```lang-bash
  # bin/sdb_sql_ctl addinst mysqld3306 -p 3306 -D 3306/
  ```

6. 停止实例

  ```lang-bash
  # bin/sdb_sql_ctl stop mysqld3306
  ```

7. 备份新新的实例数据目录下的 auto.cnf

  ```lang-bash
  # mv 3306/auto.cnf 3306/auto.cnf_bk
  ```

8. 将备份数据拷贝到新的实例目录下，并将上一步操作中备份的配置文件还原

  ```lang-bash
  # cp -r 3306_bk/* 3306
  # mv 3306/auto.cnf_bk 3306/auto.cnf
  ```

9. 将之前安装路径下 my.cnf 端口号为 3306 的数据库实例的配置项合入 3306/auto.cnf

10. 启动实例

  ```lang-bash
  # bin/sdb_sql_ctl start mysqld3306
  ```