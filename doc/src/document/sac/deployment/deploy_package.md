部署包是将指定的包部署到指定的主机中。目前支持部署SequoiaSQL-PostgreSQL包和SequoiaSQL-MySQL包。  
当前文档以SequoiaSQL-PostgreSQL包为例。

> **Note:**  
> 部署包之前需要将run包放在SAC所在主机的安装路径packet路径下，默认：/opt/sequoiadb/packet。

1. 在 **部署** 页面勾选需要部署包的主机，点击 **主机操作 - 部署包** 进入配置页面。

   ![部署包](sac/deployment/deploy_package_1.jpg)

2. 填写好参数之后，点击下一步进行安装。

   > **Note:**  
   > 安装方式可以开启 **强制安装**，默认不开启。  
   > 当选择 false 时，会忽略已经安装的主机。  
   > 当选择 true 时，会在选择的主机上重新安装，并且强制重启的对应服务。

   ![部署包](sac/deployment/deploy_package_2.jpg)

3. 安装完成。

   ![部署包](sac/deployment/deploy_package_3.jpg)

4. 点击 **完成**，回到 **部署** 页面，查看主机，可以看见刚刚安装的两台主机已经支持SequoiaSQL-PostgreSQL。

   ![部署包](sac/deployment/deploy_package_4.jpg)