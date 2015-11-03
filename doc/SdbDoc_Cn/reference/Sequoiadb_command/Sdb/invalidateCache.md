## 语法##
***db.invalidateCache([options])***

清除节点（数据节点/协调节点）的缓存。

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 清除缓存的选项 | 否 |

### options 格式###

目前通过 options 可设置域的属性有：

<table>
    <tr>
        <th>属性名</th>
        <th>描述</th>
        <th>格式</th>
    </tr>
    <tr>
        <td>Groups</td>
        <td>需要清除缓存的目标。</td>
        <td><p>Groups:null -- 当前协调节点；
        <p>Groups:['data1','data2'] -- 当前协调节点和指定的两个数据组；
        <p>Groups:'data1' -- 当前协调节点和指定的一个数据组。 </td>
    </tr>
</table>

**Note:**

当不指定 Groups 时，作用域为当前协调节点和所有数据节点。

## 示例##

* 清除当前协调节点和数据组‘data1’的缓存信息。

<pre class="prettyprint lang-javascript">
> db.invalidateCache({Groups:'data1'})</pre>
