SequoiaDB 通过与 Elasticsearch 配合提供全文检索能力。在 SequoiaDB 中，提供一种新类型的索引：全文索引。该索引与普通索引的典型区别在于，索引数据不是存在于数据节点的索引文件中，
而是存储在 Elasticsearch 中。在使用该索引进行查询的时候，会在 Elasticsearch 中进行搜索，数据节点根据其返回的结果，再到本地查找数据。
若要使用全文检索功能，在进行部属时，涉及到三种要素：

1. SequoiaDB 数据节点

	用户通过 SequoiaDB 进行全文索引的管理（创建、删除等），以及使用全文检索条件进行查询。

2. Elasticsearch 集群环境

	用于存储全文索引数据，以及在索引中进行搜索。每个数据组上的一个全文索引对应 Elasticsearch 上的一个索引，索引名为对应的固定集合名后接 “_” 及复制组名。在进行查询时，根据原始查询条件中包含的全文检索条件进行搜索，并返回结果。

3. 适配器 sdbseadapter

	作为 SequoiaDB 数据节点与 Elasticsearch 交互的桥梁，进行数据转换与传输等。

##相关内容##
* [搜索引擎适配器](database_management/text_search/sdbseadapter.md)
* [全文检索语法](database_management/text_search/text_search_grammer.md)
