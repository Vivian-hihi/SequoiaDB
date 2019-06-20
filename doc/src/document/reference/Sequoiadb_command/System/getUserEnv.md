##语法##

***System.getUserEnv()***

##类别##

System

##描述##

获取环境变量

##参数##

无

##返回值##

返回环境变量

##错误##

[错误码](reference/Sequoiadb_error_code.md)

##示例##

* 获取环境变量

	```lang-javascript
  > System.getUserEnv()
  {
    "MAIL": "/var/mail/name",
    "SSH_CLIENT": "192.168.10.124 49706 22",
    "USER": "name",
    "LANGUAGE": "zh_CN:zh",
    "SHLVL": "1",
    "HOME": "/home/users/name",
    "OLDPWD": "/opt/trunk/doc/config",
    "SSH_TTY": "/dev/pts/8",
    "LOGNAME": "name",
    "_": "bin/sdb",
    "XDG_SESSION_ID": "1518",
    "TERM": "xterm",
    "PATH": "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin",
    "XDG_RUNTIME_DIR": "/run/user/2160",
    "LANG": "zh_CN.UTF-8",
    "SHELL": "/bin/bash",
    "PWD": "/opt/trunk",
    "XDG_DATA_DIRS": "/usr/local/share:/usr/share:/var/lib/snapd/desktop",
    "SSH_CONNECTION": "192.168.10.124 49706 192.168.20.62 22"
  }
	```