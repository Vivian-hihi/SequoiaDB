此部分是相关 [CSharp API](api/cs/html/index.html) 文档。



##历史更新情况：##

**注意：** 

* 删除接口 - 不再兼容 
* 废弃接口 - 保持兼容性

**Version 2.10**

1. com.sequoiadb.base.DBCollection接口变更：

	* 增加OpenLob(ObjectId id, int mode)方法，其中mode取值为DBLob.SDB_LOB_READ或DBLob.SDB_LOB_WRITE。

2. com.sequoiadb.base.DBLob接口变更：

	* 增加Lock方法。
 	* 增加LockAndSeek方法。
	* 增加GetModificationTime方法。
	* Seek方法原来只能在读lob模式下使用，现在该方法支持在创建的lob或写lob模式下使用。

**Version 2.9**

1. 类SequoiaDB::Sequoiadb接口变更：

	* 增加sync方法控制数据持久化。

2. SequoiaDB::ReplicaGroup接口变更：

	* 废弃GetNodeNum接口，该接口描述的节点状态信息不准确。

3. SequoiaDB::Node接口变更：

	* 废弃GetStatus接口，该接口描述的节点状态信息不准确。


**Version 1.12**

1. 添加使用SSL连接数据库的接口