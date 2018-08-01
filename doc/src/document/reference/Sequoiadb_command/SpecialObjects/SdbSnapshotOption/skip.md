##语法##
***option.skip( [num] )***

指定结果集从哪条记录开始返回。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | -------- | ---- | -------- |
| num | int | 自定义结果集从哪条记录返回。 | 否 |

> **Note：**  
> option.skip() 方法的定义格式包含 num 参数，它是 int 类型。如果不设定 num 的内容或者设定 num 的值为0，相当于返回所有的结果集；如果想从结果集的第3条记录开始返回，可是设置 num 的值等于2。

##返回值##

返回 option 自身，类型为 SdbSnapshotOption。

##错误##

[错误码](reference/Sequoiadb_error_code.md)

