SequoiaSQL-OLTP数据库操作页面可以进行创建数据库、删除数据库、创建数据表、删除数据表等操作。

###创建数据库
1. 从SAC左侧导航选择进入SequoiaSQL-OLTP数据库操作页面。  
  ![OLTP数据库](sac/data_operation/SequoiaSQL-OLTP/database_oltp.png)

2. 点击右下角创建数据库，输入需要创建的数据库名后点击确定即可完成创建。
  ![创建数据库](sac/data_operation/SequoiaSQL-OLTP/add_database.png)

###创建数据表

创建数据表可以选择创建普通表和外部表，创建外部表需要当前数据库已经进行了[关联业务](sac/deployment/relationship_module.md)操作

1. 点击右下角创建数据表，选择所属数据库之后填写表名及字段。如果选择创建外部表的话，则需要选择服务名，填写对应映射的集合空间及集合名。

  ![创建数据表](sac/data_operation/SequoiaSQL-OLTP/create_table.png)

2. 点击确定开始创建数据表。

> **Note:**  
> 创建完数据表后，可点击数据表名进入[数据操作](sac/sequoiasqloltp_data/record.md)
> 创建外部表不可定义主键和唯一键。

###删除数据库
1. 点击左下角删除数据库，选择需要删除的数据库，点击确定开始删除。

  ![删除数据库](sac/data_operation/SequoiaSQL-OLTP/drop_database.png)

> **Note:**  
> 无法删除当前打开的数据库，并且通过SAC进行删除的话，至少需要保留有一个数据库。

###删除数据表
1. 从数据表列表中选择需要删除的表，点击行中的“X”图标，确认后点击确定即可进行删除该表。

  ![删除数据表](sac/data_operation/SequoiaSQL-OLTP/drop_table.png)

> **Note:**  
> 系统表不可被删除。
