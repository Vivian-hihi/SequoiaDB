fil = new Array();
fil["0"]= "SYSCOLLECTIONS.html@@@SYSCOLLECTIONS集合@@@所属集合空间 SYSCAT 概念 SYSCOLLECTIONS集合中包含了该集群中所有的用户集合信息。每个用户集合保存为一个文档。 每个文档包含以下字段： 字段名 类型 描述 Name 字符串 集合的完整名，为<集合空间>.<集合名>形式。 Version 整数 集合的版本号，由1起始，每次对该集合的元数据变更会造成版本号+1。 ReplSize...";
fil["1"]= "SYSCOLLECTIONSPACES.html@@@SYSCOLLECTIONSPACES集合@@@所属集合空间 SYSCAT 概念 SYSCOLLECTIONSPACES集合中包含了该集群中所有的用户集合空间信息。每个用户集合空间保存为一个文档。 每个文档包含以下字段： 字段名 类型 描述 Name 字符串 集合空间名。 Collection 数组 该集合空间中包含的所有的集合名，每个集合为一个JSON对象，包含“Name”字段与相应的集合名。 Group 数组 该集合空间所在的复制组ID...";
fil["2"]= "SYSNODES.html@@@SYSNODES集合@@@所属集合空间 SYSCAT 概念 SYSNODES集合中包含了该集群中所有的节点与分区组信息。每个分区组保存为一个文档。 每个文档包含以下字段： 字段名 类型 描述 GroupName 字符串 分区组名 GroupID 整数 分区组ID，该ID在集群中唯一 PrimaryNode 整数 该分区组内主节点ID Role 整数 分区组角色，可以为： 0：数据节点 2：编目节点 Status...";
