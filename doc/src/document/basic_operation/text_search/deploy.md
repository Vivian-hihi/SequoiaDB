全文检索功能需要在 SequoiaDB 集群环境下使用，单机模式暂不支持。要使用全文检索功能，需要完成 Elasticsearch 集群、SequoiaDB 集群及搜索引擎适配器部署。
##软件安装

### SequoiaDB 及搜索引擎适配器安装
SequoiaDB 的搜索引擎适配器已包含在软件发布包中，按照 SequoiaDB 的安装步骤正常完成安装即可。适配器可执行程序为安装目录下的 bin/sdbseadapter。

### Elasticsearch 安装 

请到 [Elasticsearch 官网](http://www.elastic.co)下载 Elasticsearch 安装包，并按照实际业务需要，参考 Elasticsearch 相关文档完成软件安装及集群部署。当前 SequoiaDB 适配的 Elasticsearch 版本为 6.2.2。


## 配置全文检索运行环境

### SequoiaDB 及 Elasticsearch 部署
请参考 SequoiaDB 及 Elasticsearch 的相关指导，完成 SequoiaDB 及 Elasticsearch 集群的部署，并确保其正常运行。

### 搜索引擎适配器部署
#### 适配器节点配置文件准备
每一个数据节点（包括主节点和备节点）需要启动一个对应的适配器节点，二者需要运行在同一台主机上。适配器启动的时候必需指定配置文件路径，且一个配置文件只能启动一个适配器实例。尝试使用同一个配置文件启动多个适配器实例将会失败。

当需要使用全文检索功能时，在 SequoiaDB 安装目录的 conf 目录下，创建 seadapter 目录，并在该目录下，按适配器对应的数据节点的服务端口号，分别创建下层子目录并存放一份配置文件。配置文件模板可从 conf/samples/sdbseadapter.conf 拷贝，文件名应保持一致，然后依次对配置文件内容进行修改。详细的配置项内容请参考[搜索引擎适配器](basic_operation/text_search/sdbseadapter.md)章节内容。

#### 适配器节点启动
目前适配器进程通过手工方式启动，通过 -c 指定配置文件路径（不需要带配置文件名）：

```lang-javascript
$ nohup sdbseadapter -c <conf_path> &
```

可使用 ps 命令查看是否所有适配器进程均已启动成功：

```lang-javascript
$ ps -ef | grep sdbseadapter
```

结果参考：

```lang-javascript
sdbseadapter(11837) A
```

括号内为其监听搜索请求的端口号。
全文检索环境部署完成之后，在允许的情况下，建议参考[全文检索语法](basic_operation/text_search/text_search_grammer.md)进行简单的功能验证。