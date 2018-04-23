创建SequoiaSQL-OLTP业务是指在已经部署了SequoiaSQL-OLTP包的主机中创建一个实例。

> **Note:**  
> 创建SequoiaSQL-OLTP业务需要先进行[部署包](sac/deployment/operation_host/deploy_package.md) 操作。

1. 进入部署首页点击添加业务-创建业务，业务类型选择“SequoiaSQL OLTP”点击确定开始创建业务。

  ![创建OLTP](sac/deployment/add_module/add_oltp.png)

2. 进入业务配置页面，页面中填写SequoiaSQL-OLTP的相关配置信息，配置项留空表示使用该项默认值，填写完成后点击下一步开始安装。

  ![安装OLTP](sac/deployment/add_module/config_oltp.png)

3. 等待安装完成。

  ![安装OLTP](sac/deployment/add_module/install_oltp.png)

4. 创建SequoiaSQL-OLTP业务完成。

> **Note：**
>
> SequoiaSQL-OLTP创建完成后，可以通过首页进行与SequoiaDB业务的[关联](sac/deployment/relationship_module.md)及进行[数据操作](sac/sequoiasqloltp_data/database.md)。