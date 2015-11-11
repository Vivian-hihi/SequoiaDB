###创建集合空间###

   1.1 点击&lt;**数据库操作**&gt;。

   ![](visualization_data_operation_cs_cn_1.jpg)

   1.2 点击&lt;**创建集合空间**&gt;。

   ![](visualization_data_operation_cs_cn_2.jpg)

   1.3 填写新建集合空间的名字，其他参数不是必填项，可以根据实际情况设置。

   ![](visualization_data_operation_cs_cn_3.jpg)

   1.4 其他参数说明，请参考 [数据模型 - 集合空间](SdbDoc_Cn/data_model/collectionspace.html) 。

   1.5 点击&lt;**确定**&gt;。

   ![](visualization_data_operation_cs_cn_4.jpg)

   1.6 创建成功;。

   ![](visualization_data_operation_cs_cn_5.jpg)

###创建集合（普通类型）###

   1.1 点击&lt;**创建集合**&gt;。

   ![](visualization_data_operation_cl_cn_1.jpg)

   1.2 选择创建集合所属的集合空间。因为只有刚刚创建的**集合空间foo**，所以自动选择了**foo**。

   ![](visualization_data_operation_cl_cn_2.jpg)

   1.3 现在要创建一个 **普通类型** 的集合，因此&lt;**集合类型**&gt;不需要修改，填写新建集合的名字。
   **Note: ** 集合名可以自由取。

   ![](visualization_data_operation_cl_cn_3.jpg)

   1.4 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

   1.5 点击&lt;**确定**&gt;。

   ![](visualization_data_operation_cl_cn_4.jpg)

   1.6 等待创建结束。

   ![](visualization_data_operation_cl_cn_5.jpg)

   1.7 创建成功。

   ![](visualization_data_operation_cl_cn_6.jpg)

###创建集合（水平范围分区）###

   1.1 点击&lt;**创建集合**&gt;。

   ![](visualization_data_operation_cl_cn_1.jpg)

   1.2 选择创建集合所属的集合空间。这里仍然默认选**foo**。

   ![](visualization_data_operation_cl_cn_2.jpg)

   1.3 **集合类型** 选 **水平范围分区**。

   ![](visualization_data_operation_cl_cn_7.jpg)

   1.4 填写新建集合的名字。

   **Note: ** 集合名可以自由取。

   ![](visualization_data_operation_cl_cn_8.jpg)

   1.5 填写分区键，这里演示取 **id** 和 **time**，详细说明参考 [基本操作 - 数据分区 - 分区键](SdbDoc_Cn/basic_operation/sharding/shardingkey.html) 。

   **Note: ** 分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

   ![](visualization_data_operation_cl_cn_9.jpg)

   ![](visualization_data_operation_cl_cn_10.jpg)

   ![](visualization_data_operation_cl_cn_11.jpg)
   
   1.6 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

   1.7 点击&lt;**确定**&gt;。

   ![](visualization_data_operation_cl_cn_4.jpg)

   1.8 等待创建结束。

   ![](visualization_data_operation_cl_cn_5.jpg)

   1.9 创建成功。

   ![](visualization_data_operation_cl_cn_12.jpg)

###创建集合（水平散列分区）###

   1.1 点击&lt;**创建集合**&gt;。

   ![](visualization_data_operation_cl_cn_1.jpg)

   1.2 选择创建集合所属的集合空间。这里仍然默认选**foo**。

   ![](visualization_data_operation_cl_cn_2.jpg)

   1.3 **集合类型** 选 **水平范围分区**。

   ![](visualization_data_operation_cl_cn_18.jpg)

   1.4 填写新建集合的名字。

   **Note: ** 集合名可以自由取。

   ![](visualization_data_operation_cl_cn_13.jpg)

   1.5 填写分区键，这里演示取 **id** 和 **time**，详细说明参考 [基本操作 - 数据分区 - 分区键](SdbDoc_Cn/basic_operation/sharding/shardingkey.html) 。

   **Note: ** 分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

   ![](visualization_data_operation_cl_cn_14.jpg)

   ![](visualization_data_operation_cl_cn_15.jpg)

   ![](visualization_data_operation_cl_cn_16.jpg)
   
   1.6 其他参数说明，请参考 [数据模型 - 集合](SdbDoc_Cn/data_model/collection.html) 。

   1.7 点击&lt;**确定**&gt;。

   ![](visualization_data_operation_cl_cn_4.jpg)

   1.8 等待创建结束。

   ![](visualization_data_operation_cl_cn_5.jpg)

   1.9 创建成功。

   ![](visualization_data_operation_cl_cn_17.jpg)

























