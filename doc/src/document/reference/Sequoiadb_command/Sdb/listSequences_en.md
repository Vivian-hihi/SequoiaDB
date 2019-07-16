##NAME##

listSequences - Show the sequences name of the current database.

##SYNOPSIS##

***db.listSequences()***

##CATEGORY##

Sdb

##DESCRIPTION##

Show the sequences name of the current database.

>**Note:**

>db.listSequences() can only show the sequences name. If you want to show detail information of the sequences, please refer to [db.snapshot( SDB_SNAP_SEQUENCES )](database_management/monitoring/snapshot/SDB_SNAP_SEQUENCES.md).

##PARAMETERS##

NULL

##RETURN VALUE##

On success, return the sequences name of the current database.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Show the sequences name of the current database.

```lang-javascript
> db.listSequences()
{
  "Name": "SYS_8589934593_id_SEQ"
}
```
