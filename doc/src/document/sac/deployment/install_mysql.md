创建 SequoiaSQL-MySQL 服务是指在已经部署了 SequoiaSQL-MySQL 包的主机中创建一个实例。

> **Note:**  
> 创建SequoiaSQL-MySQL服务需要先进行部署包操作，[点击查看](sac/deployment/deploy_package.md)。

1. 进入 **部署** 页面。

  ![创建SequoiaSQL-MySQL](sac/deployment/add_mysql_1.jpg)

2. 点击 **添加服务-创建服务**，服务类型选择 **SequoiaSQL-MySQL**， 点击 **确定** 按钮进入 **配置服务** 页面。
  ![安装SequoiaSQL-MySQL](sac/deployment/add_mysql_2.jpg)

3. 在 **配置服务** 页面，可以修改 SequoiaSQL-MySQL 的配置，点击 **下一步** 按钮开始安装。

  > **Note：**  
  > 配置项 **访问授权** 支持三种授权方式，让 SAC 可以访问 SequoiaSQL-MySQL。  
  > 选择 **root账号授权任意主机访问**，SAC 通过 root 用户访问。  
  > 选择 **创建SAC访问的账号**，需要填写用户密码，SAC 通过该账号访问。  
  > 选择 **不设置鉴权**，需要在 SequoiaSQL-MySQL 中新建用户，把新建的用户密码设置在 SAC 该服务的鉴权中。  
  > 设置鉴权的流程，[点击查看](sac/deployment/modify_module/sdb_auth.md)。

  ![安装SequoiaSQL-MySQL](sac/deployment/add_mysql_3.jpg)

4. 安装服务完成。

  ![安装SequoiaSQL-MySQL](sac/deployment/add_mysql_4.jpg)

> **Note：**  
> SequoiaSQL-MySQL 服务创建后，可以关联 SequoiaDB 服务，[点击查看](sac/deployment/relationship_module.md)  
> 也可以对 SequoiaSQL-MySQL 进行数据操作，[点击查看](sac/mysql_data/database.md)。