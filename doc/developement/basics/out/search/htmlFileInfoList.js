fil = new Array();
fil["0"]= "topics/create.html@@@创建@@@在sequoiadb中，create操作是向集合中添加新的文档记录。我们可以使用insert方法向sequoiadb中的集合中添加记录。 所有的插入操作在sequoiadb中具有如下性质： 如果插入的文档记录没有_id字段，客户端将会为记录自动添加_id字段，并且填充一个唯一值。 如果指定_id字段，那个在集合中_id的值必须唯一；否则出现操作异常。 最大的BSON文档长度为16MB...";
fil["1"]= "topics/delete.html@@@删除@@@四大基本数据库操作即（CRUD），删除操作即移除集合中的记录。sequoiadb中使用 remove() 方法做删除操作。 本文档的所有例子都是使用sequoiadb的shell接口。 remove() remove()方法是删除集合中记录主要方法，它的语法结构为： db.collectionspace.collection.remove([cond],[hint...";
fil["2"]= "topics/read.html@@@读@@@四大基本数据库操作即（CRUD），读操作是从sequoiadb中的集合中检索文档记录，包括所有响应应用程序请求数据并返回游标的操作。 本文档描述了应用程序从sequoiadb中请求数据的查询语法和结构，以及不同因素影响效率的读取请求。 本文档的所有例子都是使用sequoiadb的shell接口。 find() 我们使用 find...";
fil["3"]= "topics/shell.html@@@Shell中的基本操作@@@在shell查看操作数据会用到4个基本操作：创建、读取、更新和删除(CRUD...";
fil["4"]= "topics/update.html@@@更新@@@四大基本数据库操作即（CRUD），更新操作即修改集合中已存在的记录。sequoiadb中使用 update() 方法做更新操作。 本文档的所有例子都是使用sequoiadb的shell接口。 update() update()方法是修改集合中记录的主要方法，它的语法结构为： db.collectionspace.collection.update(<rule>,[cond],[hint...";
