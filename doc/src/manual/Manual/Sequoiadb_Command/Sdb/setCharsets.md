##名称##

setCharsets - 设置客户端及结果集的字符集

##语法##

**db.setCharsets(\<charset\>)**

##类别##

Sdb

##描述##

该函数用于设置客户端及结果集的字符集

##参数##

charset （ *string，必填* ）

通过 charset 指定用户需要设置的字符集


##返回值##

函数执行成功时，无返回值。

函数执行失败时，将抛异常并输出错误信息。

##错误##

常见异常如下：

| 错误码 | 错误类型 | 描述 | 解决方法 |
| ------ | ------ | --- | ------ |
| -6 | SDB_INVALIDARG | 无效参数 | |

当异常抛出时，可以通过 [getLastErrMsg()][getLastErrMsg] 获取错误信息或通过 [getLastError()][getLastError] 获取[错误码][error_code]。更多错误处理可以参考[常见错误处理指南][faq]。

##版本##

v5.10 及以上版本

##示例##
设置客户端及结果集的字符集为 GB18030

```lang-javascript
> db.setCharsets("GB18030")
```

[^_^]:
    本文使用的所有引用及链接
[getLastErrMsg]:manual/Manual/Sequoiadb_Command/Global/getLastErrMsg.md
[getLastError]:manual/Manual/Sequoiadb_Command/Global/getLastError.md
[faq]:manual/FAQ/faq_sdb.md
[error_code]:manual/Manual/Sequoiadb_error_code.md