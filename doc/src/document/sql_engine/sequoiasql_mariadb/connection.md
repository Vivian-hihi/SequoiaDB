## 连接 MariaDB 实例与数据库分布式存储引擎
+ **配置 SequoiaDB 连接地址**

   默认的 SequoiaDB 连接地址为“localhost:11810”，可通过两种方式修改。例如，设置实例 myinst 连接地址为“sdbserver1:11810”和“sdbserver2:11810”。以下均为默认安装路径，用户根据实际情况修改。
   
   + 通过`bin/sdb_sql_ctl`指定实例名修改

      1.进入 MariaDB 安装目录。

      ```lang-bash
      $cd /opt/sequoiasql/mariadb
      ```

      2.修改 SequoiaDB 连接地址。

      ```lang-bash
      $ bin/sdb_sql_ctl chconf myinst --sdb-conn-addr=sdbserver1:11810,sdbserver2:11810
      Changing configure of instance myinst ...
      ok
      ```

   + 通过配置文件修改。
   
      进入该实例目录，修改配置文件。

      ```lang-bash
      $cd /opt/sequoiasql/mariadb/database/6101
      $vim auto.cnf
      ```
      
      修改配置文件如下：

      ```lang-bash
      ...
      sequoiadb_conn_addr=sdbserver1:11810,sdbserver2:11810
      ...
      ```

       >   **Note:**
       >    
       >   目前 sdb_sql_ctl 工具仅支持一些简单配置项的修改，建议用户采用配置文件的方式修改配置，修改方式同上。具体配置参考[引擎配置](sql_engine/sequoiasql_mariadb/config.md#引擎配置)章节。

+ **登录 MariaDB shell** 
 
   MariaDB 支持基于 UNIX 域套接字文件和 TCP/IP 的连接方式。
 
   + UNIX 套接字文件连接  
      
      进程间通信，不需要使用网络协议，比 TCP/IP 传输效率更高，但仅限于本地连接，连接时指定对应的套接字文件。

      ```lang-bash
      $ cd /opt/sequoiasql/mariadb
      $ bin/mysql -S database/6101/mysqld.sock -u root
      ```

      > **Note:** SequoiaSQL-MariaDB 实例默认无密码，所以无需输入 -p 选项。

   + TCP/IP 连接方式  
   
      网络通信，可以本地连接（环回接口）和远程连接，同时可以灵活地配置和授权客户端 IP 的访问权限。
      + 本地连接  

         ```lang-bash
         $ cd /opt/sequoiasql/mariadb
         $ bin/mysql -h 127.0.0.1 -P 6101 -u root
         ```
         
         >   **Note:**
         >    
         >   本地连接在设置密码前，不支持 TCP/IP 的连接方式。
        
      + 远程连接  
         
         MariaDB 默认未授予远程连接的权限，所以首先需要在服务端对客户端 IP 进行访问授权，以下例子对所有的 IP 都授权访问，且设置授权密码123456。

         ```lang-sql
         MariaDB [(none)]> GRANT ALL PRIVILEGES ON *.* TO root@'%' IDENTIFIED BY '123456' WITH GRANT OPTION;
         MariaDB [(none)]> FLUSH PRIVILEGES;
         ```

         假设 mariadb 服务器地址为“sdbserver1:6101”，在客户端可以这样远程连接：

         ```lang-bash
         $ /opt/sequoiasql/mariadb/bin/mysql -h sdbserver1 -P 6101 -u root
         ```

+ **设置密码**

   可以为本地连接的用户设置密码，也可以为远程连接的用户设置密码。例如为远程连接的 root 用户设置密码 123456。

   ```lang-sql
   MariaDB [(none)]> ALTER USER root@'%' IDENTIFIED BY '123456';
   ```
   
   注意，设置密码后，后续登录 MariaDB shell 的时候要指定 -p 参数，回车并输入密码。

## 基本操作示例
+ **创建数据库实例**

 ```lang-sql
 MariaDB [(none)]> CREATE DATABASE company;
 MariaDB [company]> USE company;
 ```

+ **创建表**

 ```lang-sql
 MariaDB [company]> CREATE TABLE employee(id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(128), age INT);
 MariaDB [company]> CREATE TABLE manager(employee_id INT, department TEXT, INDEX id_idx(employee_id));
 ```

+ **基本数据操作**

 ```lang-sql
 MariaDB [company]> INSERT INTO employee(name, age) VALUES("Jacky", 36);
 MariaDB [company]> INSERT INTO employee(name, age) VALUES("Alice", 18);
 MariaDB [company]> INSERT INTO manager VALUES(1, "Wireless Business");
 MariaDB [company]> SELECT * FROM employee ORDER BY id ASC LIMIT 1;
 MariaDB [company]> SELECT * FROM employee, manager WHERE employee.id=manager.employee_id;
 MariaDB [company]> UPDATE employee SET name="Bob" WHERE id=1;
 MariaDB [company]> DELETE FROM employee WHERE id=2;
 ```

+ **创建索引**

 ```lang-sql
 MariaDB [company]> ALTER TABLE employee ADD INDEX name_idx(name(30));
 ```

+ **存储过程**

 ```lang-sql
 MariaDB [company]> DELIMITER //
 MariaDB [company]> CREATE PROCEDURE delete_match()
     -> BEGIN
     -> DELETE FROM employee WHERE id=1;
     -> END//
 MariaDB [company]> DELIMITER ;
 MariaDB [company]> CALL delete_match();
 ```

+ **视图**

 ```lang-sql
 MariaDB [company]> CREATE VIEW manager_view AS
     -> SELECT
     -> e.name, m.department
     -> FROM
     -> employee AS e, manager AS m
     -> WHERE e.id=m.employee_id;
 MariaDB [company]> SELECT * FROM manager_view;
 ```

## 在线修改 DDL

MariaDB 实例组件支持大多数 DDL 的在线修改。在线修改支持原表中（INPLACE）修改表属性，并且允许并发的 DML。可以通过 ALGORITHM 参数控制 ALTER TABLE 语句修改 DDL 时使用的算法。当 ALGORITHM = INPLACE 时，可以在线地修改表属性，而 ALGORITHM = COPY 时，则会把原表内容拷贝到新表，性能会下降。不指定 ALGORITHM 时会自动选择算法。如需在线修改 DDL，一般建议显式地指定 ALGORITHM = INPLACE。例子：

 ```lang-sql
 MariaDB [company]> ALTER TABLE employee ADD INDEX age_idx(age), ALGORITHM = INPLACE;
 ```

+ **建表选项**

   可以在线修改字符集（CHARACTER SET）、重命名（RENAME）。支持修改表备注（COMMENT）中的自定义注释，以及更改或追加 table_options 中的配置项，不支持修改 auto_partition。表备注选项参见[自定义表配置](sql_engine/sequoiasql_mariadb/config.md#自定义表配置)。

+ **主键与索引**

   支持在线添加删除主键和（唯一）索引。

+ **列**

    支持在线添加列、删除列和数据类型扩容的操作。其中，添加 DEFAULT NULL 的列，以及为数据类型扩容（如 VARCHAR(64) 改 VARCHAR(128)、INT 改 BIGINT）是轻量级的操作，可以快速返回。


