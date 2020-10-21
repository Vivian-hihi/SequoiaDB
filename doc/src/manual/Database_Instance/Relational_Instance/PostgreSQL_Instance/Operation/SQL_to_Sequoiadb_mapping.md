本文档将对比介绍 SQL 实例和 JSON 对象的概念、术语及基本操作。

##概念和术语##

| SQL 实例                                 | JSON 对象                                     |
| ---------------------------------------- | ------------------------ |
| database                                 | collection space         |
| table                                    | collection               |
| row                                      | document / BSON document |
| column                                   | field                    |
| index                                    | index                    |
| primary key （指定任何唯一的列作为主键） | primary key （是自动创建到记录的 \_id 字段） |

##Create与Alter##

下表列出了各种表级别的操作：

| SQL 实例语句 | JSON对象语句 |
| -------- | -------------- |
| CREATE TABLE employee ( name char(10), age  integer ); | db.sample.createCL( "employee" ) |
| ALTER TABLE employee ADD COLUMN sex char(5); | 集合不强制执行文档的结构，即在集合上不需要改动结构操作 |
| ALTER TABLE employee DROP COLUMN sex; |  集合不强制执行文档的结构，即在集合上不需要改动结构操作 |
| CREATE INDEX aIndex ON employee (age); | db.sample.employee.createIndex( "aIndex", { age: 1 } )  |
| DROP TABLE employee; | db.sample.dropCL( "employee" ) |

##Insert##

下表列出了在表上的插入操作：

| SQL 实例语句                        | JSON 对象语句                                  |
| ----------------------------------- | ---------------------------------------------- |
| INSERT INTO employee VALUES ('Harry',8); | db.sample.employee.insert( { name: "Harry", age: 8 } ) |

##Select##

下表列出了在表上的读操作：

| SQL 实例语句                    | JSON 对象语句                                                |
|-------------------------------- |----------------------------------------------- |
| SELECT * FROM employee;              | db.sample.employee.find() |
| SELECT name, age FROM employee;      | db.sample.employee.find( {},{ name: null, age: null } ) |
| SELECT * FROM employee WHERE age > 25; | db.sample.employee.find( { age: { $gt: 25 } } )            |
| SELECT age FROM employee WHERE age = 25 AND name = 'Harry'; | db.sample.employee.find( { age: 25, name: "Harry" }, { age: null } ) |
| SELECT COUNT( * ) FROM employee;     | db.sample.employee.count()                             |
| SELECT COUNT( name ) FROM employee;  | db.sample.employee.count( { name: { $exists: 1 } } )     |


##Update##

下表列出了在表上的更新操作：

| SQL 实例语句                                            | JSON 对象语句                                                |
| ------------------------------------------------------- | ------------------------------------------------------------ |
| UPDATE employee SET age = 25 WHERE name = 'Harry';      | db.sample.employee.update( { name: "Harry" },{ $set: { age: 25 } } ) |
| UPDATE employee SET age = age + 2 WHERE name = 'Harry'; | db.sample.employee.update( { name: "Harry" },{ $inc: { age: 2 } } ) |

##Delete##

下表列出了在表上的删除操作：

| SQL 实例语句 | JSON 对象语句 |
| ---------------------------------- | ------------------------------------------- |
| DELETE FROM employee WHERE age = 20;    | db.sample.employee.remove( { age: 20 } ) |
| DELETE FROM employee;                   | db.sample.employee.remove() |
