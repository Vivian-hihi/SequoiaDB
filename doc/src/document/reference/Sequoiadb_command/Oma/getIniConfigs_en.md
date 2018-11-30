##NAME##

getIniConfigs - Get the INI file configuration information.

##SYNOPSIS##

**Oma.getIniConfigs(\<configPath\>, [options])**

##CATEGORY##

Oma

##DESCRIPTION##

Get the INI file configuration information.

##PARAMETERS##

* `configPath` ( *String*, *Required* )

   The path of INI file.

* `options` ( *Object*, *Optional* )

   Options for parsing configuration items.

   sensitive: true is data type sensitive, false is the data type as a string, default false.

   delimiter: true is string only supports double quotes, false is String supports double quotes and single quotes, default true.

##RETURN VALUE##

On success, return an object of BSONObj, which contains the configuration information of INI file.

On error, exception will be thrown.

##ERRORS##

the exceptions of `getIniConfigs()` are as below:

| Error Code | Error Type | Description | Solution |
| ------ | --- | ------------ | ----------- |
| -4 | SDB_FNE | File not exist. | Check the configuration file exists or not. |

##HISTORY##

Since v3.0.2.

##EXAMPLES##

1. print the configuration information of INI file.

	```lang-javascript
	> Oma.getIniConfigs( "/opt/config.ini" )
	{
		"datestyle": "iso, ymd",
		"listen_addresses": "*",
		"log_timezone": "PRC",
		"port": "1234",
		"shared_buffers": "128MB",
		"timezone": "PRC"
	}
	```

2. print the configuration information of INI file and data type sensitive.

	```lang-javascript
	> Oma.getIniConfigs( "/opt/config.ini", { "sensitive": true } )
	{
		"datestyle": "iso, ymd",
		"listen_addresses": "*",
		"log_timezone": "PRC",
		"port": 1234,
		"shared_buffers": "128MB",
		"timezone": "PRC"
	}
	```