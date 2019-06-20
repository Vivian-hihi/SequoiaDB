##语法##

***System.runService( \<servicename\>, \<command\>, \[option\] )***

##类别##

System

##描述##

运行service命令

##参数##

| 参数名  | 参数类型 | 默认值       | 描述             | 是否必填 |
| ------- | -------- | ------------ | ---------------- | -------- |
| servicename   | string   | --- | 服务名       | 是       |
| command     | string   | ---   | 命令     | 是       |
| option | boolean  | ---    | 选项 | 否       |

##返回值##

返回运行service命令的信息

##错误##

常见错误可参考[错误码](reference/Sequoiadb_error_code.md)。

##示例##

* 查看ssh服务信息

  ```lang-javascript
  > System.runService( "ssh", "status", "")
  ● ssh.service - OpenBSD Secure Shell server
     Loaded: loaded (/lib/systemd/system/ssh.service; enabled; vendor preset: enabled)
       Active: active (running) since 三 2019-05-29 10:29:47 CST; 6 days ago
   Main PID: 1637 (sshd)
      Tasks: 1
     Memory: 5.6M
        CPU: 1.268s
     CGroup: /system.slice/ssh.service
             └─1637 /usr/sbin/sshd -D
  
  6月 04 14:57:06 hostname sshd[24292]: pam_unix(sshd:session): session opened for user sdbadmin by (uid=0)
  6月 04 15:12:22 hostname sshd[17900]: pam_unix(sshd:auth): authentication failure; logname= uid=0 euid=0 tty=ssh ruser= rhost=192.168.10.124  user=username
  ```