通过OM部署好数据库之后，就可以进行数据库操作了。

**Note: **

新界面的设计采用更合理的方式展示信息。

1. 所有**<span style="color:#00BFDD">天蓝色</span>**的字体、按钮都是可点击的。

![](visualization_data_operation_cn_4.jpg)

![](visualization_data_operation_cn_5.jpg)

2. 所有出现 ![](visualization_data_operation_cn_7.jpg) 图标的，当鼠标停在图标上，都会出现相关提示。

![](visualization_data_operation_cn_6.jpg)

3. 当出现 **...** 省略符时，把鼠标停留在上面，会出现完整的信息。

![](visualization_data_operation_cn_8.jpg)

![](visualization_data_operation_cn_9.jpg)

##安装 OM 服务##

如何没有安装OM服务，并且部署SequoiaDB数据库，请参考 [可视化安装](SdbDoc_Cn/installation/deployment/visualization_installation.html) 。

###进入数据库业务操作###

-   在首页点击集群对应的&lt;**业务**&gt;，进入业务列表页面。

    如图，点击myCluster集群的&lt;**业务**&gt;。

    ![](visualization_data_operation_cn_1.jpg)

-   点击要进行操作的&lt;**业务名**&gt;，进入数据操作页面。

    如图，点击myModule业务。

    ![](visualization_data_operation_cn_2.jpg)
    
    由于是新创建的SequoiaDB业务，还没有创建任何集合空间和集合。
    
    ![](visualization_data_operation_cn_3.jpg)

###数据库操作###

**创建集合空间**

-   创建集合空间

    1.1 点击&lt;**数据库操作**&gt;。

    ![](visualization_data_operation_cs_cn_1.jpg)

    1.2 点击&lt;**创建集合空间**&gt;。

    ![](visualization_data_operation_cs_cn_2.jpg)

    1.3 填写新建集合空间的名字，其他参数不是必填项，可以根据实际情况设置。

    ![](visualization_data_operation_cs_cn_3.jpg)

    1.4 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_cs_cn_4.jpg)

    1.5 创建成功;。

    ![](visualization_data_operation_cs_cn_5.jpg)

-   创建集合（普通类型）

    1.1 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)

    1.2 选择创建集合所属的集合空间。因为只有刚刚创建的**集合空间foo**，所以自动选择了**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    1.3 现在要创建一个**普通类型**的集合，因此&lt;**集合类型**&gt;不需要修改，填写新建集合的名字。

    ![](visualization_data_operation_cl_cn_3.jpg)

    1.4 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_cl_cn_4.jpg)

    1.5 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    1.6 创建成功。

    ![](visualization_data_operation_cl_cn_6.jpg)


























