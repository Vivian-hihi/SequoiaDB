## 语法##
***rg.getDetail()***

返回分区组的信息。

## 示例##

<pre class="prettyprint lang-javascript">
> rg.getDetail()</pre>

返回：
<pre class="prettyprint lang-diy">
{
  "Group": [
    {
      "dbpath": "/opt/sequoiadb/data/11800",
      "HostName": "vmsvr2-suse-x64",
      "Service": [
        {
          "Type": 0,
          "Name": "11800"
        },
        {
          "Type": 1,
          "Name": "11801"
        },
        {
          "Type": 2,
          "Name": "11802"
        },
        {
          "Type": 3,
          "Name": "11803"
        }
      ],
      "NodeID": 1000
    },
    {
      "dbpath": "/opt/sequoiadb/data/11850",
      "HostName": "vmsvr2-suse-x64",
      "Service": [
        {
          "Type": 0,
          "Name": "11850"
        },
        {
          "Type": 1,
          "Name": "11851"
        },
        {
          "Type": 2,
          "Name": "11852"
        },
        {
          "Type": 3,
          "Name": "11853"
        }
      ],
      "NodeID": 1001
    }
  ],
  "GroupID": 1001,
  "GroupName": "group",
  "PrimaryNode": 1001,
  "Role": 0,
  "Status": 1,
  "Version": 3,
  "_id": {
    "$oid": "517b2fc33d7e6f820fc0eb57"
  }
}</pre>
