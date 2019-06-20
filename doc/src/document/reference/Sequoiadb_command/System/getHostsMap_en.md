##NAME##

getHostsMap - Acquire all host to ip address mapping in the host file

##SYNOPSIS##

***System.getHostsMap()***

##CATEGORY##

System

##DESCRIPTION##

Acquire all host to ip address mapping in the host file

##PARAMETERS##

NULL

##RETURN VALUE##

On success, return all host to ip address mapping in the host file.

On error, exception will be thrown.

##ERRORS##

when exception happen, use [getLastError()](reference/Sequoiadb_command/Global/getLastError.md) to get the [error code](reference/Sequoiadb_error_code.md)  and use [getLastErrMsg()](reference/Sequoiadb_command/Global/getLastErrMsg.md) to get [error message](reference/Sequoiadb_command/Global/getLastErrMsg.md). For more detial, please  reference to [Troubleshooting](troubleshooting/general/general_guide.md).

##EXAMPLES##

* Acquire all host to ip address mapping to the host file

	```lang-javascript
    > System.getHostsMap()
    {
      "Hosts": [
        {
          "ip": "127.0.0.1",
          "hostname": "localhost"
        },
        ...
      ]
    } 
	```