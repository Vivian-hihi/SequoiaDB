##语法##
***db.collectionspace.collection.truncate()***

truncate 会删除集合内所有数据（包括普通文档和 LOB 数据），但不会影响其元数据。与 remove 需要按照条件筛选目标不同，truncate 会直接释放数据页，在清空集合（尤其是大数据量下）数据时效率比 remove 更加高效。

##示例##

-   我们在集合 foo.bar 中插入了普通数据和 LOB 数据。通过快照查看其数据页使用情况：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_COLLECTIONS) ;
{
  "Name": "foo.bar",
  "Details": [
    {
      "GroupName": "datagroup",
      "Group": [
        {
          "ID": 0,
          "LogicalID": 0,
          "Sequence": 1,
          "Indexes": 1,
          "Status": "Normal",
          "TotalRecords": 10000,
          "TotalDataPages": 33,
          "TotalIndexPages": 7,
          "TotalLobPages": 36,
          "TotalDataFreeSpace": 41500,
          "TotalIndexFreeSpace": 103090
        }
      ]
    }
  ]
}</pre>

可以看到其中数据页为33，索引页为7，LOB 页为36。下面执行 truncate 操作：

<pre class="prettyprint lang-javascript">
> db.foo.bar.truncate();</pre>

再次通过快照查看数据页使用情况：

<pre class="prettyprint lang-javascript">
> db.snapshot(SDB_SNAP_COLLECTIONS) ;
{
  "Name": "foo.bar",
  "Details": [
    {
      "GroupName": "datagroup",
      "Group": [
        {
          "ID": 0,
          "LogicalID": 0,
          "Sequence": 1,
          "Indexes": 1,
          "Status": "Normal",
          "TotalRecords": 0,
          "TotalDataPages": 0,
          "TotalIndexPages": 2,
          "TotalLobPages": 0,
          "TotalDataFreeSpace": 0,
          "TotalIndexFreeSpace": 65515
        }
      ]
    }
  ]
}</pre>

可以看到除索引页为2（存储了索引的元数据信息）外，其余数据页已经全部被释放了。
