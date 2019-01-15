
添加服务流程共3步骤：

| 步骤 | 说明 |
| ---- | ------------- |
| [配置服务](sac/deployment/add_sdb_module/config_module.md)  | 配置服务部署模式、主机、节点分布。 |
| [修改服务](sac/deployment/add_sdb_module/mod_module.md)     | 修改服务的配置参数。 |
| [安装服务](sac/deployment/add_sdb_module/install_module.md) | 安装服务的服务。 |

1. 进入修改服务页面，这里可以修改服务的配置，这里演示的是集群模式的配置。

   ![修改服务](sac/deployment/add_sdb_module/modify_module_1.jpg)

2. 一般自动生成的配置不需要修改就能直接下一步安装。

   ![修改服务](sac/deployment/add_sdb_module/modify_module_3.jpg)

3. 点击底部的 **下一步** 按钮，进入安装服务页面。

4. 开始安装服务，[点击查看](sac/deployment/add_sdb_module/install_module.md)。

> **Note:**  
> 1. 如果要开启事务，必须所有节点都开启。  
> 2. 批量修改节点配置 **数据路径** 和 **服务名** 支持特殊规则来简化修改。规则可以点击页面 **提示** 的 **帮助**。  
> 3. 批量修改节点配置时，如果值为空，那么代表该参数的值不修改。

####导入导出配置####

**v2.8版本**新增SequoiaDB集群模式下导入导出配置，支持JSON和XML格式。

点击 **编辑配置** 按钮，出现编辑配置的窗口。

![修改服务](sac/deployment/add_sdb_module/modify_module_4.jpg)