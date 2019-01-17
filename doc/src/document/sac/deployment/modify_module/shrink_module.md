**服务减容** 可以删除服务中的一个或多个节点。

1. 进入 **部署** 页面。

   ![服务选择](sac/deployment/modify_module/shrink_1.jpg)

2. 点击 **服务操作 - 服务减容**

   ![节点选择](sac/deployment/modify_module/shrink_2.jpg)

3. 在弹窗选择要减容的服务。

   ![节点选择](sac/deployment/modify_module/shrink_3.jpg)

4. 演示的是2个3节点的数据组环境。

   ![节点选择](sac/deployment/modify_module/shrink_4.jpg)

5. 演示删除整个数据分区组 group2，把 group2 的节点都选上，也可以直接点击左边分区组列表的 **group2**。

   ![节点选择](sac/deployment/modify_module/shrink_5.jpg)

6. 等待任务。

   ![节点选择](sac/deployment/modify_module/shrink_6.jpg)

7. 任务完成。

   ![节点选择](sac/deployment/modify_module/shrink_7.jpg)

> **Note:**  
> 1. 服务减容功能仅支持 SequoiaDB 集群模式。  
> 2. 如果删除整个数据组，要确保该组没有数据，否则该数据组将无法全部删除，会留下一个节点。  
> 3. 为了保证服务正常运行，不能删除全部 Coord、Catalog 节点。