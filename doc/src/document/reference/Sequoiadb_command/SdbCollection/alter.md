##名称##

alter - 修改集合的属性。

##语法##
**db.collectionspace.collection.alter(\<options\>)**

##类别##

Collection

##描述##

修改集合的属性。

##参数##

* `options` ( *Object*， *必填* )

    通过`options`参数可以修改集合属性，如指定集合的分区键，
    是否以压缩的形式插入数据等。可组合使用 `options` 的如下选项：

    1. `ReplSize` ( *Int32* )：写操作需同步的副本数。其可选取值如下：

        * -1：表示写请求需同步到该复制组若干活跃的节点之后，数据库写操作才
              返回应答给客户端。
        * 0：表示写请求需同步到该复制组的所有节点之后，数据库写操作才返回应
             答给客户端。
        * 1 - 7：表示写请求需同步到该复制组指定数量个节点之后，数据库写操作
                 才返回应答给客户端。

        格式：`ReplSize: <num>`

    2. `ShardingKey` ( *Object* )：分区键。

        格式：`ShardingKey:{<字段1> : <1|-1>,[<字段2> : <1|-1>, ...]}`

    3. `ShardingType` ( *String* )：分区方式，默认为 hash 分区。其可选取值如下：

        * "hash"：hash 分区。
        * "range"：范围分区。

        格式：`ShardingType:"hash"|"range"`

    4. `Partition` ( *Int32* )：分区数。仅当选择 hash 分区时填写，
                                代表了 hash 分区的个数。其值必须是2的幂。
                                范围在[2\^3，2\^20]。

        格式：`Partition: <分区数>`

	**Note:**

	* ShardingKey，ShardingType，Partition 的使用方式见 db.collectionspace.createCL()。
	* 分区集合不能修改与分区相关的属性。
	* 修改为分区集合后需要手动进行 split。

##返回值##

成功：无。  

失败：抛出异常。

##错误##

`alter()`函数常见异常如下：

| 错误码 | 错误类型 | 可能的原因 | 解决方法 |
| ------ | --- | ------------ | ----------- |
| -32 | SDB_OPTION_NOT_SUPPORT | 选项暂不支持 | 检查当前集合属性，如果是分区集合不能修改与分区相关的属性。|

当异常抛出时，可以通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取[错误码](reference/Sequoiadb_error_code.md)，
或通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息。
可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)了解更多内容。

##版本##

v1.12及以上版本。

##示例##

1. 创建一个普通集合，然后将该集合修改为分区集合。

	```lang-javascript
 	> db.foo.createCL('bar')
 	> db.foo.bar.alter( { ShardingKey: { a: 1 }, ShardingType: "hash" } )
	```
