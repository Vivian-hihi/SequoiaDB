##描述##

操作系统快照 SNAPSHOT_SYSTEM 列出当前数据库节点所在操作系统中主要的状态与性能监控参数，输出一条记录。

##标示##

SNAPSHOT_SYSTEM

##非协调节点字段信息##

| 字段名               | 类型   |  描述                                                          |
| -------------------- | ------ | -------------------------------------------------------------- |
| NodeName             | 字符串 | 节点名，为“< HostName > : < ServiceName >”                     |
| HostName             | 字符串 | 数据库节点所在物理节点的主机名                                 |
| ServiceName          | 字符串 | svcname 所指定的服务名，与 HostName 共同作为一个逻辑节点的标示 |
| GroupName            | 字符串 | 该逻辑节点所属的分区组名，standalone 模式下，该字段为空字符串  |
| IsPrimary            | 布尔   | 该节点是否为主节点，standalone 模式下，该字段为 false          |
| ServiceStatus        | 布尔   | 是否为可提供服务状态。<br>一些特殊状态，例如 [全量同步](infrastructure/replication/replicate.md) 会使该状态为 false |
| Status               | 字符串 | 数据库状态：包括"Normal"、 "Shutdown"、"Rebuilding"、"FullSync"、"OfflineBackup"状态 |
| BeginLSN.Offset      | 长整型 | 起始 LSN 的偏移                                                |
| BeginLSN.Version     | 整型   | 起始 LSN 的版本号                                              |
| CurrentLSN.Offset    | 长整型 | 当前 LSN 的偏移                                                |
| CurrentLSN.Version   | 整型   | 当前 LSN 的版本号                                              |
| CommittedLSN.Offset  | 长整型 | 已提交 LSN 的偏移                                              |
| CommittedLSN.Version | 整型   | 已提交 LSN 的版本号                                            |
| CompleteLSN          | 长整型 | 已完成 LSN 的偏移                                              |
| LSNQueSize           | 整型   | 等待同步的LSN队列长度                                          |
| TransInfo.TotalCount | 整型   | 正在执行的事务数量                                             |
| TransInfo.BeginLSN   | 长整型 | 正在执行的事务的起始 LSN 的偏移                                |
| NodeID               | 数组   | 节点的 ID，为“[ <分区组 ID>, <节点 ID> ]”<br>在 standalone 模式下，该字段为“[ 0，0 ]” |
| CPU.User             | 浮点数 | 操作系统启动后所消耗的总用户 CPU 时间（单位：秒）              |
| CPU.Sys              | 浮点数 | 操作系统启动后所消耗的总系统 CPU 时间（单位：秒）              |
| CPU.Idle             | 浮点数 | 操作系统启动后所消耗的总空闲 CPU 时间（单位：秒）              |
| CPU.Other            | 浮点数 | 操作系统启动后所消耗的总其它 CPU 时间（单位：秒）              |
| Memory.LoadPercent   | 整型   | 当前操作系统的内存使用百分比（包括文件系统缓存）               |
| Memory.TotalRAM      | 长整型 | 当前操作系统的总内存空间（单位：字节）                         |
| Memory.FreeRAM       | 长整型 | 当前操作系统的空闲内存空间（单位：字节）                       |
| Memory.TotalSwap     | 长整型 | 当前操作系统的总交换空间（单位：字节）                         |
| Memory.FreeSwap      | 长整型 | 当前操作系统的空闲交换空间（单位：字节）                       |
| Memory.TotalVirtual  | 长整型 | 当前操作系统的总虚拟空间（单位：字节）                         |
| Memory.FreeVirtual   | 长整型 | 当前操作系统的空闲虚拟空间（单位：字节）                       |
| Disk.Name            | 字符串 | 数据库路径所在的磁盘名称<br>                                   |
| Disk.DatabasePath    | 字符串 | 数据库路径                                                     |
| Disk.LoadPercent     | 整型   | 数据库路径所在文件系统的空间占用百分比                         |
| Disk.TotalSpace      | 长整型 | 数据库路径总空间（单位：字节）                                 |
| Disk.FreeSpace       | 长整型 | 数据库路径空闲空间（单位：字节）                               |

##协调节点字段信息##

| 字段名              | 类型   | 描述                                              |
| ------------------- | ------ | ------------------------------------------------- |
| CPU.User            | 浮点数 | 操作系统启动后所消耗的总用户 CPU 时间（单位：秒） |
| CPU.Sys             | 浮点数 | 操作系统启动后所消耗的总系统 CPU 时间（单位：秒） |
| CPU.Idle            | 浮点数 | 操作系统启动后所消耗的总空闲 CPU 时间（单位：秒） |
| CPU.Other           | 浮点数 | 操作系统启动后所消耗的总其它 CPU 时间（单位：秒） |
| Memory.TotalRAM     | 长整型 | 当前操作系统的总内存空间（单位：字节）            |
| Memory.FreeRAM      | 长整型 | 当前操作系统的空闲内存空间（单位：字节）          |
| Memory.TotalSwap    | 长整型 | 当前操作系统的总交换空间（单位：字节）            |
| Memory.FreeSwap     | 长整型 | 当前操作系统的空闲交换空间（单位：字节）          |
| Memory.TotalVirtual | 长整型 | 当前操作系统的总虚拟空间（单位：字节）            |
| Memory.FreeVirtual  | 长整型 | 当前操作系统的空闲虚拟空间（单位：字节）          |
| Disk.TotalSpace     | 长整型 | 数据库路径总空间（单位：字节）                    |
| Disk.FreeSpace      | 长整型 | 数据库路径空闲空间（单位：字节）                  |
| ErrNodes.NodeName   | 字符串 | 返回异常节点名（主机名 + 端口）                   |
| ErrNodes.GroupName  | 字符串 | 返回异常节点所属分区组名                          |
| ErrNodes.Flag       | 整型   | 错误码，详细请参见：[错误码](reference/Sequoiadb_error_code.md) |
| ErrNodes.ErrInfo    | 字符串 | 返回节点出错信息                                  |

> Note:
>
> 存在异常节点时才显示ErrNodes字段。

##示例##

```lang-javascript
> db.exec( "select * from $SNAPSHOT_SYSTEM" )
```

