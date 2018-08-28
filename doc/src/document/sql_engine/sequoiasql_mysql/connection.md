以下操作均在MySQL shell 环境下执行。
##连接MySQL与SequoiaDB##

1. 登录MySQL shell

 ```lang-javascript
 $ export MYSQL_HOME=/opt/sequoiasql/mysql
 $ ${MYSQL_HOME}/bin/mysql --defaults-file=${MYSQL_HOME}/my.cnf -S ${MYSQL_HOME}/data3306/mysqld.sock -u root -p
 ```

2. 配置SequoiaDB连接地址

   默认的SequoiaDB连接地址为“localhost:11810”，如需修改可通过bin/sdb_mysql_ctl工具或配置文件的方式进行修改，具体修改方法详见后面的[配置说明](sql_engine/sequoiasql_mysql/connection.md#配置说明)

3. 创建数据库实例

 ```lang-javascript
 mysql> create database cs;
 mysql> use cs;
 ```

4. 创建表

 ```lang-javascript
 mysql> create table cl(a int, b int, c text, primary key(a, b) ) engine = SequoiaDB ;
 mysql> create table cl1(a int, b int, unique index idx_a(a) ) engine = SequoiaDB ;
 ```

5. 基本数据操作

 ```lang-javascript
 mysql> insert into cl values(1, 101, "SequoiaDB test");
 mysql> insert into cl values(2, 102, "SequoiaDB test");
 mysql> insert into cl1 values(1, 99);
 mysql> select * from cl order by b asc limit 1;
 mysql> select * from cl, cl1 where cl.a=cl1.a;
 mysql> update cl set c="My test" where a=1;
 mysql> delete from cl where b=102;
 ```

6. 创建索引

   使用非索引字段（"c"）执行查询时，访问计划信息如下：

 ```lang-javascript
 mysql> explain select * from cl where c="test";
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+---------------------------------------------------------------+
 | id | select_type | table | partitions | type | possible_keys | key  | key_len | ref  | rows | filtered | Extra                                                         |
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+---------------------------------------------------------------+
 |  1 | SIMPLE      | cl    | NULL       | ALL  | NULL          | NULL | NULL    | NULL |    2 |    50.00 | Using where with pushed condition (`test1`.`cl`.`c` = 'test') |
 +----+-------------+-------+------------+------+---------------+------+---------+------+------+----------+---------------------------------------------------------------+
 1 row in set, 1 warning (0.00 sec)

 ```

   对“c”字段创建索引并执行相同查询时将会使用该索引，具体访问计划信息如下：
 
 ```lang-javascript
 mysql> alter table cl add index idx_c(c(20));
 Query OK, 0 rows affected (0.03 sec)
 Records: 0  Duplicates: 0  Warnings: 0

 mysql> explain select * from cl where c="test";
 +----+-------------+-------+------------+------+---------------+-------+---------+-------+------+----------+---------------------------------------------------------------+
 | id | select_type | table | partitions | type | possible_keys | key   | key_len | ref   | rows | filtered | Extra                                                         |
 +----+-------------+-------+------------+------+---------------+-------+---------+-------+------+----------+---------------------------------------------------------------+
 |  1 | SIMPLE      | cl    | NULL       | ref  | idx_c         | idx_c | 23      | const |    1 |   100.00 | Using where with pushed condition (`test1`.`cl`.`c` = 'test') |
 +----+-------------+-------+------------+------+---------------+-------+---------+-------+------+----------+---------------------------------------------------------------+
 1 row in set, 1 warning (0.01 sec)

 ```

7. 存储过程

 ```lang-javascript
 mysql> delimiter //
 mysql> create procedure delete_match()
     -> begin
     -> delete from cl where a=1;
     -> end//
 mysql> delimiter ;
 mysql> call delete_match();
 ```

8. 视图

 ```lang-javascript
 mysql> create view
     -> v1
     -> as select
     -> cl.a,cl.c,cl1.b
     -> from
     -> cl,cl1
     -> where cl.a=cl1.a;
 mysql> select * from v1;
 ```

##配置说明##

1. 配置SequoiaDB连接地址  
   默认的SequoiaDB连接地址为“localhost:11810”，可以通过以下两种方式修改该地址：  
   (1)通过bin/sdb_mysql_ctl指定端口号修改

 ```lang-javascript
    # ./bin/sdb_mysql_ctl config 3306 sequoiadb_conn_addr 192.168.20.37:11810,192.168.20.38:11810
 ```

      注意：改操作目前会修改所有实例sequoiadb_conn_addr的值

   (2)修改安装路径下的配置文件my.cnf，在[mysqldN]下添加如下配置（N表示正整数）：  

 ```lang-javascript
 sequoiadb_conn_addr=192.168.20.37:11810,192.168.20.38:11810
 ```

	  注意：修改配置文件后需要重新启动MySQL服务

 配置完成后，可以通过以下命令查看配置结果

 ```lang-javascript
 mysql> show variables like 'sequoiadb%';
 ```
 
2. 配置分区表  
   默认情况下，在mysql上创建表将同步在SequoiaDB上创建对应的分区表（hash分区，包含所有分区组）。  
   分区键优先使用主键字段，如果建表时没有创建主键则使用唯一键，如果没有创建唯一键则使用第一个字段。  
   用户可以通过将配置参数“sequoiadb_use_partition”设置为“OFF”禁止创建默认分区表，该配置参数可以在shell命令行和配置文件中修改：  
   (1)修改安装路径下的配置文件my.cnf，在[mysqldN]下添加如下配置（N表示正整数）：  

 ```lang-javascript
 sequoiadb_use_partition=OFF
 ```

	  注意：修改配置文件后需要重新启动MySQL服务

   (2)通过MySQL shell修改  

 ```lang-javascript
 mysql> SET GLOBAL sequoiadb_use_partition=OFF;
 ```

	  注意：通过shell方式进行的配置为临时有效，当重启MySQL服务后配置将失效。如果需要配置永久生效则必须通过修改配置文件。  
 
3. 建表参数  
   在mysql上创建表时，也可以通过comment参数传入自定义分区表配置，comment参数为json格式，具体配置参数如下表：
 
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | --- | ------ | ------ |
| table_options | json | 创建集合的相关参数。详见SequoiaDB集合相关参数，如：分区键(ShardingKey)、分区类型(ShardingType)等均在options中添加。| 否	|

 示例：  
 在SequoiaDB上创建分区键为“{a:1,b:-1}”，分区类型为范围分区的集合cl  

 ```lang-javascript
 mysql> create table cl(a int, b int, c text) engine = SequoiaDB comment="{table_options:{ShardingKey:{a:1,b:-1},ShardingType:\"range\"}}";
 ```


##数据类型对应关系##

	| MySQL           | SequoiaDB         | 备注                                                              |
	| --------------- | ----------------- | ----------------------------------------------------------------- |
	| TINYINT         | INT               |                                                                   |
	| SMALLINT        | INT               |                                                                   |
	| MEDIUMINT       | INT               |                                                                   |
	| INT             | INT               |                                                                   |
	| BIGINT          | LONG LONG         |                                                                   |
	| FLOAT           | DOUBLE            |                                                                   |
	| DOUBLE          | DOUBLE            |                                                                   |
	| DECIMAL         | DECIMAL           |                                                                   |
	| DATE            | DATE              |                                                                   |
	| DATETIME        | STRING            |取值范围：1000-01-01 00:00:00.000000 至 9999-12-31 23:59:59.999999 |
	| TIMESTAMP       | TIMESTAMP         |取值范围：1902-01-01 00:00:00.000000 至 2037-12-31 23:59:59.999999 |
	| CHAR            | STRING            |                                                                   |
	| VARCHAR         | STRING            |                                                                   |
	| TEXT            | STRING            |最大长度16MB                                                       |
	| BINARY          | BINARY            |                                                                   |
	| VARBINARY       | BINARY            |                                                                   |
	| BLOB            | BINARY            |最大长度16MB                                                       |
	| NULL            | UNDEFINE          |                                                                   |
