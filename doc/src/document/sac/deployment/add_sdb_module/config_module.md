
添加服务流程共3步骤：

| 步骤 | 说明 |
| ---- | ------------- |
| [配置服务](sac/deployment/add_sdb_module/config_module.md)  | 配置服务部署模式、主机、节点分布。 |
| [修改服务](sac/deployment/add_sdb_module/mod_module.md)     | 修改服务的配置参数。 |
| [安装服务](sac/deployment/add_sdb_module/install_module.md) | 安装服务的服务。 |

1. 在部署首页点击 **添加服务**，选择 **创建服务**，服务类型选择SequoiaDB数据库，点击确定开始添加服务。
   ![配置服务](sac/deployment/add_sdb_module/add_module.jpg)

2. 配置服务，根据实际需求配置。

   ![配置服务](sac/deployment/add_sdb_module/config_module_1.jpg)

3. 点击 **选择安装服务的主机**，可以指定主机安装服务，默认全选。

   ![配置服务](sac/deployment/add_sdb_module/config_module_2.jpg)

4. 演示将会使用3台主机安装SequoiaDB集群，3副本，3分区组

   ![配置服务](sac/deployment/add_sdb_module/config_module_3.jpg)

5. 点击底部的 **下一步** 按钮，进入修改服务页面。

6. 开始修改服务，[点击查看](sac/deployment/add_sdb_module/mod_module.md)。