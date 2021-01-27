##NAME##

forceStepUp - force the standby node to be upgraded to the primary node 

##SYNOPSIS##

**db.forceStepUp([options])**

##CATEGORY##

Sdb

##DESCRIPTION##

This function is used to forcibly promote the standby node to the primary node in a replication group that is not eligible for election. If the LSN of the upgraded primary node is smaller than the LSN of the original primary node, the previously successful operation will be rolled back, so use this command with caution.

>**Note:**
>
> Currently, only forced promotion in a catalog replication group is supported.

##PARAMETERS##

| Name | Type| Description | Required or not |
| ---- | --- | ----------- | --------------- |
| options |object | parameter set | not |

options 选项：

| Name | Type| Description | Defaults |
| ---- | --- | ----------- | --------------- |
|Seconds   |number      | Duration of forced promotion to master. |120|

> **Note:**
>
> * The primary node cannot exist in the target replication group, and the LSN of other nodes cannot be greater than the LSN of the target node. To obtain the node LSN information, please refer to [Node Health Detection Snapshot][SDB_SNAP_HEALTH].
> * When the duration expires, all nodes will re-elect according to the election rules.
> * If a user is created, it is not possible to connect directly to the catalog node. Users can modify the auth parameter in the configuration file of the catalog node first, configure auth=false to disable the authentication function of the catalog, and restart the cluster before proceeding.

##RETURN VALUE##

When the function executes successfully, there is no return value.

When the function fails, an exception will be thrown and an error message will be printed.

##ERRORS##

When the exception happens, use [getLastErrMsg()][getLastErrMsg] to get the error message or use [getLastError()][getLastError] to get the [error code][error_code]. For more details, refer to [Troubleshooting][faq].

##VERSION##

v3.4 and above

##EXAMPLES##

Connect to the catalog node (hostname1:30000) and force it to be promoted to the master for 300s.

```lang-javascript
> var db = new Sdb("hostname1", 30000)
> db.forceStepUp({Seconds: 300})
```

[^_^]:
     Links
[getLastErrMsg]:manual/Manual/Sequoiadb_Command/Global/getLastErrMsg.md
[getLastError]:manual/Manual/Sequoiadb_Command/Global/getLastError.md
[faq]:manual/FAQ/faq_sdb.md
[error_code]:manual/Manual/Sequoiadb_error_code.md