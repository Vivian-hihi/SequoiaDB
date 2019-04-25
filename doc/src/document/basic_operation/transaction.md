事务是由一系列操作组成的逻辑工作单元。在同一个会话（或连接）中，同一时刻只允许存在一个事务，也就是说当用户在一次会话中创建了一个事务，在这个事务结束前用户不能再创建新的事务。

事务作为一个完整的工作单元执行，事务中的操作要么全部执行成功要么全部执行失败。SequoiaDB事务中的操作只能是插入数据、修改数据以及删除数据，在事务过程中执行的其它操作不会纳入事务范畴，也就是说事务回滚时非事务操作不会被执行回滚。如果一个表或表空间中有数据涉及事务操作，则该表或表空间不允许被删除。

## 启停事务 ##

[数据库配置](database_management/runtime_configuration.md)中，关于事务启停的配置项如下：

| 配置项 | 描述 | 取值 | 默认值 |
| ------ | ------ | --- | ------ |
| transactionon | 表示 SequoiaDB 是否开启事务功能。 | true/false | true |

默认情况下，SequoiaDB 所有节点的事务功能都是开启的。若用户不需要使用事务功能，可参考示例 3 **全局关闭事务** 的方法，关闭事务功能。

注意：

1. 开启及关闭节点的事务功能都要求重启该节点。
2. 在开启节点事务功能的情况下，节点的配置项[logfilenum](database_management/runtime_configuration.md)（该配置项默认值为20）的值不能小于 5。

## 隔离级别 ##

[数据库配置](database_management/runtime_configuration.md)中，关于事务隔离级别的配置项如下：

| 配置项 | 描述 | 取值 | 默认值 |
| ------ | ------ | --- | ------ |
| transisolation | 表示在开启事务的情况下，使用的事务隔离级别。 | 0 表示 RU，1 表示 RC，2 表示 RS | 0 |

## 提交与回滚 ##

SequoiaDB 事务支持 **手工提交与回滚** 以及 **自动提交与回滚**。

### 手工提交与回滚 ###

对于 **手工提交与回滚**，事务中所有的操作将被认为是一个整体，这个整体要么全部被提交，要么全部被回滚。 **手工提交与回滚**需要显式调用 "transBegin" 和 "transCommit" 或者 "transRollback" 方法，其使用方式如：

```lang-javascript
> db.transBegin()
> 事务操作1
> 事务操作2
> ...
> ...
> db.transCommit() or db.transRollback()
```

### 自动提交与回滚 ###

在集群开启事务的情况下，通过设置[数据库配置](database_management/runtime_configuration.md)的 "transautocommit" 及 "transautorollback" 配置项，可以让单个操作（如：批量插入）在不显式调用 "transBegin" 和 "transCommit" 或者 "transRollback" 方法的情况下，也具有事务的能力。

关于事务自动提交与回滚的配置项如下：

| 配置项 | 描述 | 取值 | 默认值 |
| ------ | ------ | --- | ------ |
| transautocommit | 表示是否开启事务自动提交与回滚功能。 | true/false | false |
| transautorollback | 表示是否开启事务自动回滚功能。 | true/false | true |

transautorollback 的默认值为 true，表示当单个操作失败时，该操作将自动回滚。
例如：当 transautorollback 设置为 true，批量插入失败时，该批次的所有记录将全部被回滚。当 transautorollback 设置为 false, 批量插入失败时，批次中部分成功插入的记录将没有被回滚。  

transautocommit 的默认值为 false。当 transautocommit 被设置为 true 时，系统将忽略 transautorollback 设置的值。在 transautocommit 设置为 true的情况下，单个操作成功时，该操作将自动被提交；失败时，该操作将自动被回滚。

## 其它配置 ##

[数据库配置](database_management/runtime_configuration.md)中，关于事务的其它主要配置项如下：

| 配置项 | 描述 | 取值 | 默认值 |
| ------ | ------ | --- | ------ |
| transactiontimeout | 事务锁等待超时时间（单位：秒） | [0, 3600] | 60 |
| translockwait | 事务在RC隔离级别下是否需要等锁。 | true/false | false |
| transuserbs | 事务操作是否使用回滚段。 | true/false | true |

## 调整设置 ##

当用户希望调整事务的设置时（如：是否开启事务、调整事务配置项等），有如下 3 种方式供用户调整事务设置。

1. 修改节点配置文件

	用户可以将[数据库配置](database_management/runtime_configuration.md)描述的事务配置项，配置到集群所有（或者部分）节点的配置文件中。若修改的配置项要求重启节点才能生效，用户需重启相应的节点。
2. 使用 [updateConf()](reference/Sequoiadb_command/Sdb/updateConf.md) 命令在 sdb shell 中修改集群的事务配置项。若修改的配置项要求重启节点才能生效，用户需重启相应的节点。
3. 使用 [setSessionAttr()](reference/Sequoiadb_command/Sdb/setSessionAttr.md) 命令在会话中修改当前会话的事务配置项。该设置只在当前会话生效，并不影响其它会话的设置情况。


## 示例 ##

假设集群的安装目录为 "/opt/sequoiadb"，协调节点地址为 "ubuntu-dev1:11810"。通过如下操作，获取 db 以及 cl 对象。

	```lang-javascript
	> db = new Sdb( "ubuntu-dev1", 11810 )
    > cl = db.createCS("foo").createCL("bar")
	```

1. 使用事务回滚插入操作。事务回滚后，插入的记录将被回滚，集合中无记录：

	```lang-javascript
	> cl.count()
	Return 0 row(s). 
	> db.transBegin()
    > cl.insert( { date: 99, id: 8, a: 0 } )
	> db.transRollback()
	> cl.count()
	Return 0 row(s). 
	```

2. 使用事务提交插入操作。提交事务后，插入的记录将被持久化到数据库：

	```lang-javascript
	> cl.count()
	Return 0 row(s). 
	> db.transBegin()
    > cl.insert( { date: 99, id: 8, a: 0 } )
	> db.transCommit()
	> cl.count()
	Return 1 row(s). 
	```

3. 全局关闭事务。

	步骤1：通过sdb shell 设置集群所有节点都关闭事务。

	```lang-javascript
	> db.updateConf( { transactionon: false }, { Global: true } )
	```

	步骤2：在集群每台服务器上都重启 SequoiaDB 的所有节点。

	```lang-javascript
	[sdbadmin@ubuntu-dev1 ~]$ /opt/sequoiadb/bin/sdbstop -t all
	[sdbadmin@ubuntu-dev1 ~]$ /opt/sequoiadb/bin/sdbstart -t all
	```
	注意：必须在每台服务器上都重启 SequoiaDB 的所有节点，才能保证事务功能在所有节点上都关闭的。