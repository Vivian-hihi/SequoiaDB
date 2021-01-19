##名称##

listSequences - 枚举序列名称

##语法##

**db.listSequences()**

##类别##

Sdb

##描述##

该函数用于枚举当前数据库的序列名称。

listSequences() 只会列出序列的名称。如果想要查看序列的详细信息，可以使用[db.snapshot(SDB_SNAP_SEQUENCES)][SDB_SNAP_SEQUENCES]。

##参数##

无

##返回值##

函数执行成功时，将返回当前数据库的序列名称。

函数执行失败时，将抛异常并输出错误信息。

##错误##

当异常抛出时，可以通过 [getLastErrMsg()][getLastErrMsg] 获取错误信息或通过 [getLastError()][getLastError] 获取错误码。更多错误处理可以参考[常见错误处理指南][error_guide]。

##版本##

v3.2 及以上版本

##示例##

* 查看当前数据库的序列名称。

  ```lang-javascript
  > db.listSequences()
  {
      "Name": "SYS_8589934593_id_SEQ"
  }
  ```

[^_^]:
     本文使用的所有引用及链接
[SDB_SNAP_SEQUENCES]:manual/Manual/Snapshot/SDB_SNAP_SEQUENCES.md
[getLastErrMsg]:manual/Manual/Sequoiadb_Command/Global/getLastErrMsg.md
[getLastError]:manual/Manual/Sequoiadb_Command/Global/getLastError.md
[error_guide]:manual/faq.md