##NAME##

find - Find a file.

##SYNOPSIS##

***File.find( \<options\>, \[filter\] )***

##CATEGORY##

File

##DESCRIPTION##

Find a file.

##PARAMETERS##

| Name    | Type     | Description                | Required or not |
| ------- | -------- | -------------------------- | --------------- |
| options | JSON     | find mode and find content | yes             |
| filter  | JSON     | Filtered conditions        | not             |

The detail description of 'options' parameter is as follow:

| Attributes | Type | Required or not | Format | Description |
| ---------- | ---- | --------------- | ------ | ----------- |
| mode       | char | yes             | { mode: 'n' }<br>{ mode: 'u' }<br>{ mode: 'g' }<br>{ mode: 'p' } | find files based on file name<br>find files based on username<br>find find based on group name<br>find files based on permission | 
| pathname   | char | not             | { pathname: "pathname" } | specify the file path to find |
| value      | char | yes             | { value: "content" }     | find content |

The optional parameter filterObj supports the AND, the OR, the NOT and exact matching of some fields in the result, and the result set is filtered.

##RETURN VALUE##

On success, return rearch result set.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Find a file;

```lang-javascript
> File.find( { mode: "n", value: "tmp" } )
{
  "pathname": "./tmp"
}
{
  "pathname": "./database/50000/tmp"
}
{
  "pathname": "./.svn/tmp"
}
{
  "pathname": "./bin/tmp"
}
```

* Find a file and filter the result set

```lang-javascript
> File.find( { mode: "n", value: "tmp" }, { $or: [ { pathname: "./bin/tmp" }, { pathname: "./database/41000/tmp" } ] } )
{
  "pathname": "./database/41000/tmp"
}
{
  "pathname": "./bin/tmp"
}
 ```