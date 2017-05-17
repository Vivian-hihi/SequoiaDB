##名称##

backupOffline - 备份数据库。

##语法##

***db.backupOffline( [options] )***

##类别##

Sdb

##描述##

备份数据库。

##参数描述##

* `options` ( *Object*， *选填* )

   	`options`参数可以设置备份的属性，如指定设定备份名，指定复制组，备份方式等。
	可组合使用 `options` 的如下选项：

    1. `GroupID` ( *Array* )：指定备份的复制组 ID，缺省为所有复制组。

        格式：`GroupID:1000` 或 `GroupID:[1000, 1001]`

    2. `GroupName` ( *String* )：指定备份的复制组名，缺省为所有复制组。

        格式：`GroupName: "data1"` 或 `GroupName: ["data1", "data2"]`

    3. `Name` ( *String* )：备份名称，缺省为 “YYYY-MM-DD-HH:mm:SS” 时间格式的备份名。

		格式：`Name: "backup-2014-1-1"`

    4. `Path` ( *String* )：备份路径，缺省为配置参数指定的备份路径。
                            该路径支持通配符
                        （%g/%G: group name, %h/%H: host name, %s/%S: service name）。

		格式：`Path: "/opt/sequoiadb/backup/%g"`

    5. `IsSubDir` ( *Bool* )：上述 Path 参数所配置的路径是否为配置
                              参数指定的备份路径的子目录，缺省为 false。

		格式：`IsSubDir: false`

    6. `Prefix` ( *String* )：备份前缀名，支持通配符（%g,%G,%h,%H,%s,%S），缺省为空。	

		格式：`Prefix: "%g_bk_"`

    7. `EnableDataDir` ( *Bool* )：是否开启日期子目录功能，如果开启则会自动根据
                                   当前日期创建 “YYYY-MM-DD” 的子目录，缺省为 false。

		格式：`EnableDataDir: false`

    8. `Description` ( *String* )：备份描述。

		格式：`Description: "First backup"`

    9. `EnsureInc` ( *Bool* )：是否开启增量备份，缺省为 false。

		格式：`EnsureInc: false`

    10. `OverWrite` ( *Bool* )：存在同名备份是否覆盖，缺省为 false。

		格式：`OverWrite: false`

##返回值##

成功：返回新集合的对象。  

失败：抛出异常。

##错误##

`backupOffline()`函数常见异常如下：

| 错误码 | 错误类型 | 描述 | 解决方法 |
| ------ | ------ | --- | ------ |
||||

当异常抛出时，可以通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取[错误码](reference/Sequoiadb_error_code.md)，
或通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息。
可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)了解更多内容。

##示例##

1. 对整个数据库进行全量备份。

	```lang-javascript
	> db.backupOffline( { Name: "FullBackup1" } )
	> db.listBackup()
	{
	  	"Name": "FullBackup1",
	  	"NodeName": "susetzb:30000",
  		"GroupName": "SYSCatalogGroup",
  		"EnsureInc": false,
	  	"BeginLSNOffset": 0,
  		"EndLSNOffset": 5299104,
  		"StartTime": "2015-10-20-16:52:42",
  		"HasError": false
	}
	{
  		"Name": "FullBackup1",
  		"NodeName": "susetzb:40000",
  		"GroupName": "db2",
  		"EnsureInc": false,
  		"BeginLSNOffset": 0,
  		"EndLSNOffset": 230209508,
  		"StartTime": "2015-10-20-16:52:42",
  		"HasError": false
	}
	{
  		"Name": "FullBackup1",
  		"NodeName": "susetzb:20000",
  		"GroupName": "db1",
  		"EnsureInc": false,
  		"BeginLSNOffset": 0,
  		"EndLSNOffset": 272453160,
  		"StartTime": "2015-10-20-16:52:42",
  		"HasError": false
	}
	Return 3 row(s).
	```
