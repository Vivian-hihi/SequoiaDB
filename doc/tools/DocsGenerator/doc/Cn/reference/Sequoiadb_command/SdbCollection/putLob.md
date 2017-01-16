##语法##
***db.collectionspace.collection.putLob(&lt;file path&gt;)***

在集合中插入大对象。

## 参数描述##
| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| file path | string | 待上传的文件全路径。 | 是 |

**Note:**

* 上传大对象成功后会返回其 OID。
* 需要拥有文件的读权限。

## 示例##

* 创建集合空间与集合

<pre class="prettyprint lang-javascript">
> db.createCS('foo')
> db.foo.createCL('bar')</pre>

* 上传大对象文件

<pre class="prettyprint lang-javascript">
> db.foo.bar.putLob('/opt/mylob');</pre>
