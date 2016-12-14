##语法##
***db.removeBackup([options])***

删除数据库备份

## 参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 设定备份名，指定复制组，备份路径等参数 | 否 |

### options 格式###

| 属性名 | 描述 | 格式 |
| ------ | ------ | ------ |
| GroupID | 指定备份的复制组 ID，缺省为所有复制组 | GroupID:1000 或 GroupID:[1000, 1001] |
| GroupName | 指定备份的复制组名，缺省为所有复制组 | GroupName:"data1" 或 GroupName:["data1", "data2"] |
| Name | 备份名称，缺省删除所有备份 | Name:"backup-2014-1-1" |
| Path | 备份路径，缺省为配置参数指定的备份路径。该路径支持通配符（%g/%G: group name, %h/%H: host name, %s/%S:service name） | Path:"/opt/sequoiadb/backup/%g" |
| IsSubDir | 上述 Path 参数所配置的路径是否为配置参数指定的备份路径的子目录，缺省为 false | IsSubDir:false |
| Prefix | 备份前缀名，支持通配符（%g,%G,%h,%H,%s,%S），缺省为空 | Prefix:"%g_bk_" |

## 示例##

* 删除数据库中备份名为“backup-2014-1-1”的备份信息

<pre class="prettyprint lang-javascript">
> db.removeBackup({Name:"backup-2014-1-1"})</pre>
