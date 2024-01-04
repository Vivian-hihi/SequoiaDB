##NAME##

setCharsets - Set character set for client and result set

##SYNOPSIS##

**db.setCharsets(\<charset\>)**

## CATEGORY ##

Sdb

## DESCRIPTION ##

This function is used to set character set for client and result set.

## PARAMETERS ##

charset (*string, required*)

Specifies the character set name of the client and result set. The optional value of this parameter is UTF8 or GB18030, which is not case sensitive.

## RETURN VALUE ##

There is no return value when the function is executed successfully.

When the function execution fails, an exception will be thrown with an error message.

## ERRORS ##

Common exceptions are as follows:

| Error Code | Error Type | Description | Solution |
| ------ | ------ | --- | ------ |
| -6 | SDB_INVALIDARG | Invalid arguments | |

When an exception is thrown, you can retrieve the error message through [getLastErrMsg()][getLastErrMsg] or [getLastError()][getLastError] to get the [error code][error_code]. For more error handling, you can refer to the [Common Error Handling Guide][faq].

## VERSION ##

v5.10 and above

## EXAMPLE ##

Set character set for the client and result set to GB18030.

```lang-javascript
> db.setCharsets("GB18030")
```

[^_^]:
    All references and links used in this document:
[getLastErrMsg]:manual/Manual/Sequoiadb_Command/Global/getLastErrMsg.md
[getLastError]:manual/Manual/Sequoiadb_Command/Global/getLastError.md
[faq]:manual/FAQ/faq_sdb.md
[error_code]:manual/Manual/Sequoiadb_error_code.md