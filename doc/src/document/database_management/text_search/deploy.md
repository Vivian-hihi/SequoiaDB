要使用全文检索功能，需要完成 Elasticsearch 集群、SequoiaDB 集群及搜索引擎适配器部属。
##软件安装

### SequoiaDB 搜索引擎适配器安装
SequoiaDB 的搜索引擎适配器已包含在软件发布包中，在进行 SequoiaDB 安装的时候会一并完成安装，其可执行程序为安装目录下的 bin/sdbseadapter。

### Elasticsearch 安装 

请到 [Elasticsearch 官网](www.elastic.co)下载 Elasticsearch 安装包，并按照实际业务需要，参考 Elasticsearch 相关文档完成软件安装及集群部属。当前 SequoiaDB 适配的 Elasticsearch 版本为 6.2.2。

##组网

以下是一个简略的组网示例。三台主要上分布着 SequoiaDB 的三个复制组的所有数据节点。蓝色为 SequoiaDB 数据节点，绿色为与每个节点对应的适配器节点，最下面为 Elasticsearch 集群环境。
![](database_management/full_text_search_deploy.jpg)

- 每一个数据节点启动一个对应的适配器节点。适配器启动的时候必需指定配置文件路径，且一个配置文件只能启动一个适配器实例。尝试使用同一个配置文件启动多个适配器将会失败。配置项内容请参考[搜索引擎适配器](database_management/text_search/sdbseadapter.md)章节内容。

- 适配器的配置文件可放在任意可访问的路径下。为了方便管理和查看，建议在软件安装目录的 conf 下创建子目录，并在其中按照连接的节点端口号再创建子目录存放。