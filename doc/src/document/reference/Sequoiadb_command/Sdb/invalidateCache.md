##语法##
***db.invalidateCache( [options] )***

清除节点（数据节点/协调节点）的缓存。

##参数描述##

| 参数名 | 参数类型 | 描述 | 是否必填 |
| ------ | ------ | ------ | ------ |
| options | Json 对象 | 清除缓存的选项 | 否 |

 1. **options 格式**

 目前通过options可设置的属性有：

 | 属性名 | 描述 | 格式 |
 | ------ | -------| ----- |
 | Groups | 需要清除缓存的目标。| Groups: null -- 当前协调节点；<br>Groups: ['group1','group2'] -- 当前协调节点和指定的两个数据组；<br>Groups: 'group1' -- 当前协调节点和指定的一个数据组。|

> **Note:**
>
> 当不指定 Groups 时，作用域为当前协调节点和所有数据节点。

##返回值##

无返回值，出错抛异常，并输出错误信息，可以通过 [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) 获取错误信息 或 通过 [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) 获取错误码。关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md) 。

##示例##

* 清除当前协调节点和数据组‘group1’的缓存信息。

 ```lang-javascript
 > db.invalidateCache( { Groups: 'group1' } )
 ```
