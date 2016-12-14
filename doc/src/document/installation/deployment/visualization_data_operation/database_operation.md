###页面简介###

![页面简介](installation/deployment/visualization_data_operation/visualization_data_operation_cn_10.jpg)

###创建集合空间###

- **准备工作： 创建SequoiaDB业务。**
  1. 点击< **数据库操作** >。  
     ![点击数据库操作](installation/deployment/visualization_data_operation/visualization_data_operation_cs_cn_1.jpg)  
  2. 点击< **创建集合空间** >。  
     ![点击创建集合空间](installation/deployment/visualization_data_operation/visualization_data_operation_cs_cn_2.jpg)  
  3. 填写 **集合空间名** ，其他参数不是必填项，可以根据实际情况设置。  
     ![填写集合空间参数](installation/deployment/visualization_data_operation/visualization_data_operation_cs_cn_3.jpg)    
  4. 其他参数说明，请参考 [数据模型 - 集合空间](data_model/collectionspace.md) 。  
  5. 点击< **确定** >。  
     ![确定创建集合空间](installation/deployment/visualization_data_operation/visualization_data_operation_cs_cn_4.jpg)  
  6. 创建成功;。  
     ![成功创建集合空间](installation/deployment/visualization_data_operation/visualization_data_operation_cs_cn_5.jpg)  

###创建集合（普通类型）###

- **准备工作： 创建SequoiaDB业务、创建集合空间。**
  1. 点击< **创建集合** >。  
     ![创建集合](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_1.jpg)    
  2. 选择创建集合所属的集合空间。因为只有刚刚创建的 **集合空间foo** ，所以自动选择了 **foo** 。  
     ![选择集合所属集合空间](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_2.jpg)    
  3. 现在要创建一个 **普通类型** 的集合，因此< **集合类型** >不需要修改，填写新建集合的名字。  
    
     >**Note:**  
     >集合名可以自由取。  

     ![填写集合参数](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_3.jpg)  
  4. 其他参数说明，请参考 [数据模型 - 集合](data_model/collection.md) 。  
  5. 点击< **确定** >。  
     ![确认创建集合](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_4.jpg)    
  6. 等待创建结束。  
     ![等待创建结束](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_5.jpg)    
  7. 创建成功。  
     ![创建集合成功](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_6.jpg)  

###创建集合（水平范围分区）###

- **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**
  1. 点击< **创建集合** >。  
     ![创建集合](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_1.jpg)  
  2. 选择创建集合所属的集合空间。这里仍然默认选 **foo** 。  
     ![填写集合参数](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_2.jpg)  
  3. **集合类型** 选 **水平范围分区** 。
     ![调整集合类型为水平范围分区](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_7.jpg)  
  4. 填写新建集合的名字。

     >**Note:**  
     >集合名可以自由取。

     ![填写新集合名字](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_8.jpg)  
  5. 填写分区键，这里演示取 **id** 和 **time** ，详细说明参考 [基本操作 - 数据分区 - 分区键](basic_operation/sharding/shardingkey.md) 。  

     >**Note:**  
     >分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

     ![分区键设置1](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_9.jpg)  

     ![分区键设置2](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_10.jpg)  

     ![分区键设置3](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_11.jpg)  
  6. 其他参数说明，请参考 [数据模型 - 集合](data_model/collection.md) 。  
  7. 点击< **确定** >。  
     ![确认创建集合](installation/deployment/visualization_data_operation/visualization_data_operation_rangecl_cn_1.jpg)  
  8. 等待创建结束。  
     ![等待创建结束](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_5.jpg)  
  9. 创建成功。  
     ![创建集合成功](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_12.jpg)    

###创建集合（水平散列分区）###

- **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**
  1. 点击< **创建集合** >。  
     ![创建集合](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_1.jpg)    
  2. 选择创建集合所属的集合空间。这里仍然默认选 **foo** 。  
     ![选择所属集合空间](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_2.jpg)  
  3. **集合类型** 选 **水平散列分区** 。  
     ![设置集合类型](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_18.jpg)    
  4. 填写新建集合的名字。  

     >**Note:**  
     >集合名可以自由取。

     ![填写集合名](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_13.jpg)  
  5. 填写分区键，这里演示取 **id** 和 **time** ，详细说明参考 [基本操作 - 数据分区 - 分区键](basic_operation/sharding/shardingkey.md) 。  

     >**Note:**  
     >分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。

     ![设置分区键1](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_14.jpg)    

     ![设置分区键2](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_15.jpg)  

     ![设置分区键3](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_16.jpg)  
  6. 其他参数说明，请参考 [数据模型 - 集合](data_model/collection.md) 。  
  7. 点击< **确定** >。  
     ![确定创建](installation/deployment/visualization_data_operation/visualization_data_operation_hashcl_cn_1.jpg)  
  8. 等待创建结束。  
     ![等待创建结束](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_5.jpg)  
  9. 创建成功。  
     ![创建成功](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_17.jpg)  

###创建集合（垂直分区）###

- **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间。**
  1. 点击< **创建集合** >。  
     ![点击创建集合](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_1.jpg)  
  2. 选择创建集合所属的集合空间。这里仍然默认选 **foo** 。  
     ![选择所属集合空间](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_2.jpg)  
  3. **集合类型** 选 **垂直分区** 。  
     ![选择集合类型](installation/deployment/visualization_data_operation/visualization_data_operation_maincl_cn_1.jpg)  
  4. 填写新建集合的名字。  

     >**Note:**  
     >集合名可以自由取。  

     ![填写集合名字](installation/deployment/visualization_data_operation/visualization_data_operation_maincl_cn_2.jpg)  
  5. 填写分区键，这里演示取 **id** 和 **time** ，详细说明参考 [基本操作 - 数据分区 - 分区键](basic_operation/sharding/shardingkey.md) 。  

     >**Note:**  
     >分区键可以有多个，通过后面的 **“+”** 和 **“-”** 可以添加或删除分区键。  

     ![设置分区键1](installation/deployment/visualization_data_operation/visualization_data_operation_maincl_cn_3.jpg)  

     ![设置分区键2](installation/deployment/visualization_data_operation/visualization_data_operation_maincl_cn_4.jpg)  

     ![设置分区键3](installation/deployment/visualization_data_operation/visualization_data_operation_maincl_cn_5.jpg)  
  6. 其他参数说明，请参考 [数据模型 - 集合](data_model/collection.md) 。  
  7. 点击< **确定**>。  
     ![点击确定](installation/deployment/visualization_data_operation/visualization_data_operation_maincl_cn_6.jpg)  
  8. 等待创建结束。  
     ![等待创建结束](installation/deployment/visualization_data_operation/visualization_data_operation_cl_cn_5.jpg)  
  9. 创建成功。  
     ![创建成功](installation/deployment/visualization_data_operation/visualization_data_operation_maincl_cn_7.jpg)  

###切分数据###

- **准备工作： 创建集群模式的SequoiaDB业务(必须有2个或以上的分区组)、创建集合空间、创建水平分区的集合并且插入一定数量的记录。**

  >**Note:**  
  >列表中必须至少有1个水平分区集合，才可以做 **切分数据** 操作。  

  ![至少存在一个水平分区集合](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_1.jpg)  

  1. 点击< **切分数据** >。  
     ![点击切分数据](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_2.jpg)  
  2. 选择 **切分方式** ，实际根据需求选择，这里使用默认 **百分比切分** 。  
     ![选择切分方式](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_3.jpg)  
  3. 选择要 **切分数据** 的集合。  
     ![选择集合](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_4.jpg)  
  4. 设置 **源分区组** 和 **目标分区组** 。  
     ![设置源和目标1](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_5.jpg)  

     ![设置源和目标2](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_6.jpg)  
  5. 设置 **百分比切分** 。  
     ![设置切分百分比](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_7.jpg)  
  6. 点击< **确定** >。  
     ![确认切分](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_8.jpg)  
  7. 等待切分操作结束。  
     ![等待切分结束](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_9.jpg)  
  8. 切分数据完成。  
     ![切分完成1](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_10.jpg)  

     ![切分完成2](installation/deployment/visualization_data_operation/visualization_data_operation_split_cn_11.jpg)    


###挂载###

- **准备工作： 创建集群模式的SequoiaDB业务、创建集合空间、创建1个垂直分区的集合、创建1个或更多普通集合或水平分区集合。**  

  >**Note:**  
  >列表中必须至少有1个垂直分区集合，1个普通集合或水平分区集合，才可以做去 **挂载** 操作。  

  ![确认存在分区集合](installation/deployment/visualization_data_operation/visualization_data_operation_attach_cn_1.jpg)  
  
  1. 点击< **挂载集合** >。  
     ![点击挂载集合](installation/deployment/visualization_data_operation/visualization_data_operation_attach_cn_2.jpg)  
  2. 选择< **垂直分区** >的 **集合**。  
     ![选择集合](installation/deployment/visualization_data_operation/visualization_data_operation_attach_cn_3.jpg)  
  3. 选择要挂载的 **集合** 。  
     ![选择挂载集合](installation/deployment/visualization_data_operation/visualization_data_operation_attach_cn_4.jpg)  
  4. 填写分区范围，这里演示取 **time** 。  

     >**Note:**  
     >分区范围可以有多个字段，通过后面的 **“+”** 和 **“-”** 可以添加或删除。  

     把 **foo.y2014** 挂载到 **foo.main** 。  

     ![选择分区范围](installation/deployment/visualization_data_operation/visualization_data_operation_attach_cn_5.jpg)  

     把 **foo.y2015** 挂载到 **foo.main** 。  

     ![完成挂载参数设置](installation/deployment/visualization_data_operation/visualization_data_operation_attach_cn_6.jpg)
  5. 点击< **确定** >。  
  6. 挂载完成。  
     ![挂载完成](installation/deployment/visualization_data_operation/visualization_data_operation_attach_cn_7.jpg)  
  7. 如果不希望在列表中呈现子集合，可以设置 **屏蔽子集合** 。  
     ![屏蔽子集合](installation/deployment/visualization_data_operation/visualization_data_operation_attach_cn_8.jpg)  