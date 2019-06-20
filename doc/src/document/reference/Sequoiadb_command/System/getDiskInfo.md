##语法##

***System.getDiskInfo()***

##类别##

System

##描述##

获取磁盘的信息

##参数##

无

##返回值##

返回磁盘的信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取磁盘的信息

  ```lang-javascript
  > System.getDiskInfo()
  {
  "Disks": [
      {
        "Filesystem": "udev",
        "FsType": "devtmpfs",
        "Size": 2963,
        "Used": 0,
        "Unit": "MB",
        "Mount": "/dev",
        "IsLocal": false,
        "ReadSec": 0,
        "WriteSec": 0
      },
      {
        "Filesystem": "tmpfs",
        "FsType": "tmpfs",
        "Size": 596,
        "Used": 60,
        "Unit": "MB",
        "Mount": "/run",
        "IsLocal": false,
        "ReadSec": 0,
        "WriteSec": 0
      },
	  ...
    ]
  }
  ```