##语法##
***db.setSessionAttr (&lt;options&gt;)***

设置会话属性

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 会话属性选项 | 是 |

### options 格式###

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| PreferedInstance | 会话读操作优先选取的数据库实例标识；取值"m"/"M"/"s"/"S"/"a"/"A"/1-7，分别表示 master/slave/anyone/node1-node7 | PreferedInstance:"M" |

## 示例##

* 设置会话优先从“主”数据库实例获取数据

<pre class="prettyprint lang-javascript">
> db.setSessionAttr({PreferedInstance:"M"})</pre>
