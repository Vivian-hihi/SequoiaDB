服务扩容有两种方式：  
  一、添加分区组：增加分区组。  
  二、添加副本数：在原有的组上增加节点，如果原有的组节点数量比设置要多，则该组不增加节点。

1. 从部署首页选择需要扩容的服务。
![服务选择](sac/deployment/extend_module/extend_1.jpg)

2. 选择扩容模式。  
   * 选择添加分区组，设置添加的分区组数和副本数，点击下一步。
   ![水平扩容](sac/deployment/extend_module/extend_2.jpg)

   * 选择添加副本数，设置每个分区组需要添加的副本数，点击下一步。
   ![垂直扩容](sac/deployment/extend_module/extend_rep_1.jpg)

3. 进入修改服务页面，可以修改扩容的节点配置。
![修改服务](sac/deployment/extend_module/extend_3.jpg)

4. 一般自动生成的配置不需要修改就能直接下一步安装。

5. 这一步不是必须做的操作，因为演示的主机都只有1个磁盘，mount在根目录（/ 路径），因此需要修改数据路径。 点击 **全选**，然后点击 **批量修改节点**，把 数据路径 改成 /opt/sequoiadb/database/[role]/[svcname]，点击 **确定**。
![修改服务](sac/deployment/extend_module/extend_edit_2.jpg)

6. 修改后的配置，点击下一步，开始安装。
![修改服务](sac/deployment/extend_module/extend_edit_3.jpg)

7. 等待安装完成。
![安装服务](sac/deployment/extend_module/extend_install_1.jpg)

8. 安装完成。
![安装服务](sac/deployment/extend_module/extend_install_2.jpg)

> **Note:**  
> 1. 如果要开启事务，必须所有节点都开启。  
> 2. 批量修改节点配置 **数据路径** 和 **服务名** 支持特殊规则来简化修改。规则可以点击页面 **提示** 的 **帮助**。  
> 3. 批量修改节点配置时，如果值为空，那么代表该参数的值不修改。