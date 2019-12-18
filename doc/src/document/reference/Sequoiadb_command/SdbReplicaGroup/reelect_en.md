##NAME##

reelect - Re-elect the master node in the replica group.

##SYNOPSIS##

**rg.reelect([options])**

##CATEGORY##

ReplicaGroup

##DESCRIPTION##

Re-elect the master node in the replica group.

##PARAMETERS##

* `options` ( *json object*)

    Parameter collection, can be the following options:

    1. `Seconds` ( *int* ): Re-election start in how many seconds.

    2. `NodeID` ( *int* ): Node ID of the expected primary node.

    3. `HostName` ( *string* ): Host name of the expected primary node.

    4. `ServiceName` ( *string* ): Service name of the expected primary node.

**Note:**

1. Returning timeout error means that the re-election hasn't completed during the time we set. Using db.listReplicaGroups() to view the result.
2. The re-election can only be started when there is a master node in the replica group.
3. When NodeID is used, will ignore HostName and ServiceName.
4. When no specific NodeID or ServiceName is specified, if more than one node can be selected as the primary node, the matching rule of the election is : LSN of the node > node weight > NodeID. The node with the largest LSN is selected. If LSN is consistent, the node with the largest weight value is selected, if the weight value is consistent, the node with the largest ID value is selected.
5. The surviving nodes in a replication group need to account for at least half of the total number of nodes

##RETURN VALUE##

There is no return value. On error, exception will be thrown.

##ERRORS##

The exceptions of `reelect()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | --- | ------------ | ----------- |
| -104 | SDB_CLS_NOT_PRIMARY | Primary node does not exit | Check if the current replicaGroup has a node with "isPrimary" being "true". Start the node if there is a node that is not started in the current replicaGroup. |


When error happen, use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)
to get the error message or use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md)
to get the error code. See [troubleshooting](troubleshooting/general/general_guide.md) for
more detail.

##EXAMPLES##

1. Conduct the re-election in 60s with the group of 'datagroup1'

	```lang-javascript
	> var rg = db.getRG("datagroup1") 
	> rg.reelect({Seconds:60})
	```

