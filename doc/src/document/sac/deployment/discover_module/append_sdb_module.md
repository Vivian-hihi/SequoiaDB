**发现服务** 可以将 SequoiaDB 服务添加到 SAC 中。

1. 进入 **部署** 页面。

   ![发现服务](sac/deployment/discover_module/append_sdb_1.jpg)

2. 点击 **添加服务 - 发现服务** 按钮。

   ![发现服务](sac/deployment/discover_module/append_sdb_2.jpg)

3. **服务类型** 选择 **SequoiaDB**，点击 **确定** 按钮。

   ![发现服务](sac/deployment/discover_module/append_sdb_3.jpg)

4. **地址** 支持 IP 和 主机名，填写集群模式的 coord 节点地址，或单机模式的节点地址；服务名填写节点的端口号。  
   如果服务创建了鉴权，需要填写 **数据库用户名** 和 **数据库密码**。点击 **确定** 按钮。

   ![发现SequoiaDB](sac/deployment/discover_module/append_sdb_4.jpg)

   > Note:  
   > 当服务的主机不都在 SAC 管理，会有提示弹窗；点击 **是**，进入 **安装主机**，安装完成后，再按步骤1开始操作 。

   ![发现服务](sac/deployment/discover_module/append_sdb_6.jpg)

5. 完成，在这个页面可以查看该服务的信息。

   ![发现SequoiaDB](sac/deployment/discover_module/append_sdb_5.jpg)
