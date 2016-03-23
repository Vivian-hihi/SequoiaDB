## 语法##
***db.collectionspace.collection.alter(&lt;options&gt;)***

修改集合的属性。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 修改的属性。 | 是 |

### options 中可选的属性格式###

| 参数名 | 描述 | 格式 |
| ------ | ------ | ------ |
| ReplSize | 一次写请求完成副本数。 | ReplSize: &lt;int32&gt; |
| ShardingKey | 分区键 | ShardingKey:{&lt;字段1&gt;:&lt;1\|-1&gt;,[&lt;字段2&gt;:&lt;1\|-1&gt;, ...]} |
| ShardingType | 分区方式，默认为 range 分区。 | ShardingType:"hash"\|"range" |
| Partition | 分区数，hash 分区时填写，代表了 hash 分区的个数。其值必须是2的幂。范围在[2^3 , 2^20]。 | Partition:&lt;分区数&gt; |

**Note:**

* ShardingKey，ShardingType，Partition 的使用方式见 db.collectionspace.createCL()。
* 分区集合不能修改与分区相关的属性。
* 修改为分区集合后需要手动进行 split。

## 示例##

* 创建一个普通集合；

<pre class="prettyprint lang-javascript">
> db.foo.createCL('bar')</pre>

* 修改为分区集合

<pre class="prettyprint lang-javascript">
> db.foo.bar.alter({ShardingKey:{a:1},ShardingType:"hash"})</pre>
