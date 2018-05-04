##NAME##

detachNode - Detach a node in the current group.

##SYNOPSIS##

**rg.detachNode( \<host\>, \<service\>, [options] )**

##CATEGORY##

Replica Group

##DESCRIPTION##

Detach a node in the current partition group, but its configuration information will not be deleted. Used with [rg.attachNode()](reference/Sequoiadb_command/SdbReplicaGroup/attachNode_en.md). Currently it is possible to support separation of nodes from a data group or a catalog group.

##PARAMETERS##

* `host` ( *String*， *Required* )

	Hostname or IP address of node. 

* `service` ( *String*， *Required* )

	Service name or port of node. 

* `options` ( *Object*, *Optional* )

    Can be the following options:

    1. `KeepData` ( *Bool* ): Whether to keep the original data of the current node, default to be false.

**Note:**

1. It can not detach when the node is master or when only has a node in current group.
Never open KeepData if the node does not originally belong to the current group.
2. The separated nodes will no longer be managed by the cluster. Please join other groups as soon as possible..

##RETURN VALUE##

On success, return void.

On error, exception will be thrown.

##ERRORS##

the exceptions of `attachNode()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | --- | ------------ | ----------- |
| -15 | SDB_NETWORK | Network error. | 1. Check the state of sdbcm. 2. Check whether hostname or service name is ok or not. |
| -155 | SDB_CLS_NODE_NOT_EXIST | Node does not exist. | Check whether the note exists in current group or not. |
| -204 | SDB_CATA_RM_NODE_FORBIDDEN | Unable to remove the last node or primary in a group. | 1. Check if the node is master. 2. Check if the node is the last node of current group no not. |

When error happen, use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)
to get the error message or use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md)
to get the error code. See [troubleshooting](troubleshooting/general/general_guide.md) for
more detail.

##HISTORY##

* since v1.12

##EXAMPLES##

1. Detach node from group1 and then add it to group2.

	```lang-javascript
	> var rg1 = db.getRG("group1")
	> rg1.detachNode('hostname1', '11830')
	> var rg2 = db.getRG("group2")
	> rg2.attachNode('hostname1', '11830')
	```