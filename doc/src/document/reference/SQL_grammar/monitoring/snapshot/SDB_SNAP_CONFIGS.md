##描述##

配置快照 SNAPSHOT_CONFIGS 列出数据库中指定节点的配置信息。

每一个节点上的配置信息为一条记录。

##标示##

SNAPSHOT_CONFIGS

###字段信息###

字段信息详见[数据库配置](database_management/runtime_configuration.md)一节。

##快照参数##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
| Mode   |	String  | 指定返回配置的模式。在 run 模式下，显示当前运行时配置信息，在 local 模式下，显示配置文件中配置信息。如 { "Mode": "local" }。默认为 run。 | 否 |
| Expand |	Bool/String  | 是否扩展显示用户未配置的配置项。如 { "Expand": false }。默认为 true。| 否 |

> **Note:**

>在查询快照时指定快照参数，请参考 [SdbSnapshotOption](reference/Sequoiadb_command/AuxiliaryObjects/SdbSnapshotOption.md)。

##示例##

查看数据组 db1 中数据节点 20000 上的配置信息

```lang-javascript
> db.exec( "select * from $SNAPSHOT_CONFIGS where GroupName = 'db1' and SvcName = '20000'" )
```
