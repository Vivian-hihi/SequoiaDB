CLCount 对象。

##语法##

**CLCount.hint(\<hint\>)**

**CLCount.valueOf()**

**CLCount.toString()**

##方法##

###hint(\<hint\>)###

统计当前集合符合条件的记录总数，可通过 hint 指定查询使用的索引。

###valueOf()###

获取 CLCount 的原始值。

###toString()###

把 CLCount 以字符串的形式输出。

##错误##

如果出错则抛异常，并输出错误信息，可以通过[getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)获取错误信息或通过[getLastError()](reference/Sequoiadb_command/Global/getLastError.md)获取错误码。
关于错误处理可以参考[常见错误处理指南](troubleshooting/general/general_guide.md)。

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 获取 CLCount 对象。

   ```lang-javascript
   > var db = new Sdb( "localhost", 11810 )
   > var clcount = db.foo.bar.find().count()
   ```

* 指定索引统计当前集合符合条件的记录总数。

   ```lang-javascript
   > clcount.hint( { "": "ageIndex" } )
   50004
   ```

* 获取 CLCount 的原始值。

   ```lang-javascript
   > clcount.valueOf()
   50004
   ```

* 把 CLCount 以字符串的形式输出

   ```lang-javascript
   > clcount.toString() 
   50004
   ```

