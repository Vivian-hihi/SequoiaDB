##语法##

***System.getCpuInfo()***

##类别##

System

##描述##

获取CPU的信息

##参数##

无

##返回值##

返回CPU的信息

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取CPU的信息

	```lang-javascript
    > System.getCpuInfo()
    {
      "Cpus": [
        {
        "Core": 1,
        "Info": "Intel(R) Xeon(R) CPU E5-2650 v4 @ 2.20GHz",
        "Freq": "2.19999814GHz"
        },
        {
        "Core": 1,
        "Info": "Intel(R) Xeon(R) CPU E5-2650 v4 @ 2.20GHz",
        "Freq": "2.19999814GHz"
        }
      ],
      "User": 47223380,
      "Sys": 46662920,
      "Idle": 3513293040,
      "Other": 3023840
    }
	```