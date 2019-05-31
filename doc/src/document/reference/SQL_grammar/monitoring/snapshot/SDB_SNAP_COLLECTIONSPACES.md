##描述##

集合空间快照 SNAPSHOT_CS 列出当前数据库节点中所有的集合空间，每个集合空间为一条记录。

##标示##

SNAPSHOT_CS

##字段信息##

由于数据节点与编目节点保存的集合空间信息不同，集合空间快照在协调节点与其它节点所返回的结构有所不同：

##非协调节点字段信息##

| 字段名          | 类型       | 描述                                         |
| --------------- | ---------- | -------------------------------------------- |
| NodeName        | 字符串     | 集合空间所属节点名（主机名：端口号）         |
| GroupName       | 字符串     | 集合空间所属分区组名                         |
| Name            | 字符串     | 集合空间名                                   |
| UniqueID        | 整型       | 集合空间的UniqueID，在集群上全局唯一         |
| Collection.Name | 字符串     | 集合空间所包含的集合的名字                   |
| Collection.UniqueID | 长整型 | 集合空间所包含的集合的UniqueID               |
| PageSize        | 整型       | 集合空间数据页大小（单位：字节）             |
| LobPageSize     | 整型       | 集合空间大对象数据页大小（单位：字节）       |
| MaxCapacitySize | 长整型     | 集合空间的最大容量上限（单位：字节）         |
| MaxDataCapSize  | 长整型     | 集合空间数据文件最大容量上限（单位：字节）   |
| MaxIndexCapSize | 长整型     | 集合空间索引文件最大容量上限（单位：字节）   |
| MaxLobCapSize   | 长整型     | 集合空间大对象文件最大容量上限（单位：字节） |
| NumCollections  | 整型       | 集合数量                                     |
| TotalRecords    | 整型       | 集合空间的记录总数                           |
| TotalSize       | 长整型     | 集合空间的总大小（单位：字节）               |
| FreeSize        | 长整型     | 集合空间的空闲大小（单位：字节）             |
| TotalDataSize   | 长整型     | 集合空间数据文件总大小（单位：字节）         |
| FreeDataSize    | 长整型     | 集合空间数据文件空闲空间大小（单位：字节）   |
| TotalIndexSize  | 长整型     | 集合空间索引文件总大小（单位：字节）         |
| FreeIndexSize   | 长整型     | 集合空间索引文件空闲空间大小（单位：字节）   |
| TotalLobSize    | 长整型     | 集合空间大对象文件总大小（单位：字节）       |
| FreeLobSize     | 长整型     | 集合空间大对象文件空闲空间大小（单位：字节） |
| DataCommitLSN   | 长整型     | 集合空间数据文件最后提交LSN                  |
| IndexCommitLSN  | 长整型     | 集合空间索引文件最后提交LSN                  |
| LobCommitLSN    | 长整型     | 集合空间大对象文件最后提交LSN                |
| DataCommitted   | 布尔型     | 集合空间数据文件当前是否有效提交             |
| IndexCommitted  | 布尔型     | 集合空间索引文件当前是否有效提交             |
| LobCommitted    | 布尔型     | 集合空间大对象文件当前是否有效提交           |
| DirtyPage       | 整型       | 集合空间大对象文件在开启缓存下脏页数量       |
| Type            | 整型       | 集合空间类型，0 表示普通集合空间，1 表示固定（Capped）集合空间 |

##协调节点字段信息##

| 字段名          | 类型       | 描述                                         |
| --------------- | ---------- | -------------------------------------------- |
| Name            | 字符串     | 集合空间名                                   |
| UniqueID        | 整型       | 集合空间的UniqueID，在集群上全局唯一         |
| Collection.Name | 字符串     | 集合空间所包含的集合的名字                   |
| Collection.UniqueID | 长整型 | 集合空间所包含的集合的UniqueID               |
| PageSize        | 整型       | 集合空间数据页大小（单位：字节）             |
| LobPageSize     | 整型       | 集合空间大对象数据页大小（单位：字节）       |
| TotalSize       | 长整型     | 集合空间的总大小（单位：字节）               |
| FreeSize        | 长整型     | 集合空间的空闲大小（单位：字节）             |
| TotalDataSize   | 长整型     | 集合空间数据文件总大小（单位：字节）         |
| FreeDataSize    | 长整型     | 集合空间数据文件空闲空间大小（单位：字节）   |
| TotalIndexSize  | 长整型     | 集合空间索引文件总大小（单位：字节）         |
| FreeIndexSize   | 长整型     | 集合空间索引文件空闲空间大小（单位：字节）   |
| TotalLobSize    | 长整型     | 集合空间大对象文件总大小（单位：字节）       |
| FreeLobSize     | 长整型     | 集合空间大对象文件空闲空间大小（单位：字节） |
| Group           | 字符串数组 | 该集合空间所在的分区组名列表                 |

##示例##

```lang-javascript
> db.exec( select * form $SNAPSHOT_CS )
```
