##语法##

***System.snapshotNetcardInfo()***

##类别##

System

##描述##

获取网卡的详细信息

##参数##

无

##返回值##

返回网卡的详细信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取网卡的详细信息

  ```lang-javascript
  > System.snapshotNetcardInfo()
  {
    "CalendarTime": 1559722067,
    "Netcards": [
      {
        "Name": "lo",
        "RXBytes": 108885345140,
        "RXPackets": 97058303,
        "RXErrors": 0,
        "RXDrops": 0,
        "TXBytes": 108885345140,
        "TXPackets": 97058303,
        "TXErrors": 0,
        "TXDrops": 0
      },
      {
        "Name": "ens160",
        "RXBytes": 8267964446,
        "RXPackets": 6629177,
        "RXErrors": 0,
        "RXDrops": 141152,
        "TXBytes": 1864089945,
        "TXPackets": 2306206,
        "TXErrors": 0,
        "TXDrops": 0
      }
    ]
  }
  ```