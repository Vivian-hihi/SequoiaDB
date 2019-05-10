##MySQL 实例组件手工升级指导##

假如升级前 MySQL 实例组件中已添加端口号为3306，数据路径为/opt/sequoiasql/mysql/data3306的数据库实例，那么升级至版本3.2步骤如下：

1. 卸载之前的mysql，保留数据目录

2. 安装3.2版本的SequoiaSQL-MySQL

3. 切换目录和用户

  ```lang-javascript
  # cd /opt/sequoiasql/mysql
  # su sdbadmin
  ```

4. 将旧的数据目录data3306进行备份

  ```lang-javascript
  # mv data3306 data3306_bk
  ```

5. 添加实例名为mysqld3306的数据库实例

  ```lang-javascript
  # bin/sdb_sql_ctl addinst mysqld3306 -p 3306 -D data3306/
  ```

6. 停止实例

  ```lang-javascript
  # bin/sdb_sql_ctl stop mysqld3306
  ```

7. 备份新新的实例数据目录下的auto.cnf

  ```lang-javascript
  # mv data3306/auto.cnf data3306/auto.cnf_bk
  ```

8. 将备份数据拷贝到新的实例目录下，并将上一步操作中备份的配置文件还原

  ```lang-javascript
  # cp -r data3306_bk/* data3306
  # mv data3306/auto.cnf_bk data3306/auto.cnf
  ```

9. 将之前安装路径下my.cnf端口号为3306的数据库实例的配置项合入data3306/auto.cnf
10. 启动实例

  ```lang-javascript
  # bin/sdb_sql_ctl start mysqld3306
  ```