##语法##
***option.options( \<options\> )***

控制查询返回的记录条数。

##参数描述##

| 参数名 |  参数类型  | 描述 | 是否必填 |
| ------ | ---------  | ---- | -------- |
| options|	Json 对象 | 指定快照选项，因不同快照类型而异，在对应[快照类型](database_management/monitoring/snapshot/snapshot.md)查看选项及示例。  | 否 |

> **Note：**  
> 目前拥有快照选项的有[配置快照](database_management/monitoring/snapshot/SDB_SNAP_CONFIGS.md)。

##返回值##

返回 option 自身，类型为 SdbSnapshotOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)
