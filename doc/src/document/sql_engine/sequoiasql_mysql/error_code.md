返回的错误码包括 **MySQL 错误码** 和 **SequoiaDB 错误码**。

##MySQL 错误码##

来自 MySQL 的错误码范围为 1~4000。可以通过 **perror** 工具获取错误码的描述信息。perror 工具位于安装目录的 bin 目录下。如以下例子，在默认的安装配置下，获取 157 错误的描述信息。

```javascript
$ cd /opt/sequoiasql/mysql
$ bin/perror 157
MySQL error code 157: Could not connect to storage engine
```

##SequoiaDB 错误码##

来自 SequoiaDB 的错误码范围为 40000~50000。由于 MySQL 上的错误码需为正数，而 SequoiaDB 上错误码为负数，因此 SequoiaSQL-MySQL 对原 SequoiaDB 的错误码进行了范围的调整。SequoiaSQL-MySQL 上的 SequoiaDB 错误码（记为 ssql_code）与 SequoiaDB 原本的错误码（记为 sdb_code），可根据如下公式转换。

    sdb_code = -(ssql_code - 40000)

如 40006 经转换即是 -(40006 - 40000) = -6。

获取 sdb_code 的错误码描述信息，可以通过在 sdb shell 中执行[getErr(\<error code\>)](reference/Sequoiadb_command/Global/getErr.md)方法，或查阅[错误码列表](reference/Sequoiadb_error_code.md)。
