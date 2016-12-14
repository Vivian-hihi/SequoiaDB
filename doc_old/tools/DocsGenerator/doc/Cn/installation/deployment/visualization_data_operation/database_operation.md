###页面简介###

![](visualization_data_operation_cn_10.jpg)

###创建集合空间###

-   

    **准备工作： 创建SequoiaDB业务。**

    1. 点击&lt;**数据库操作**&gt;。

    ![](visualization_data_operation_cs_cn_1.jpg)

    2. 点击&lt;**创建集合空间**&gt;。

    ![](visualization_data_operation_cs_cn_2.jpg)

    3. 填写新建集合空间的名字，其他参数不是必填项，可以根据实际情况设置。

    ![](visualization_data_operation_cs_cn_3.jpg)

    4. 其他参数说明，请参考 [数据模型 - 集合空间](SdbDoc_Cn/data_model/collectionspace.html) 。

    5. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_cs_cn_4.jpg)

    6. 创建成功;。

    ![](visualization_data_operation_cs_cn_5.jpg)

###创建集合（普通类型）###

-   

    **准备工作： 创建SequoiaDB业务、创建集合空间。**

    1. 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)
 
    2. 选择创建集合所属的集合空间。因为只有刚刚创建的**集合空间foo**，所以自动选择了**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    3. 现在要创建一个 **普通类型** 的集合，因此&lt;**集合类型**&gt;不需要修改，填写新建集合的名字。

    **Note: ** 集合名可以自由取。

    ![](visualization_data_operation_cl_cn_3.jpg)

    4. 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

    5. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_cl_cn_4.jpg)

    6. 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    7. 创建成功。

    ![](visualization_data_operation_cl_cn_6.jpg)

###创建集合（水平范围分区）###

-   

    **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**

    1. 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)

    2. 选择创建集合所属的集合空间。这里仍然默认选**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    3. **集合类型** 选 **水平范围分区**。

    ![](visualization_data_operation_cl_cn_7.jpg)

    4. 填写新建集合的名字。

    **Note: ** 集合名可以自由取。

    ![](visualization_data_operation_cl_cn_8.jpg)

    5. 填写分区键，这里演示取 **id** 和 **time**，详细说明参考 [基本操作 - 数据分区 - 分区键](SdbDoc_Cn/basic_operation/sharding/shardingkey.html) 。

    **Note: ** 分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

    ![](visualization_data_operation_cl_cn_9.jpg)

    ![](visualization_data_operation_cl_cn_10.jpg)

    ![](visualization_data_operation_cl_cn_11.jpg)
    
    6. 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

    7. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_rangecl_cn_1.jpg)

    8. 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    9. 创建成功。

    ![](visualization_data_operation_cl_cn_12.jpg)

###创建集合（水平散列分区）###

-   

    **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**

    1. 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)

    2. 选择创建集合所属的集合空间。这里仍然默认选**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    3. **集合类型** 选 **水平散列分区**。

    ![](visualization_data_operation_cl_cn_18.jpg)

    4. 填写新建集合的名字。

    **Note: ** 集合名可以自由取。

    ![](visualization_data_operation_cl_cn_13.jpg)

    5. 填写分区键，这里演示取 **id** 和 **time**，详细说明参考 [基本操作 - 数据分区 - 分区键](SdbDoc_Cn/basic_operation/sharding/shardingkey.html) 。

    **Note: ** 分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

    ![](visualization_data_operation_cl_cn_14.jpg)

    ![](visualization_data_operation_cl_cn_15.jpg)

    ![](visualization_data_operation_cl_cn_16.jpg)

    6. 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

    7. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_hashcl_cn_1.jpg)

    8. 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    9. 创建成功。

    ![](visualization_data_operation_cl_cn_17.jpg)

###创建集合（垂直分区）###

-   

    **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**

    1. 点击&lt;**创建集合**&gt;。

    ![](visualization_data_operation_cl_cn_1.jpg)

    2. 选择创建集合所属的集合空间。这里仍然默认选**foo**。

    ![](visualization_data_operation_cl_cn_2.jpg)

    3. **集合类型** 选 **垂直分区**。

    ![](visualization_data_operation_maincl_cn_1.jpg)

    4. 填写新建集合的名字。

    **Note: ** 集合名可以自由取。

    ![](visualization_data_operation_maincl_cn_2.jpg)

    5. 填写分区键，这里演示取 **id** 和 **time**，详细说明参考 [基本操作 - 数据分区 - 分区键](SdbDoc_Cn/basic_operation/sharding/shardingkey.html) 。

    **Note: ** 分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

    ![](visualization_data_operation_maincl_cn_3.jpg)

    ![](visualization_data_operation_maincl_cn_4.jpg)

    ![](visualization_data_operation_maincl_cn_5.jpg)

    6. 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

    7. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_maincl_cn_6.jpg)

    8. 等待创建结束。

    ![](visualization_data_operation_cl_cn_5.jpg)

    9. 创建成功。

    ![](visualization_data_operation_maincl_cn_7.jpg)

###切分数据###

-   

    **准备工作： 创建集群模式的SequoiaDB业务(必须有2个或以上的分区组)、创建集合空间、创建水平分区的集合并且插入一定数量的记录。**

    **Note:** 列表中必须至少有1个水平分区集合，才可以做**切分数据**操作。

    ![](visualization_data_operation_split_cn_1.jpg)

    1. 点击&lt;**切分数据**&gt;。

    ![](visualization_data_operation_split_cn_2.jpg)


    2. 选择 **切分方式**，实际根据需求选择，这里使用默认**百分比切分**。

    ![](visualization_data_operation_split_cn_3.jpg)

    3. 选择要**切分数据**的集合。

    ![](visualization_data_operation_split_cn_4.jpg)

    4. 设置**源分区组**和**目标分区组**。

    ![](visualization_data_operation_split_cn_5.jpg)

    ![](visualization_data_operation_split_cn_6.jpg)

    5. 设置**百分比切分**。

    ![](visualization_data_operation_split_cn_7.jpg)

    6. 点击&lt;**确定**&gt;。

    ![](visualization_data_operation_split_cn_8.jpg)

    7. 等待切分操作结束。

    ![](visualization_data_operation_split_cn_9.jpg)

    8. 切分数据完成。

    ![](visualization_data_operation_split_cn_10.jpg)

    ![](visualization_data_operation_split_cn_11.jpg)


###挂载###

-   

    **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间、创建1个垂直分区的集合、创建1个或更多普通集合或水平分区集合。**

    **Note:** 列表中必须至少有1个垂直分区集合，1个普通集合或水平分区集合，才可以做**挂载**操作。

    ![](visualization_data_operation_attach_cn_1.jpg)

    1. 点击&lt;**挂载集合**&gt;。

    ![](visualization_data_operation_attach_cn_2.jpg)

    2. 选择<垂直分区>的**集合**。

    ![](visualization_data_operation_attach_cn_3.jpg)

    3. 选择要挂载的**集合**。

    ![](visualization_data_operation_attach_cn_4.jpg)

    4. 填写分区范围，这里演示取 **time**。

    **Note: ** 分区范围可以有多个字段，通过后面的 **“+”** 和 **“-”** 可以添加或删除。

    把 **foo.y2014** 挂载到 **foo.main**。

    ![](visualization_data_operation_attach_cn_5.jpg)

    把 **foo.y2015** 挂载到 **foo.main**。

    ![](visualization_data_operation_attach_cn_6.jpg)

    5. 点击&lt;**确定**&gt;。

    6. 挂载完成。

    ![](visualization_data_operation_attach_cn_7.jpg)

    7. 如果不希望在列表中呈现子集合，可以设置**屏蔽子集合**。

    ![](visualization_data_operation_attach_cn_8.jpg)