此部分是相关 [Java API](api/java/html/index.html) 文档。

##历史更新情况##

**注意：** 

* 删除接口 - 不再兼容 
* 废弃接口 - 保持兼容性

**Version 2.10**

1. com.sequoiadb.base.DBCollection接口变更：

	* 增加openLob(ObjectId id, int mode)方法，其中mode取值为DBLob.SDB_LOB_READ或DBLob.SDB_LOB_WRITE。
	* 增加truncateLob方法。

2. com.sequoiadb.base.DBLob接口变更：

	* 增加lock方法。
 	* 增加lockAndSeek方法。
	* 增加getModificationTime方法。
	* seek方法原来只能在读lob模式下使用，现在该方法支持在创建的lob或写lob模式下使用。

3. com.sequoiadb.base.ReplicaGroup接口变更：

	* getSlave方法增加可指定节点位置的参数。

**Version 2.9**

1. com.sequoiadb.base.Sequoiadb接口变更： 

    * 删除getConnection方法，IConnection是内部网络通信接口，不再对外开放。
	* 删除getDataCenter方法。
	* 删除setServerAddress方法，该方法无意义。
	* 废弃getServerAddress方法，增加getHost、getPort方法。
	* 废弃changeConnectionOptions方法。
	* 废弃disconnect方法。
	* 废弃com.sequoiadb.net.ConfigOptions相关的构造方法，增加com.sequoiadb.base.ConfigOptions相关的构造方法。
	* 废弃isEndianConvert方法，增加getByteOrder方法。
	* closeAllCursors方法在连接已关闭时不再报错。
	* 实现java.io.Closeable接口，增加close方法，在JDK1.7上支持资源自动释放。新增的close方法取代disconnect接口。
	* 增加getLastUseTime方法，该接口主要被数据源使用。
	* 增加close方法取代原来disconnect的功能。
	* 增加sync方法控制数据持久化。

2. com.sequoiadb.base.DBCollection接口变更：

	* getCollection方法要获取的collection不存在时，不再返回null而是抛出异常。

3. com.sequoiadb.base.DBCusor接口变更：

	* 实现java.io.Closeable接口，在JDK1.7上支持资源自动释放。
	* getNext和getNextRaw方法可以混合交替使用。
	* 废弃hasNextRaw接口，可使用hasNext取代该接口。

4. 废弃com.sequoiadb.base.SequoiadbDatasource类，增加com.sequoiadb.datasource.SequoiadbDatasource类。

5. com.sequoiadb.base.ReplicaGroup接口变更：

	* 废弃getNodeNum接口，该接口描述的节点状态信息不准确。

6. com.sequoiadb.base.Node接口变更：

	* 废弃getStatus接口，该接口描述的节点状态信息不准确。

7. 删除DataCenter接口，待相关功能发布之后再提供接口。

8. org.bson.BSONObject接口实现java.io.Serializable接口。

9. org.bson.types.BSONTimestamp支持从java.util.Date和java.sql.Timestamp构造，并增加转换为java.util.Date和java.sql.Timestamp的方法。

10. BSONObject支持java.sql.Timestamp，将java.sql.Timestamp编码为timestamp类型。

**Version 1.10**

1. DBCollection类新添加的接口：

	* createLob，创建一个大对象。
	* openLob，打开一个已存在的大对象。
	* removeLob，删除一个大对象。
	* listLobs，列出所有大对象。
	* explain，获取执行访问计划。

2. 增加大对象类DBLob，用于操作大对象：

	* write，向一个大对象写入数据。
	* read，从大对象中读取数据。
	* seek，指定读取数据的偏移。
	* close，关闭一个大对象。
	* getID，获取大对象的标识ID。
	* getSize，获取大对象的大小。
	* getCreateTime，获取大对象的创建时间。

**Version 1.8**

1. Sequoiadb 类新添加的接口：

	* isValid，判断当前连接是否有效。
	* createCollectionSpace，提供一个 BSONObject 的选项，使创建集合空间更加灵活。
	* backupOffline，离线备份支持更多的选项。
	* evalJS，执行javascript代码。
	* createDomain，创建域。
	* getDomain，获取域。
	* dropDomain，删除域。
	* isDomainExist，域是否存在。
	* listDomain，列出所有域。

2. DBCollection 类新添加的接口：

	* alterCollection，修改集合（表）属性
	  setMainKeys，设置主键。此接口只与 save 接口配合使用，
      它设置的主键并不对其他接口起作用。
	* save，可使用默认的主键"_id"或者指定其他主键，同时插入或更新多条记录。
3. 添加 Domain 类用于与域相关的操作。
4. SequoiadbDatasource类新添加的接口：

	* SequoiadbDatasource，可提供多个地址的构造器，便于机器负载均衡。
	* getIdleConnNum，获取当前可用的连接数量。
	* getUsedConnNum，获取当前已使用的连接数量。
	* getNormalAddrNum，获取当前正常的地址数量。
	* getAbnormalAddrNum，获取当前异常的地址数量。

5. SequoiadbOption类新添加接口：

	* setRecaptureConnPeriod，设置周期检测异常地址是否重新可用的时间。
	* getRecaptureConnPeriod，获取周期检测异常地址是否重新可用的时间。

**Version 1.6**

1. 添加类Node来取代原来的类ReplicaGroup。类 ReplicaNode 以及与它们相关的方法将在 version 2.x 中被弃用。

详情请查看相关 [Java API](api/java/html/index.html)。
