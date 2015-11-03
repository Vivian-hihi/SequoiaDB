## 语法##
***db.listDomains()***

枚举域，执行此方法会显示系统中所有由用户创建的域。

## 示例##

<pre class="prettyprint lang-javascript">
> db.listDomains()</pre>

返回：

<pre class="prettyprint lang-diy">
{
  "_id": {
    "$oid": "539ea19669d195f36432111a"
  },
  "Name": "hello",
  "Groups": 
  [
    {
      "GroupName": "data1",
      "GroupID": 1000
    },
    {
      "GroupName": "data2",
      "GroupID": 1001
    }
  ]
}</pre>
