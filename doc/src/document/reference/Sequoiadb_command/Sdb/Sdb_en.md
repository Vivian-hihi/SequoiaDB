##NAME##

Sdb - connection SequoiaDB object.

##SYNOPSIS##
**var db = new Sdb([hostname],[svcname])**

**var db = new Sdb([hostname],[svcname],[username],[password])**

##CATEGORY##

Sdb

##DESCRIPTION##

Create a Sdb object to connect to the SequoiaDB.

##PARAMETERS##

* `hostname` ( *String*， *Required* )

	Target hostname, default to be "localhost".

* `svcname` ( *int*, *Required* )

	Target svcname, default to be 11810.

* `username` ( *String*)

   The username of the SequoiaDB user, if the SequoiaDB has no users. default to be empty.

* `password` ( *String* )

   The password name of the SequoiaDB user, if the SequoiaDB has no users. default to be empty.


>**Note:**

>1. Use [createUsr()](reference/Sequoiadb_command/Sdb/createUsr.md) to create a user and set the password.

>2. If the SequoiaDB has users, then must use username and password to create the Sdb object.


##RETURN VALUE##

On success, return an object of Sdb.

On error, exception will be thrown.

##ERRORS##

the exceptions of `Sdb()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | --- | ------------ | ----------- |
| -15 | SDB_NETWORK | Network error. | Check the hostname and the port fo sdbcm is reachable. |
| -79 | SDB_NET_CANNOT_CONNECT| Unable to connect to the address | Check that the address, port and node configuration are correct. |
| -104 | SDB_CLS_NOT_PRIMARY| Primary node does not exit | Checks if there is a true node of "isPrimary" in current replicaGroup. Start the node if there is a node that is not started in the current replicaGroup. |
| -250 | SDB_CLS_NODE_BSFAULT | The node is not in normal status | ChecKs the node statusf of the current replicaGroup. like chesks that  check that catalog node is started. |


when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##HISTORY##

since v1.12

##EXAMPLES##

1. Create a Sdb object connection to the SequoiaDB, hostname default to be "localhost", svcname default to be 11810.

	```lang-javascript
 	> var db = new Sdb()
 	```

2. Create a Sdb object  connection to the SequoiaDB on the specified host, target hostname is "sdbserver1".

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810)
	```

3. Create a Sdb object  connection to the SequoiaDB on the specified host using the default username and password, default username and password are empty.

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"","")
	```

4. Create a Sdb object  connection to the SequoiaDB on the specified host with a username and password.

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"admin","123")
	```