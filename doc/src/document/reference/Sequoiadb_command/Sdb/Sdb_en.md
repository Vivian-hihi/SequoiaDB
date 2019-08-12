##NAME##

Sdb - connection sequoiadb

##SYNOPSIS##
**var db = new Sdb([hostname],[svcname])**

**var db = new Sdb([hostname],[svcname],[username],[password])**

##CATEGORY##

Sdb

##DESCRIPTION##

Create a Sdb object to connect to the sequoiadb.

##PARAMETERS##

* `hostname` ( *String*， *Required* )

	Target hostname, default to be "localhost".

* `svcname` ( *int*， *Required* )

	Target svcname, default to be 11810.

* `username` ( *String*)

   If a user name is set when the sequoiadb is installed, it is required, default to be empty.

* `password` ( *String* )

   If a password is set when the sequoiadb is installed, it is required, default to be empty.

##RETURN VALUE##

On success, return an object of Sdb.

On error, exception will be thrown.

##ERRORS##

the exceptions of `Sdb()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | --- | ------------ | ----------- |
| -79 | SDB_NET_CANNOT_CONNECT | Unable to connect to the specified address  | Check the correct address, port, and node configuration information are reachable |

When error happen, use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md)
to get the error message or use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md)
to get the error code. See [troubleshooting](troubleshooting/general/general_guide.md) for
more detail.

##HISTORY##

since v1.12

##EXAMPLES##







1. Create a Sdb object connection to the default sequoiadb, hostname default to be "localhost"，svcname default to be 11810.

	```lang-javascript
 	> var db = new Sdb()
 	```

2. Create a Sdb object  connection to the sequoiadb on the specified host, target hostname is "sdbserver1".

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810)
	```

3. Create a Sdb object  connection to the sequoiadb on the specified host using the default username and password. default username and password are empty.

	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"","")
	```

4. Create a Sdb object  connection to the sequoiadb on the specified host with a username and password.



	```lang-javascript
 	> var db = new Sdb("sdbserver1",11810,"sdbadmin","123")
	```