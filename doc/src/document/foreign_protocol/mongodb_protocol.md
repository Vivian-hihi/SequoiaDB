MongoDB 是一款开源的非关系型数据库，也是目前最流行的非关系型数据库之一。
SequoiaDB 兼容 MongoDB 语法和协议，用户可以使用 MongoDB 的驱动访问 SequoiaDB 数据库，完成对数据的增、删、查、改操作以及其他操作。

**SequoiaDB 所支持的 MongoDB 版本**

- MongoDB 2.x

- MongoDB 3.x

- MongoDB 4.x

## 使用

假定 sequoiadb 安装目录为 /opt/sequoiadb，协调节点端口号为 11810。

- 配置连接器 fap

  修改协调节点配置文件

  ```lang-bash
  $ cd /opt/sequoiadb
  $ vi conf/local/11810/sdb.conf
  ```
  
  添加一行配置
  
  ```
  fap=fapmongo2
  ```

  > **Note:**  
  >
  > * 如兼容 mongodb 2.6 版本，请添加配置“fap=fapmongo2”；如兼容 mongodb 3.4 版本请添加配置“fap=fapmongo3”；如兼容 mongodb 4.2 版本请添加配置“fap=fapmongo4”

- 重启协调节点

  ```lang-bash
  $ ./bin/sdbstop -p 11810
  $ ./bin/sdbstart -p 11810
  ```

- 查看 fap 端口

  连接器 fap 的端口号为协调节点端口号+7

  ```lang-bash
 $ netstat -anp | grep 11817
  tcp        0      0 0.0.0.0:11817           0.0.0.0:*               LISTEN      20462/sequoiadb(118
  ```

  这时可以使用 mongodb 驱动连接到 11817 端口上执行命令。

## 兼容命令

| 命令              | fapmongo2是否支持 | fapmongo3是否支持 | fapmongo4是否支持 |
| ----------------  | ----------------- | ----------------- | ----------------- |
| create collection | 是 | 是 | 是 |
| drop collection   | 是 | 是 | 是 |
| list collections  | 是 | 是 | 是 |
| drop database     | 是 | 是 | 是 |
| list databases    | 否 | 是 | 是 |
| insert            | 是 | 是 | 是 |
| remove            | 是 | 是 | 是 |
| update            | 是 | 是 | 是 |
| find              | 是 | 是 | 是 |
| count             | 是 | 是 | 是 |
| aggregate         | 是 | 是 | 是 |
| distinct          | 否 | 是 | 是 |
| create index      | 是 | 是 | 是 |
| drop index        | 是 | 是 | 是 |
| list indexes      | 是 | 是 | 是 |
| create user       | 是 | 是 | 否 |
| drop user         | 是 | 是 | 否 |
| auth              | 只支持 MONGODB-CR 算法 | 只支持 MONGODB-CR 算法 | 否 |
| logout            | 是 | 是 | 是 |

