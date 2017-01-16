## 语法##
***db.listBackup([options], [cond], [sel])***

查看数据库备份

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 设定备份名，指定复制组，路径等参数 | 否 |
| cond | Json 对象 | 备份过滤条件 | 否 |
| sel | Json 对象 | 查看备份选择输出的字段 | 否 |

### Options 格式###

| 参数名 | 描述 | 格式 |
| ------ | ------ | ------ |
| GroupID | 指定备份的复制组 ID，缺省为所有复制组 | GroupID:1000 或 GroupID:[1000, 1001] |
| GroupName | 指定备份的复制组名，缺省为所有复制组 | GroupName:"data1" 或 GroupName:["data1", "data2"] |
| Name | 备份名称，缺省查看所有备份 | Name:"backup-2014-1-1" |
| Path | 备份路径，缺省为配置参数指定的备份路径。该路径支持通配符（%g/%G: group name, %h/%H: host name, %s/%S:service name） | Path:"/opt/sequoiadb/backup/%g" |
| IsSubDir | 上述 Path 参数所配置的路径是否为配置参数指定的备份路径的子目录，缺省为 false IsSubDir:false | Prefix 备份前缀名，支持通配符（%g,%G,%h,%H,%s,%S），缺省为空 Prefix:"%g_bk_" |

## 示例##

* 查看数据库所有备份信息

<pre class="prettyprint lang-javascript">
> db.listBackup()</pre>
