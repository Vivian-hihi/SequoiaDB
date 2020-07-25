用户安装好 MariaDB 实例组件后，可直接通过 MariaDB Shell 使用标准的 SQL 语言访问 SequoiaDB。

##连接MariaDB实例与数据库分布式存储引擎

**配置 SequoiaDB 连接地址**

SequoiaDB 巨杉数据库默认的连接地址为 `localhost:11810`，用户可通过命令行或修改配置文件两种方式修改连接地址。以下步骤中的路径均为默认安装路径，用户可根据实际情况修改。

   
   * 通过 sdb_maria_ctl 指定实例名修改 SequoiaDB 连接地址

   ```lang-bash
   $ /opt/sequoiasql/mariadb/bin/sdb_maria_ctl chconf myinst --sdb-conn-addr=sdbserver1:11810,sdbserver2:11810
   ```

   * 通过实例配置文件修改 SequoiaDB 连接地址

      ```lang-bash
      $ vi /opt/sequoiasql/mariadb/database/6101/auto.cnf
      ```
      
      修改 sequoiadb_conn_addr 的内容如下：

      ```config
      sequoiadb_conn_addr=sdbserver1:11810,sdbserver2:11810
      ```

       >   **Note:**
       >    
       >   目前 sdb_maria_ctl 工具仅支持一些简单配置项的修改，建议用户采用配置文件的方式修改配置，修改方式同上。具体配置参考[引擎配置](sql_engine/sequoiasql_mariadb/config.md#引擎配置)章节。

**登录 MariaDB Shell** 
 
MariaDB 支持基于 UNIX 域套接字文件和 TCP/IP 两种连接方式。通过 UNIX 域套接字文件连接时需指定对应的套接字文件，该连接方式属于进程间通信，不需要使用网络协议且传输效率比 TCP/IP 连接方式更高，但仅限于本地连接。通过 TCP/IP 连接属于网络通信，支持本地连接（环回接口）和远程连接，同时可以灵活地配置和授权客户端 IP 的访问权限。
 
   * 通过 UNIX 域套接字文件连接

      ```lang-bash
      $ cd /opt/sequoiasql/mariadb
      $ bin/mysql -S database/6101/mysqld.sock -u sdbadmin
      ```
      
      > **Note:** 
      > 
      > SequoiaSQL-MariaDB 实例默认无密码，所以无需输入 -p 选项。

   * 通过 TCP/IP 连接  
   
     * 本地连接 

       需设置密码才能登陆，在套接字连接方式下为 sdbadmin 用户设置密码 123456

       ```lang-sql
       MariaDB [(none)]> ALTER USER sdbadmin@localhost IDENTIFIED BY '123456';    
       ``` 
       
       用 TCP/IP 方式建立连接

       ```lang-bash
       $ bin/mysql -h 127.0.0.1 -P 6101 -u sdbadmin -p
       ```
       
       > **Note:** 
       >  
       > 用户设置密码后，登录 MariaDB Shell 需要指定 -p 参数输入密码。
        
     * 远程连接  
         
         MariaDB 默认未授予远程连接的权限，所以需要在服务端对客户端 IP 进行访问授权。
	 
      1、sdbadmin 用户对所有的 IP 都授权访问权限，且设置授权密码 123456

         ```lang-sql
         MariaDB [(none)]> GRANT ALL PRIVILEGES ON *.* TO sdbadmin@'%' IDENTIFIED BY '123456' WITH GRANT OPTION;
         MariaDB [(none)]> FLUSH PRIVILEGES;
         ```

      2、假设 mariadb 服务器地址为 `sdbserver1:6101`，在客户端可以使用如下方式进行远程连接：

         ```lang-bash
         $ /opt/sequoiasql/mariadb/bin/mysql -h sdbserver1 -P 6101 -u sdbadmin
         ```

## 基本操作示例

以下列举一些简单的操作示例，具体可参考 MariaDB 官网：https://mariadb.com/kb/en/documentation/

- **创建数据库实例**

 ```lang-sql
 MariaDB [(none)]> CREATE DATABASE company;
 MariaDB [company]> USE company;
 ```

- **创建表**

 ```lang-sql
 MariaDB [company]> CREATE TABLE employee(id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(128), age INT);
 MariaDB [company]> CREATE TABLE manager(employee_id INT, department TEXT, INDEX id_idx(employee_id));
 ```

- **基本数据操作**

 ```lang-sql
 MariaDB [company]> INSERT INTO employee(name, age) VALUES("Jacky", 36);
 MariaDB [company]> INSERT INTO employee(name, age) VALUES("Alice", 18);
 MariaDB [company]> INSERT INTO manager VALUES(1, "Wireless Business");
 MariaDB [company]> SELECT * FROM employee ORDER BY id ASC LIMIT 1;
 MariaDB [company]> SELECT * FROM employee, manager WHERE employee.id=manager.employee_id;
 MariaDB [company]> UPDATE employee SET name="Bob" WHERE id=1;
 MariaDB [company]> DELETE FROM employee WHERE id=2;
 ```

- **创建索引**

 ```lang-sql
 MariaDB [company]> ALTER TABLE employee ADD INDEX name_idx(name(30));
 ```

- **删除表和数据库实例**

 ```lang-sql
 MariaDB [company]> DROP TABLE employee, manager;
 MariaDB [company]> DROP DATABASE company;
 ```