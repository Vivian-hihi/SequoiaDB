##名称##

createCL - 创建一个新的集合。

##语法##

***db.collectionspace.createCL(\<name\>,[options])***

##类别##

Collection Space

##描述##

在指定集合空间下创建集合（Collection），集合是数据库中存放文档记录的逻辑对象，
任何一条文档记录必须属于一个且仅属于一个集合。

##参数##

* `name` ( *String*， *必填* )

    集合名，在同一个集合空间中，集合名必须唯一。

* `options` ( *Object*， *选填* )

    在创建集合时，可以通过`options`参数设置集合的其他属性，如指定集合的分区键，
    是否以压缩的形式插入数据等。可组合使用 `options` 的如下选项：

    1. `ShardingKey` ( *Object* )：分区键。

        格式：`ShardingKey:{<字段1> : <1|-1>,[<字段2> : <1|-1>, ...]}`

    2. `ShardingType` ( *String* )：分区方式。默认为 hash 分区。其可选取值如下：

        * "hash"：hash 分区。
        * "range"：范围分区。

        格式：`ShardingType:"hash"|"range"`

    3. `Partition` ( *Int32* )：分区数。仅当选择 hash 分区时填写，
                                代表了 hash 分区的个数。其值必须是2的幂。
                                范围在[2\^3，2\^20]。默认为4096。

        格式：`Partition: <分区数>`

    4. `ReplSize` ( *Int32* )：写操作需同步的副本数。默认值为1。其可选取值如下：

        * -1：表示写请求需同步到该复制组若干活跃的节点之后，数据库写操作才
              返回应答给客户端。
        * 0：表示写请求需同步到该复制组的所有节点之后，数据库写操作才返回应
             答给客户端。
        * 1 - 7：表示写请求需同步到该复制组指定数量个节点之后，数据库写操作
                 才返回应答给客户端。

        格式：`ReplSize: <num>`

    5. `Compressed` ( *Bool* )：标识新集合是否开启数据压缩功能。默认为 false。

        格式：`Compressed:true|false`

    6. `CompressionType` ( *String* )：压缩算法类型。默认为 snappy 算法。
                                       其可选取值如下：

        * "snappy"：使用 snappy 算法压缩。
        * "lzw"：使用 lzw 算法压缩。

        格式：`CompressionType:"snappy"|"lzw"`

    7. `IsMainCL` ( *Bool* )：标识新集合是否为主分区集合，默认为 false。

        格式：`IsMainCL:true|false`

    8. `AutoSplit` ( *Bool* )：标识新集合是否开启自动切分功能，默认为 false。

        格式：`AutoSplit:true|false`

    9. `Group` ( *String* )：指定新集合将被创建到哪个复制组。

        格式：`Group:<group name>`

    10. `AutoIndexId` ( *Bool* )：标识新集合是否自动使用_id字段创建名字为"$id"的
                                  唯一索引，默认为 true。
 
        格式：`AutoIndexId:true|false`

    11. `EnsureShardingIndex` ( *Bool* )：标识集合是否自动使用ShardingKey包含的
                                          字段创建名字为"$shard"的索引，默认为true。

      格式：`EnsureShardingIndex:true|false`

    12. `StrictDataMode` ( *Bool* )：标识对该集合的操作是否开启严格数据类型模式，默认为false(不开启)。

	    严格数据模式的开启标识对数值操作存在以下限制：

	   * 运算过程不改数据类型；
	  * 数值运算出现溢出时直接报错，错误码SDB_VALUE_OVERFLOW；


      格式：`StrictDataMode:true|false`

    **注意：**

    * 参数 `name` 的值不能是空串、含点（.）或者美元符号（$），
      并且长度不能超过127B，否则操作失败。

    * 当参数 `options` 内设置了多个参数时，需用英文半角的逗号","将各参数的取值隔开。
  
    * 新集合从属于创建它的集合空间。若该集合空间从属于某个域，相关于新集合也从属于该域；
      若该集合空间不从属于任何域，相当于新集合也不从属于任何域。

    * 使用 Group 参数时，若新集合从属于某个域，该参数所指定的复制组也必须存在于该域；
      若新集合不从属于任何域，该参数所指定的复制组可以为当前集群的任意一个复制组；
 
    * 不使用 Group 参数时，若新集合从属于某个域，该集合将被创建到该域任意一个复制组中；
      若新集合不从属于任何域，该集合将被创建到当前集群的任意一个复制组中。
    
    * AutoSplit 不能与 Group 参数同时使用。

    * AutoSplit 必须配合散列分区使用。

    * 使用 AutoSplit 参数时，若该集合从属于某个域，当前的 AutoSplit 参数将
      覆盖域中的 AutoSplit 参数作用于当前集合；

    * 使用 AutoSplit 参数时，若该集合不从属于某个域，集合将在系统域 SYSDOMAIN 上进行自动切分；

    * 不使用 AutoSplit 参数时，若该集合从属于某个域，该域的 AutoSplit 参数将
      作用于当前集合。

##返回值##

成功：返回一个新的SdbCollection对象。  

失败：抛出异常。

##错误##

`createCL()`函数常见异常如下：

| 错误码 | 错误类型 | 描述 | 解决方法 |
| ------ | ------ | --- | ------ |
| -2 | SDB_OOM | 无可用内存。| 检查物理内存及虚拟内存的情况。|
| -6 | SDB_INVALIDARG | 参数错误。 | 查看参数是否填写正确。|
| -22 | SDB_DMS_EXIST | 集合已存在。| 检查集合是否存在。|
| -34 | SDB_DMS_CS_NOTEXIST | 集合空间不存在。| 检查集合空间是否存在。|
| -318 | SDB_VALUE_OVERFLOW | 数值运算出现溢出。| 检查运算过程是否存在溢出情况。|
当异常抛出时，可以通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取[错误码](reference/Sequoiadb_error_code.md)，
或通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息。
可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)了解更多内容。

##版本##

v1.0及以上版本。

##例子##

1. 在集合空间 foo 下创建集合 bar，不指定分区键。

    ```lang-javascript
    > db.foo.createCL("bar")
    localhost:11810.foo.bar
    Takes 0.120450s.
    ```

2. 在集合空间 foo 下创建集合 bar1。新集合若需要切分数据到其它复制组，将
   使用 a 字段进行 hash 切分；新集合开启了数据压缩功能，使用默认的 snappy 算法压缩数据；写操作作用于新集合时，只需写入主节点即可返还。

    ```lang-javascript
    > db.foo.createCL("bar1",{ShardingKey:{age:1}, ShardingType:"hash", 
                      Partition:4096, Compressed:true, ReplSize:1})
    localhost:11810.foo.bar1
    Takes 0.110319s.
    ```
3. 在集合空间 foo 下创建集合 bar，开启严格数据类型模式。

    ```lang-javascript
    > db.foo.createCL("bar", {StrictDataMode: true})
    localhost:11810.foo.bar
    Takes 0.120450s.
    ```
