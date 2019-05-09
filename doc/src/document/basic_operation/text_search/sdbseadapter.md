sdbseadapter 是 SequoiaDB 与 Elasticsearch 连接的桥梁，以在 SequoiaDB 上支持全文检索能力的工具。

##选项##

| 参数              | 缩写 | 描述                                     |
| ----------------- | ---- | ---------------------------------------- |
| --help            | -h   | 帮助选项                                 |
| --version         |      | 版本信息                                 |
| --confpath        | -c   | 配置文件路径（不需指定文件名）           |
| --diaglevel       | -v   | 日志级别，默认值：3                      |
| --datanodehost    |      | 数据节点所在主机名                       |
| --datasvcname     |      | 数据节点服务端口号                       |
| --searchenginehost|      | 搜索服务器（Elasticsearch）所在主机名    |
| --searchengineport|      | 搜索服务器（Elasticsearch）服务端口号，默认值：9200 |
| --idxprefix       | -p   | 全文检索适配器在搜索服务器（Elasticsearch）上创建索引时使用的索引名前缀，可包含英文字母（大小写不敏感）、阿拉伯数字及下划线，且不能以下划线开头，最大长度16个字符，默认值为空。注意：同一个复制组对应的适配器必须使用相同的前缀，不同的 SequoiaDB 集群在共用相同的搜索服务器时，必须配置不同的前缀名。建议一个 SequoiaDB 集群中的所有的适配器使用相同的前缀 |
| --bulkbuffsize    |      | 批量操作缓存大小，范围[1-32]，单位：MB，默认值：10 |
| --optimeout       | -t   | 在搜索服务器（Elasticsearch）上操作的超时时间，范围 [3000-3600000]，单位：ms，默认值：10000|

适配器与数据节点一一对应，每个适配器需要使用一个单独的配置文件（或手工指定启动参数）。在安装路径下的 conf/samples 目录中有配置文件模板 sdbseadapter.conf，可将该文件拷贝到目标路径下并按实际组网进行配置。建议在数据节点的 conf 目录下创建单独的目录（如 seadapter），并在该目录中按节点服务端口号创建子目录，分别存放各自的配置文件。