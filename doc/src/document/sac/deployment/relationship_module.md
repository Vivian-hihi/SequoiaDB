关联服务是指使用SequoiaDB服务对接其他类型的服务，目前支持对接SequoiaSQL-PostgreSQL服务、SequoiaSQL-MySQL服务。  
当前文档以SequoiaSQL-PostgreSQL服务关联SequoiaDB服务为例。

> **Note：**  
> 使用关联服务需要SAC有[SequoiaDB服务](sac/deployment/add_sdb_module/config_module.md)以及[SequoiaSQL-PostgreSQL服务](sac/deployment/install_postgresql.md)。


###创建关联

1. 进入部署首页点击关联服务-创建关联，选择关联类型"SequoiaSQL-PostgreSQL - SequoiaDB"和需要关联、被关联的服务，填写相关信息。

  ![关联服务](sac/deployment/create_relation.png)

2. 填写完成后点击确定，关联完成。

  ![关联完成](sac/deployment/create_relation2.png)

> **Note：**
> 关联服务参数说明：
>
> 关联名：创建关联完成后的名字，全局唯一。
>
> 数据库：选择关联的数据库名。  
> 
> preferedinstance：指定SequoiaSQL-PostgreSQL 访问SequoiaDB 数据节点时，优先连接哪种角色的数据节点，默认为’a’，可输入参数’m’/’s’/’a’/1-7，分别表示master/slave/anyone/node1-node7。
>
> transaction：设置SequoiaDB是否开启事务，默认为off。开启为on。  
>
> 选择被关联节点：选择关联SequoiaDB的coord节点，默认为全部关联。
>
> 进行“SequoiaSQL-MySQL - SequoiaDB”关联类型时，将会重启MySQL服务。

###解除关联

1. 进入部署首页点击关联服务-解除关联，选择需要解除的关联名。

  ![解除关联](sac/deployment/drop_relation.png)

2. 点击确定，解除关联完成。

> **Note：**
>
> 解除关联前需要删除关联服务建立的数据表，否则将解除失败。