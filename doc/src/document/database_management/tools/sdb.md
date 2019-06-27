sdb 是一个 SequoiaDB 数据库的接口工具。

##选项##

| 参数        | 参数 | 描述                                              |
| ----------- | ---- | ------------------------------------------------- |
| --help      | -h   | 返回帮助信息                                |
| --version   | -v   | 返回当前数据库版本            |
| --language  | -l   | 指定显示语言，可以为 “en” 或者 “cn”，默认为 “en”      |
| --file      | -f   | 指定要在 sdb 中执行的文件              |
| --eval      | -e   | 提前声明变量，与 -f 选项一起用          |
| --shell     | -s   | 指定要在 sdb 中执行的语句          |

##快捷键##

进入 sdb 是一个 shell 界面，与其他 shell 类似，sdb shell 也支持一些常用的快捷键：

| 快捷键       | 描述                               |
| ----------- |  ------------------------------------------------- |
| ctrl + a    |  光标移动到行首   |
| ctrl + b  |  光标向左移动一个字符，相当于左方向键    |
| ctrl + c  |  取消操作或退出 sdb   |
| ctrl + d  |  删除当前光标所在位置的字符，相当于 delete 键     |
| ctrl + e  |  移动到行尾   |
| ctrl + t  |  光标向右移动一个字符，相当于右方向键    |
| ctrl + g  |  退出历史命令反向查询  |
| ctrl + h  |  删除当前光标左边一位的字符，相当于 backspace 键   |
| ctrl + k  |  删除当前光标位置到行尾的字符  |
| ctrl + l  |  清屏   |
| ctrl + m  |  相当于 enter 键      |
| ctrl + n  |  相当于下方向键      |
| ctrl + p  |  相当于上方向键          |
| ctrl + r  |  历史命令反向查询      |
| ctrl + u  |  删除一整行       |
| ctrl + w  |  删除当前光标位置到上一个单词之间的字符   |
| ctrl + <-  |  光标移动到前一个单词的开头          |
| ctrl + ->  |  光标移动到后一个单词的末尾         |

##用法##

1.  返回帮助信息

    ```lang-bash
    $ sdb -h
    Command options:
    -h [ --help ]         help
    -v [ --version ]      version
    -l [ --language ] arg specified the display language, can be "en" or "cn", 
                          default to be "en"
    -f [ --file ] arg     if the -f option is present, then commands are read 
                          from the file specified by <string>
    -e [ --eval ] arg     predefined variables(format: "var varname='varvalue'")
    -s [ --shell ] arg    if the -s option is present, then commands are read 
                          from <string>
    ```

2.  提前声明变量，并指定文件执行

    ```lang-bash
    $ cat test.js 
    var db = new Sdb(hostname, "50000")
    db.list(SDB_LIST_CONTEXTS, { GroupName: "db1" } )
    $ sdb -e "hostname = 'localhost'" -f test.js 
    {
      "NodeName": "hostname:20000",
      "SessionID": 20,
      "TotalCount": 1,
      "Contexts": [
          1
      ]
    }
    ```
