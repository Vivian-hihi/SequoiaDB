sdbseadapter 是 SequoiaDB 与 Elasticsearch 连接的桥梁，以在 SequoiaDB 上支持全文检索能力的工具。

##权限需求##

sdbseadapter 需要在 Elasticsearch 上进行读写操作的权限。

##连接需求##

sdbseadapter 在工作过程中，会根据配置信息建立与 SequoiaDB 的数据节点以及 Elasticsearch 节点的连接。 


##选项##

| 参数              | 缩写 | 描述                                     |
| ----------------- | ---- | ---------------------------------------- |
| --help            | -h   | 帮助选项                                 |
| --version         |      | 版本信息                                 |
| --confpath        | -c   | 配置文件路径（不需指定文件名）           |
| --diaglevel       | -v   | 日志级别（默认为 3）                     |
| --datanodehost    |      | 数据节点所在主机名                       |
| --datasvcname     |      | 数据节点服务端口号                       |
| --searchenginehost|      | 搜索服务器（Elasticsearch）所在主机名    |
| --searchengineport|      | 搜索服务器（Elasticsearch）服务端口号（默认9200）    |
| --optimeout       | -t   | 在搜索服务器（Elasticsearch）上操作的超时时间，范围 [0-3600000]，单位为 ms，默认值：10000|

适配器与数据节点一一对应，每个适配器需要使用一个单独的配置文件（或手工指定启动参数）。在安装路径下的 conf/samples 目录中有配置文件模板 sdbseadapter.conf，可将该文件拷贝到目标路径下并按实际组网进行配置。建议在数据节点的 conf 目录下创建单独的目录（如 seadapter），并在该目录中按节点服务端口号创建子目录，分别存放各自的配置文件。

##用法示例##

该工具主要作为 SequoiaDB 的数据节点与 Elasticsearch 交互的桥梁，在正确进行配置之后启动，确保其与 SequoiaDB 的数据节点及 Elasticsearch 均可正常连接。目前该工具通过手工方式启动（或使用外部工具进行控制）。
```
nohup sdbseadapter -c <conf_path> &
```
可使用 ps 命令查看进程是否启动：
```
ps -ef | grep sdbseadapter
```
结果参考：
```
sdbseadapter(11837) A
```
括号内为其监听搜索请求的端口号。