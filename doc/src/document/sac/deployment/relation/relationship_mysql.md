**关联服务** 是指两个不同类型的服务对接，例如：  
SequoiaSQL-PostgreSQL 服务关联 SequoiaDB 服务；  
SequoiaSQL-MySQL 服务关联 SequoiaDB 服务。

这里演示 SequoiaSQL-MySQL 服务关联 SequoiaDB 服务。

> **Note：**  
> 使用关联服务需要 SAC 有 **SequoiaDB 服务** 和 **SequoiaSQL-MySQL 服务**。  
> 在 SAC 创建 SequoiaDB 服务，[点击查看](sac/deployment/add_sdb_module/config_module.md)。  
> 在 SAC 创建 SequoiaSQL-MySQL 服务，[点击查看](sac/deployment/install_mysql.md)。


###创建关联

1. 进入 **部署** 页面。

   ![关联服务](sac/deployment/relation/mysql_sdb_1.png)

2. 点击 **关联服务 - 创建关联**。

   ![关联服务](sac/deployment/relation/mysql_sdb_2.png)

3. 在弹窗填写关联参数。

   ![关联服务](sac/deployment/relation/mysql_sdb_3.png)

   ![关联服务](sac/deployment/relation/mysql_sdb_4.png)

4. 创建关联成功。

   ![关联服务](sac/deployment/relation/mysql_sdb_5.png)

5. 在 **部署** 页面可以看到服务的关联信息。

   ![关联服务](sac/deployment/relation/mysql_sdb_6.png)

###解除关联

1. 进入 **部署** 页面。

   ![关联服务](sac/deployment/relation/d_mysql_sdb_1.png)

2. 点击 **关联服务 - 解除关联**。

   ![关联服务](sac/deployment/relation/d_mysql_sdb_2.png)

3. 选择要解除的 **关联名**，SequoiaSQL-MySQL 服务解除关联需要重启。

   ![关联服务](sac/deployment/relation/d_mysql_sdb_3.png)

4. 解除关联成功。

   ![关联服务](sac/deployment/relation/d_mysql_sdb_4.png)

5. 在 **部署** 页面可以看到两个服务的关联信息已经没有了。

   ![关联服务](sac/deployment/relation/d_mysql_sdb_5.png)
