##描述##

集合空间快照 SDB_SNAP_COLLECTIONSPACES 列出当前数据库节点中所有的集合空间，每个集合空间为一条记录。

##标示##

SDB_SNAP_COLLECTIONSPACES

##字段信息##

由于数据节点与编目节点保存的集合空间信息不同，集合空间快照在协调节点与其它节点所返回的结构有所不同：

##非协调节点字段信息##

| 字段名          | 类型       | 描述                           |
| --------------- | ---------- | ------------------------------ |
| Name            | 字符串     | 集合空间名                     |
| Collection      | 字符串数组 | 集合空间中所包含的所有集合     |
| PageSize        | 整型       | 集合空间数据页大小             |
| LobPageSize     | 整型       | 集合空间大对象数据页大小       |
| MaxCapacitySize | 长整型     | 集合空间的最大容量上限         |
| MaxDataCapSize  | 长整型     | 集合空间数据文件最大容量上限   |
| MaxIndexCapSize | 长整型     | 集合空间索引文件最大容量上限   |
| MaxLobCapSize   | 长整型     | 集合空间大对象文件最大容量上限 |
| NumCollections  | 整型       | 集合数量                       |
| TotalRecords    | 整型       | 集合空间的记录总数             |
| TotalSize       | 长整型     | 集合空间的总大小               |
| FreeSize        | 长整型     | 集合空间的空闲大小             |
| TotalDataSize   | 长整型     | 集合空间数据文件总大小         |
| FreeDataSize    | 长整型     | 集合空间数据文件空闲空间大小   |
| TotalIndexSize  | 长整型     | 集合空间索引文件总大小         |
| FreeIndexSize   | 长整型     | 集合空间索引文件空闲空间大小   |
| TotalLobSize    | 长整型     | 集合空间大对象文件总大小       |
| FreeLobSize     | 长整型     | 集合空间大对象文件空闲空间大小 |

##协调节点字段信息##

| 字段名          | 类型       | 描述                           |
| --------------- | ---------- | ------------------------------ |
| Name            | 字符串     | 集合空间名                     |
| Collection      | 字符串数组 | 集合空间中所包含的所有集合     |
| PageSize        | 整型       | 集合空间数据页大小             |
| LobPageSize     | 整型       | 集合空间大对象数据页大小       |
| TotalSize       | 长整型     | 集合空间的总大小               |
| FreeSize        | 长整型     | 集合空间的空闲大小             |
| TotalDataSize   | 长整型     | 集合空间数据文件总大小         |
| FreeDataSize    | 长整型     | 集合空间数据文件空闲空间大小   |
| TotalIndexSize  | 长整型     | 集合空间索引文件总大小         |
| FreeIndexSize   | 长整型     | 集合空间索引文件空闲空间大小   |
| TotalLobSize    | 长整型     | 集合空间大对象文件总大小       |
| FreeLobSize     | 长整型     | 集合空间大对象文件空闲空间大小 |
| Group           | 字符串数组 | 该集合空间所在的分区组名列表   |

##非协调节点示例##

```lang-javascript
> db.snapshot( SDB_SNAP_COLLECTIONSPACES )
{
  "Collection": [
    {
      "Name": "bar"
    }
  ],
  "PageSize": 65536,
  "LobPageSize": 262144,
  "MaxCapacitySize": 26388279066624,  
  "MaxDataCapSize": 8796093022208,
  "MaxIndexCapSize": 8796093022208,
  "MaxLobCapSize": 8796093022208,
  "NumCollections": 4,
  "TotalRecords": 2,
  "TotalSize": 306315264,
  "FreeSize": 265551224,
  "TotalDataSize": 155254784,
  "FreeDataSize": 133627904,
  "TotalIndexSize": 151060480,
  "FreeIndexSize": 134152171,
  "TotalLobSize": 352714752,
  "FreeLobSize": 140771328,
  "Name": "foo"
}
```

##协调节点示例##

```lang-javascript
> coord.snapshot( SDB_SNAP_COLLECTIONSPACES )
{
  "Name": "foo",
  "PageSize": 4096,  
  "LobPageSize": 262144,
  "TotalSize": 918945792,
  "FreeSize": 805183062,  
  "TotalDataSize": 155254784,
  "FreeDataSize": 133627904,
  "TotalIndexSize": 151060480,
  "FreeIndexSize": 134152171,
  "TotalLobSize": 352714752,
  "FreeLobSize": 140771328,
  "Collection": [
    {
      "Name": "bar"
    }
  ],
  "Group": [
    "group1"
  ]
}
```
