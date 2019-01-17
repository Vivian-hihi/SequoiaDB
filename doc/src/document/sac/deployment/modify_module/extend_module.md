> **服务扩容** 有两种方式：  
> 1. 添加分区组：增加分区组。  
> 2. 添加副本数：在原有的组上增加节点，如果原有的组节点数量大于或等于设置的，该组不增加节点。


1. 进入 **部署** 页面。

   ![服务选择](sac/deployment/modify_module/extend_1.jpg)

2. 点击 **服务操作 - 服务扩容**。

   ![服务选择](sac/deployment/modify_module/extend_2.jpg)

3. 选择扩容的服务，点击 **确定** 按钮。

   ![服务选择](sac/deployment/modify_module/extend_3.jpg)

4. 选择 **扩容模式**，请根据实际需求选择，演示将添加一个分区组。

   * **添加分区组**，设置 **分区组数** 和 **副本数**，点击 **下一步**按钮。

     ![水平扩容](sac/deployment/modify_module/extend_4.jpg)

   * **添加副本数**，设置每个分区组需要添加的副本数，点击 **下一步**按钮。

     ![垂直扩容](sac/deployment/modify_module/extend_5.jpg)

5. 在 **修改服务** 页面，可以修改新增节点的配置，然后点击 **下一步** 按钮。

   ![修改服务](sac/deployment/modify_module/extend_6.jpg)

6. 等待任务完成。

   ![修改服务](sac/deployment/modify_module/extend_7.jpg)

7. 任务完成，点击 **完成** 按钮，回到 **部署** 页面。

   ![修改服务](sac/deployment/modify_module/extend_8.jpg)

> **Note:**  
> 1. 如果要开启事务，同一个分区组下的节点都必须开启。  
> 2. 批量修改节点配置 **数据路径** 和 **服务名** 支持特殊规则来简化修改。规则可以点击页面 **提示** 的 **帮助**。  
> 3. 批量修改节点配置时，如果值为空，那么代表该参数的值不修改。