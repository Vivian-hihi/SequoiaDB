创建SequoiaSQL-MySQL服务是指在已经部署了SequoiaSQL-MySQL包的主机中创建一个实例。

> **Note:**  
> 创建SequoiaSQL-MySQL服务需要先进行[部署包](sac/deployment/operation_host/deploy_package.md) 操作。

1. 进入部署首页点击添加服务-创建服务，服务类型选择“SequoiaSQL-MySQL”点击确定开始创建服务。

  ![创建SequoiaSQL-MySQL](sac/deployment/add_mysql.png)

2. 进入服务配置页面，页面中填写SequoiaSQL-MySQL的相关配置信息，填写完成后点击下一步开始安装。

  ![安装SequoiaSQL-MySQL](sac/deployment/config_mysql.png)

  > **Note：**
  >
  > 配置项“访问授权”选择“root账号授权任意主机访问”时，通过root用户就可以访问MySQL服务。  
  > 选择“创建SAC访问的账号”时，需要填写账号密码，访问MySQL服务时需要该账户密码。  
  > 选择“不设置鉴权”时，需要手动前往MySQL中的user表新建用户之后，再在SAC设置鉴权才能正常访问。


3. 等待安装完成。

  ![安装SequoiaSQL-MySQL](sac/deployment/install_mysql.png)

4. 创建SequoiaSQL-MySQL服务完成。

> **Note：**
>
> SequoiaSQL-MySQL创建完成后，可以通过首页进行与SequoiaDB服务的[关联](sac/deployment/relationship_module.md)及进行[数据操作](sac/mysql_data/database.md)。