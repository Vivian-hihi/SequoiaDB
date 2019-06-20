##语法##

***System.getProcUlimitConfigs()***

##类别##

System

##描述##

获取进程资源限制值

##参数##

无

##返回值##

返回获取进程资源限制值

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取进程资源限制值

  ```lang-javascript
  > System.getProcUlimitConfigs()
  {
    "core_file_size": 0,
    "data_seg_size": -1,
    "scheduling_priority": 0,
    "file_size": -1,
    "pending_signals": 23711,
    "max_locked_memory": 65536,
    "max_memory_size": -1,
    "open_files": 1024,
    "POSIX_message_queues": 819200,
    "realtime_priority": 0,
    "stack_size": 8388608,
    "cpu_time": -1,
    "max_user_processes": 23711,
    "virtual_memory": -1,
    "file_locks": -1
  }
  ```