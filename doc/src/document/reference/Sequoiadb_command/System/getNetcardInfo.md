##语法##

***System.getNetcardInfo()***

##类别##

System

##描述##

获取网卡的信息

##参数##

无

##返回值##

返回网卡的信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取网卡的信息

  ```lang-javascript
  > System.getNetcardInfo()
  {
    "Netcards": [
      {
        "Name": "lo",
        "Ip": "127.0.0.1"
      },
      {
        "Name": "ens160",
        "Ip": "192.168.20.62"
      }
    ]
  }
  ```