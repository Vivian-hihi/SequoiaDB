SequoiaSQL-MySQL数据库操作页面可以进行创建数据库、删除数据库、创建数据表、删除数据表等操作。

###创建数据库
1. 从SAC左侧导航选择进入SequoiaSQL-MySQL数据库操作页面。  
  ![SequoiaSQL-MySQL数据库](sac/data_operation/MySQL/database.png)

2. 点击右下角创建数据库，输入需要创建的数据库名后点击确定即可完成创建。
  ![创建数据库](sac/data_operation/MySQL/create_database.png)

###创建数据表

创建数据表可以选择存储引擎，在SAC中创建默认为“SequoiaDB”引擎，确保数据表创建成功请提前查看是否已经[关联服务](sac/deployment/relation/relationship_mysql.md)。

1. 点击右下角创建数据表，选择所属数据库之后填写表名及字段信息，使用“SequoiaDB”存储引擎时可填写集合类型和[集合参数](data_model/collection.md)。

  ![创建数据表](sac/data_operation/MySQL/create_table_normal.png)

2. 点击确定开始创建数据表。

> **Note:**  
> 创建完数据表后，可点击数据表名进入[数据操作](sac/mysql_data/record.md)。  
> 成功创建数据表时会在对应的SequoiaDB服务创建集合空间和集合。

###删除数据库
1. 点击左下角删除数据库，选择需要删除的数据库，点击确定开始删除。

  ![删除数据库](sac/data_operation/MySQL/drop_database.png)


###删除数据表
1. 从数据表列表中选择需要删除的表，点击行中的“X”图标，确认后点击确定即可进行删除该表。

  ![删除数据表](sac/data_operation/MySQL/drop_table.png)

> **Note:**  
> 系统表不可被删除。
