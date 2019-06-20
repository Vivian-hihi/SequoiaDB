##语法##

***System.getHostsMap()***

##类别##

System

##描述##

host文件的IP与主机的映射关系

##参数##

无

##返回值##

返回host文件的IP与主机的映射关系

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取host文件的IP与主机的映射关系

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