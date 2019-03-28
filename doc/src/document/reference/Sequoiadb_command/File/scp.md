##语法##

***File.scp( \<srcFile\>, \<dstFile\>, \[isreplace\]， \[mode\])***

##类别##

File

##描述##

远程拷贝文件。

##参数##

| 参数名    | 参数类型 | 默认值 | 描述             | 是否必填 |
| --------- | -------- | ------ | ---------------- | -------- |
| srcFile   | string   | ---    | 源文件路径       | 是       |
| desFile   | string   | ---    | 目标文件路径     | 是       |
| isreplace | boolean  | false  | 是否替换目标文件 | 否       |
| mode      | int      | 0644   | 设置文件权限     | 否       |

> Note : 

> 参数 srcFile 和 desFile 具体格式为 “ip:sdbcmPort@filepath”，例如  “192.168.20.71:11790@/opt/trunk/test/test_one”。

##返回值##

无返回值。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 远程拷贝文件。

  ```lang-javacript
  > File.scp( "192.168.20.71:11790@/opt/trunk/test/test_one", "192.168.20.71:11790@/opt/trunk/test/test_four" )
  Success to copy file from 192.168.20.71:11790@/opt/trunk/test/test_one to 192.168.20.71:11790@/opt/trunk/test/test_four
  ```