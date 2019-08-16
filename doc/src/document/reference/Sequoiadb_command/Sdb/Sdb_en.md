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

* `svcname` ( *int*， *Required* )

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

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##HISTORY##

since v1.12

##EXAMPLES##

1. Create a Sdb object connection to the SequoiaDB, hostname default to be "localhost"，svcname default to be 11810.

	```lang-javascript
 	> var db = new Sdb()
 	```

2. Create a Sdb object  connection to the SequoiaDB on the specified host, target hostname is "sdbserver1".

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810)
	```

3. Create a Sdb object  connection to the SequoiaDB on the specified host using the default username and password. default username and password are empty.

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"","")
	```

4. Create a Sdb object  connection to the SequoiaDB on the specified host with a username and password.

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"admin","123")
	```