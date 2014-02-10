/**
 *      Copyright (C) 2012 SequoiaDB Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
/**
 * @package com.sequoiadb.base;
 * @brief SequoiaDB Driver for Java
 * @author Jacky Zhang
 */
package com.sequoiadb.base;

import java.util.ArrayList;
import java.util.Map;
import java.util.Set;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.bson.BSONEncoder;
import org.bson.BSONObject;
import org.bson.BasicBSONEncoder;
import org.bson.BasicBSONObject;
import org.bson.types.Code;
import org.bson.types.CodeWScope;
import org.bson.util.JSON;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.net.ConfigOptions;
import com.sequoiadb.net.ConnectionTCPImpl;
import com.sequoiadb.net.IConnection;
import com.sequoiadb.net.ServerAddress;
import com.sequoiadb.util.SDBMessageHelper;

/**
 * @class Sequoiadb
 * @brief Database operation interfaces of admin.
 */
public class Sequoiadb {
	private ServerAddress serverAddress;
	// private CollectionSpace currentCollectionSpace;
	private IConnection connection;
	// private ConcurrentMap<String, CollectionSpace> collectionSpaces = new
	// ConcurrentHashMap<String, CollectionSpace>();
	private String userName;
	private String password;
	boolean endianConvert;

	public final static int SDB_PAGESIZE_4K = 4096;
	public final static int SDB_PAGESIZE_8K = 8192;
	public final static int SDB_PAGESIZE_16K = 16384;
	public final static int SDB_PAGESIZE_32K = 32768;
	public final static int SDB_PAGESIZE_64K = 65536;
	public final static int SDB_PAGESIZE_DEFAULT = SDB_PAGESIZE_4K;

	public final static int SDB_LIST_CONTEXTS = 0;
	public final static int SDB_LIST_CONTEXTS_CURRENT = 1;
	public final static int SDB_LIST_SESSIONS = 2;
	public final static int SDB_LIST_SESSIONS_CURRENT = 3;
	public final static int SDB_LIST_COLLECTIONS = 4;
	public final static int SDB_LIST_COLLECTIONSPACES = 5;
	public final static int SDB_LIST_STORAGEUNITS = 6;
	/**SDB_LIST_GROUPS will be deprecated in version 2.x, use SDB_LIST_SHARDS instead of it.*/
	public final static int SDB_LIST_GROUPS = 7;
	public final static int SDB_LIST_SHARDS = 7;
	public final static int SDB_LIST_STOREPROCEDURES = 8;

	public final static int SDB_SNAP_CONTEXTS = 0;
	public final static int SDB_SNAP_CONTEXTS_CURRENT = 1;
	public final static int SDB_SNAP_SESSIONS = 2;
	public final static int SDB_SNAP_SESSIONS_CURRENT = 3;
	public final static int SDB_SNAP_COLLECTIONS = 4;
	public final static int SDB_SNAP_COLLECTIONSPACES = 5;
	public final static int SDB_SNAP_DATABASE = 6;
	public final static int SDB_SNAP_SYSTEM = 7;
	public final static int SDB_SNAP_CATALOG = 8;
	
	public final static int FMP_FUNC_TYPE_INVALID = -1;
	public final static int FMP_FUNC_TYPE_JS = 0;
	public final static int FMP_FUNC_TYPE_C = 1;
	public final static int FMP_FUNC_TYPE_JAVA = 2;
	
	public final static String CATALOG_GROUP_NAME = "SYSCatalogGroup";

	/**
	 * @fn IConnection getConnection()
	 * @brief Get the current connection to remote server.
	 * @return IConnection
	 */
	public IConnection getConnection() {
		return connection;
	}

	/**
	 * @fn ServerAddress getServerAddress()
	 * @brief Get the address of remote server.
	 * @return ServerAddress
	 */
	public ServerAddress getServerAddress() {
		return serverAddress;
	}

	/**
	 * @fn void setServerAddress(ServerAddress serverAddress)
	 * @brief Set the address of remote server.
	 * @param serverAddress
	 *            the serverAddress object of remote server
	 */
	public void setServerAddress(ServerAddress serverAddress) {
		this.serverAddress = serverAddress;
	}

	/**
	 * @fn Sequoiadb(String username, String password)
	 * @brief Default Constructor Server address "127.0.0.1 : 50000".
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public Sequoiadb(String username, String password) throws BaseException {
		serverAddress = new ServerAddress();
		initConnection();
		this.userName = username;
		this.password = password;
		connect();
	}

	/**
	 * @fn Sequoiadb(String connString, String username, String password)
	 * @brief Constructor.
	 * @param connString
	 *            Remote server address "IP : Port" or "IP"(port is 50000)
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public Sequoiadb(String connString, String username, String password)
			throws BaseException {
		try {
			serverAddress = new ServerAddress(connString);
			initConnection();
		} catch (UnknownHostException e) {
			throw new BaseException("SDB_NETWORK", connString);
		}
		this.userName = username;
		this.password = password;
		connect();
	}

	/**
	 * @fn Sequoiadb(String addr, int port, String username, String password)
	 * @brief Constructor.
	 * @param addr
	 *            IP address
	 * @param port
	 *            Port
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public Sequoiadb(String addr, int port, String username, String password)
			throws BaseException {
		try {
			serverAddress = new ServerAddress(addr, port);
			initConnection();
		} catch (UnknownHostException e) {
			throw new BaseException("SDB_NETWORK", addr, port);
		}
		this.userName = username;
		this.password = password;
		connect();
	}

	/**
	 * @fn connect()
	 * @brief Connect to database
	 */
	private void connect() {
		endianConvert = requestSysInfo();
		byte[] request = SDBMessageHelper.buildAuthMsg(userName, password, 0,
				(byte) 0, endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtn = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtn.getFlags();
		if (flags != 0) {
			connection.close();
			throw new BaseException(flags, userName, password);
		}
	}

	/**
	 * @fn void createUser(String username, String password)
	 * @brief Add an user in current database.
	 * @param username
	 *            The connection user name
	 * @param password
	 *            The connection password
	 */
	public void createUser(String username, String password) throws BaseException {
		if(username == null || password == null) {
			throw new BaseException("SDB_INVALIDARG");
		}
		byte[] request = SDBMessageHelper.buildAuthMsg(username, password,
				(long) 0, (byte) 1, endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtn = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, username, password);
		}
	}

	/**
	 * @fn void removeUser(String username, String password)
	 * @brief Remove the spacified user from current database.
	 * @param username
	 *            The connection user name
	 * @param password
	 *            The connection password
	 */
	public void removeUser(String username, String password) throws BaseException {
		byte[] request = SDBMessageHelper.buildAuthMsg(username, password, (long)0,
				(byte) 2, endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtn = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, username, password);
		}
	}

	/**
	 * @fn void disconnect()
	 * @brief Disconnect the remote server.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void disconnect() throws BaseException {
		byte[] request = SDBMessageHelper.buildDisconnectRequest(endianConvert);
		connection.sendMessage(request);
		connection.close();
	}

	/**
	 * @fn void changeConnectionOptions(ConfigOptions opts)
	 * @brief Change the connection options.
	 * @param opts
	 *            The connection options
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void changeConnectionOptions(ConfigOptions opts)
			throws BaseException {
		connection.changeConfigOptions(opts);
		connect();
	}

	/**
	 * @fn void createCollectionSpace(String collectionSpaceName)
	 * @brief Create the named collection space with default SDB_PAGESIZE_4K.
	 * @param csName
	 *            The collection space name
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public CollectionSpace createCollectionSpace(String csName)
			throws BaseException {
		return createCollectionSpace(csName, SDB_PAGESIZE_4K);
	}

	/**
	 * @fn void createCollectionSpace(String collectionSpaceName, int pageSize)
	 * @brief Create the named collection space.
	 * @param csName
	 *            The collection space name
	 * @param pageSize
	 *            The Page Size as below SDB_PAGESIZE_4K SDB_PAGESIZE_8K
	 *            SDB_PAGESIZE_16K SDB_PAGESIZE_32K SDB_PAGESIZE_64K
	 *            SDB_PAGESIZE_DEFAULT
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public CollectionSpace createCollectionSpace(String csName, int pageSize)
			throws BaseException {
		if (isCollectionSpaceExist(csName))
			throw new BaseException("SDB_DMS_CS_EXIST", csName);
		if (pageSize != SDB_PAGESIZE_4K && pageSize != SDB_PAGESIZE_8K
				&& pageSize != SDB_PAGESIZE_16K && pageSize != SDB_PAGESIZE_32K
				&& pageSize != SDB_PAGESIZE_64K) {
			throw new BaseException("SDB_INVALIDARG", pageSize);
		}
		SDBMessage rtnSDBMessage = createCS(csName, pageSize);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0)
			throw new BaseException(flags);
		return getCollectionSpace(csName);
	}

	/**
	 * @fn void dropCollectionSpace(String collectionSpaceName)
	 * @brief Remove the named collection space.
	 * @param csName
	 *            The collection space name
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void dropCollectionSpace(String csName) throws BaseException {
		if (!isCollectionSpaceExist(csName)) {
			throw new BaseException("SDB_DMS_CS_NOTEXIST", csName);
		}
		BSONObject matcher = new BasicBSONObject();
		matcher.put(SequoiadbConstants.FIELD_NAME_NAME, csName);
		String commandString = SequoiadbConstants.DROP_CMD + " "
				+ SequoiadbConstants.COLSPACE;
		SDBMessage rtn = adminCommand(commandString, 0, 0, -1, -1, matcher,
				null, null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags);
		}
	}

	/**
	 * @fn CollectionSpace getCollectionSpace(String csName)
	 * @brief Get the named collection space.
	 * @param csName
	 *            The collection space name
	 * @return The CollecionSpace handle
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public CollectionSpace getCollectionSpace(String csName)
			throws BaseException {
		if (isCollectionSpaceExist(csName)) {
			return new CollectionSpace(this, csName.trim());
		} else {
			throw new BaseException("SDB_DMS_CS_NOTEXIST", csName);
		}
	}

	/**
	 * @fn boolean isCollectionSpaceExist(String csName)
	 * @brief Verify the existence of collection space.
	 * @param csName
	 *            The collecion space name
	 * @return True if existed or False if not existed
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public boolean isCollectionSpaceExist(String csName) throws BaseException {
		String commandString = SequoiadbConstants.TEST_CMD + " "
				+ SequoiadbConstants.COLSPACE;
		BSONObject matcher = new BasicBSONObject();
		matcher.put(SequoiadbConstants.FIELD_NAME_NAME, csName);
		SDBMessage rtn = adminCommand(commandString, 0, 0, -1, -1, matcher,
				null, null, null);
		int flags = rtn.getFlags();
		if (flags == 0)
			return true;
		else if (flags == new BaseException("SDB_DMS_CS_NOTEXIST")
				.getErrorCode())
			return false;
		else
			throw new BaseException(flags, csName);
	}

	/**
	 * @fn DBCursor listCollectionSpaces()
	 * @brief Get all the collecionspaces.
	 * @return cursor of all collecionspace names
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor listCollectionSpaces() throws BaseException {
		return getList(SDB_LIST_COLLECTIONSPACES, 0, 0, -1, -1, null, null,
				null, null);
	}
	
	/**
	 * @fn DBCursor listShards()
	 * @brief List all the shards.
	 * @return cursor of all collecionspace names
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor listShards() throws BaseException {
		return getList(SDB_LIST_SHARDS, 0, 0, -1, -1, null, null,
				null, null);
	}
	/**
	 * @fn ArrayList<String> getCollectionSpaceNames()
	 * @brief Get all the collecion space names
	 * @return A list of all collecion space names
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public ArrayList<String> getCollectionSpaceNames() throws BaseException {
		DBCursor cursor = getList(SDB_LIST_COLLECTIONSPACES, 0, 0, -1, -1,
				null, null, null, null);
		if (cursor == null)
			return null;
		ArrayList<String> colList = new ArrayList<String>();
		while (cursor.hasNext()) {
			colList.add(cursor.getNext().get("Name").toString());
		}
		return colList;
	}

	/**
	 * @fn ArrayList<String> getShardNames()
	 * @brief Get all the shards' names.
	 * @return A list of all the shards' names.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public ArrayList<String> getShardNames() throws BaseException {
		DBCursor cursor = getList(SDB_LIST_SHARDS, 0, 0, -1, -1,
				null, null, null, null);
		if (cursor == null)
			return null;
		ArrayList<String> colList = new ArrayList<String>();
		while (cursor.hasNext()) {
			colList.add(cursor.getNext().get("GroupName").toString());
		}
		return colList;
	}
	
	/**
	 * @fn DBCursor listCollections()
	 * @brief Get all the collections
	 * @return dbCursor of all collecions
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor listCollections() throws BaseException {
		return getList(SDB_LIST_COLLECTIONS, 0, 0, 0, -1, null, null, null,
				null);
	}

	/**
	 * @fn ArrayList<String> getCollectionNames()
	 * @brief Get all the collection names
	 * @return A list of all collecion names
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public ArrayList<String> getCollectionNames() throws BaseException {
		DBCursor cursor = getList(SDB_LIST_COLLECTIONS, 0, 0, 0, -1, null,
				null, null, null);
		if (cursor == null)
			return null;
		ArrayList<String> colList = new ArrayList<String>();
		while (cursor.hasNext()) {
			colList.add(cursor.getNext().get("Name").toString());
		}
		return colList;
	}

	/**
	 * @fn List<BSONObject> getStorageUnits()
	 * @brief Get all the storage units
	 * @return A list of all storage units
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public ArrayList<String> getStorageUnits() throws BaseException {
		DBCursor cursor = getList(SDB_LIST_STORAGEUNITS, 0, 0, -1, -1, null,
				null, null, null);
		ArrayList<String> colList = new ArrayList<String>();
		while (cursor.hasNext()) {
			colList.add(cursor.getNext().get("Name").toString());
		}
		return colList;
	}

	/**
	 * @fn void resetSnapshot()
	 * @brief Reset the snapshot
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void resetSnapshot() throws BaseException {
		String commandString = SequoiadbConstants.SNAP_CMD + " "
				+ SequoiadbConstants.RESET;
		SDBMessage rtn = adminCommand(commandString, 0, 0, -1, -1, null, null,
				null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags);
		}
	}

	/**
	 * @fn DBCursor getList(int listType, BSONObject query, BSONObject selector,
			BSONObject orderBy)
     * @brief Get the informations of specified type.
     * @param listType The list type as below:
     *<dl>
     *<dt>Sequoiadb.SDB_LIST_CONTEXTS   : Get all contexts list
     *<dt>Sequoiadb.SDB_LIST_CONTEXTS_CURRENT        : Get contexts list for the current session
     *<dt>Sequoiadb.SDB_LIST_SESSIONS        : Get all sessions list
     *<dt>Sequoiadb.SDB_LIST_SESSIONS_CURRENT        : Get the current session
     *<dt>Sequoiadb.SDB_LIST_COLLECTIONS        : Get all collections list
     *<dt>Sequoiadb.SDB_LIST_COLLECTIONSPACES        : Get all collecion spaces list
     *<dt>Sequoiadb.SDB_LIST_STORAGEUNITS        : Get storage units list
     *<dt>Sequoiadb.SDB_LIST_SHARDS        : Get shard list ( only applicable in sharding env )
     *<dt>Sequoiadb.SDB_LIST_STOREPROCEDURES           : Get stored procedure list ( only applicable in sharding env )
     *</dl>
     * @param query The matching rule, match all the documents if null.
     * @param selector The selective rule, return the whole document if null.
     * @param orderBy The ordered rule, never sort if null.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor getList(int listType, BSONObject query, BSONObject selector, BSONObject orderBy) throws BaseException {
		return getList(listType, 0, 0, 0, -1, query, selector, orderBy, null);
	}
	
	/**
	 * @fn List<String> getShardsInfo()
	 * @brief Get the infomations of the shards.
	 * @return A list of informations of the shards.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public ArrayList<String> getShardsInfo() throws BaseException {
		DBCursor cursor = getList(SDB_LIST_SHARDS, 0, 0, -1, -1, null, null,
				null, null);
		if (cursor == null)
			return null;
		ArrayList<String> colList = new ArrayList<String>();
		while (cursor.hasNext()) {
			colList.add(cursor.getNext().toString());
		}
		return colList;
	}

	/**
	 * @fn Shard getShard(String shardName)
	 * @brief Get shard by name.
	 * @param shardName
	 *            shard name
	 * @return A shard object or null for not exit.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public Shard getShard(String shardName)
			throws BaseException {
		BSONObject shard = getDetailByName(shardName);
		if (shard == null)
			return null;
		return new Shard(this, shardName);
	}

	/**
	 * @fn Shard getShard(int shardId)
	 * @brief Get shard by id.
	 * @param shardId
	 *            shard id
	 * @return A shard or null for not exit.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public Shard getShard(int shardId) throws BaseException{
		BSONObject shard = getDetailById(shardId);
		if (shard == null)
			return null;
		return new Shard(this, shardId);
	}

	/**
	 * @fn Shard createShard(String shardName)
	 * @brief Create shard by name.
	 * @param shardName
	 *            shard name
	 * @return A shard object.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public Shard createShard(String shardName)
			throws BaseException {
		BSONObject shard = new BasicBSONObject();
		shard.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, shardName);
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_CREATE_GROUP,
				0, 0, -1, -1, shard, null, null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, shardName);
		}
		return getShard(shardName);
	}

	/**
	 * @fn void removeShard(String shardName)
	 * @brief Remove shard by name.
	 * @param shardName
	 *            shard name
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void removeShard(String shardName)
			throws BaseException {
		BSONObject shard = new BasicBSONObject();
		shard.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, shardName);
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_REMOVE_GROUP,
				0, 0, -1, -1, shard, null, null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, shardName);
		}
	}

	/**
	 * @fn void activateShard(String shardName)
	 * @brief Active shard by name.
	 * @param shardName
	 *            shard name
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void activateShard(String shardName)
			throws BaseException {
		BSONObject shard = new BasicBSONObject();
		shard.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, shardName);
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_ACTIVE_GROUP,
				0, 0, -1, -1, shard, null, null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, shardName);
		}
	}

	/**
	 * @fn void createCataShard(String hostName, int port, String dbPath,
	 *     BSONObject configuration)
	 * @brief Create the Catalog shard with given options.
	 * @param hostName
	 *            The host name
	 * @param port
	 *            The port
	 * @param dbpath
	 *            The database path
	 * @param configure
	 *            The configure options
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void createCataShard(String hostName, int port,
			String dbPath, Map<String, String> configure) {
		String commandString = SequoiadbConstants.CMD_NAME_CREATE_CATA_GROUP;
		BSONObject obj = new BasicBSONObject();
		obj.put(SequoiadbConstants.FIELD_NAME_HOST, hostName);
		obj.put(SequoiadbConstants.PMD_OPTION_SVCNAME, Integer.toString(port));
		obj.put(SequoiadbConstants.PMD_OPTION_DBPATH, dbPath);
		if (configure != null) {
			for (String key : configure.keySet()) {
				if (key.equals(SequoiadbConstants.FIELD_NAME_HOST)
						|| key.equals(SequoiadbConstants.PMD_OPTION_SVCNAME)
						|| key.equals(SequoiadbConstants.PMD_OPTION_DBPATH)) {
					continue;
				}
				obj.put(key, configure.get(key).toString());
			}
		}
		SDBMessage rtn = adminCommand(commandString, 0, 0, -1, -1, obj, null,
				null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags);
		}
	}

	/**
	 * @fn void flushConfigure(BSONObject param)
	 * @brief Flush the options to configuration file
	 * @param param
	 *            The param of flush, pass {"Global":true} or {"Global":false}
	 *            In cluster environment, passing {"Global":true} will flush data's and catalog's configuration file,
	 *            while passing {"Global":false} will flush coord's configuration file
	 *            In stand-alone environment, both them have the same behaviour
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void flushConfigure(BSONObject param) throws BaseException {
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_EXPORT_CONFIG,0,0,0,-1,param,null,null,null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags);
		}
	}

	/**
	 * @fn void execUpdate(String sql)
	 * @brief Execute sql in db
	 * @param sql
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void execUpdate(String sql) {
		SDBMessage sdb = new SDBMessage();
		sdb.setRequestID(0);
		sdb.setNodeID(SequoiadbConstants.ZERO_NODEID);
		byte[] request = SDBMessageHelper.buildSqlMsg(sdb, sql, endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtn = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, sql);
		}
	}

	/**
	 * @fn DBCursor exec(String sql)
	 * @brief Execute sql in db
	 * @param sql
	 * @return the DBCursor of the result
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor exec(String sql) {
		SDBMessage sdb = new SDBMessage();
		sdb.setRequestID(0);
		sdb.setNodeID(SequoiadbConstants.ZERO_NODEID);
		byte[] request = SDBMessageHelper.buildSqlMsg(sdb, sql, endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtn = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtn.getFlags();
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC)
				return null;
			else {
				throw new BaseException(flags, sql);
			}
		}
		return new DBCursor(rtn, this);
	}

	/**
	 * @fn DBCursor getSnapshot(int snapType, String matcher, String selector,
	 *     String orderBy)
	 * @brief Get snapshot of the database.
     * @param snapType The snapshot types are as below:
     * <dl>
     * <dt>Sequoiadb.SDB_SNAP_CONTEXTS   : Get all contexts' snapshot
     * <dt>Sequoiadb.SDB_SNAP_CONTEXTS_CURRENT        : Get the current context's snapshot
     * <dt>Sequoiadb.SDB_SNAP_SESSIONS        : Get all sessions' snapshot
     * <dt>Sequoiadb.SDB_SNAP_SESSIONS_CURRENT        : Get the current session's snapshot
     * <dt>Sequoiadb.SDB_SNAP_COLLECTIONS        : Get the collections' snapshot
     * <dt>Sequoiadb.SDB_SNAP_COLLECTIONSPACES        : Get the collection spaces' snapshot
     * <dt>Sequoiadb.SDB_SNAP_DATABASE        : Get database's snapshot
     * <dt>Sequoiadb.SDB_SNAP_SYSTEM        : Get system's snapshot
     * <dt>Sequoiadb.SDB_SNAP_CATALOG        : Get catalog's snapshot
     * <dt>Sequoiadb.SDB_LIST_SHARDS        : Get shard list ( only applicable in sharding env )
     * <dt>Sequoiadb.SDB_LIST_STOREPROCEDURES           : Get stored procedure list ( only applicable in sharding env )
     * </dl>
	 * @param matcher
	 *            The matching rule, match all the documents if null
	 * @param selector
	 *            The selective rule, return the whole document if null
	 * @param orderBy
	 *            The ordered rule, never sort if null
	 * @return The DBCursor of snapshot.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor getSnapshot(int snapType, String matcher, String selector,
			String orderBy) {
		BSONObject ma = null;
		BSONObject se = null;
		BSONObject or = null;
		if (matcher != null)
			ma = (BSONObject) JSON.parse(matcher);
		if (selector != null)
			se = (BSONObject) JSON.parse(selector);
		if (orderBy != null)
			or = (BSONObject) JSON.parse(orderBy);

		return getSnapshot(snapType, ma, se, or);
	}

	/**
	 * @fn DBCursor getSnapshot(int snapType, BSONObject matcher, BSONObject
	 *     selector, BSONObject orderBy)
	 * @brief Get snapshot in db
     *<dl>
     *<dt>Sequoiadb.SDB_SNAP_CONTEXTS   : Get all contexts' snapshot
     *<dt>Sequoiadb.SDB_SNAP_CONTEXTS_CURRENT        : Get the current context's snapshot
     *<dt>Sequoiadb.SDB_SNAP_SESSIONS        : Get all sessions' snapshot
     *<dt>Sequoiadb.SDB_SNAP_SESSIONS_CURRENT        : Get the current session's snapshot
     *<dt>Sequoiadb.SDB_SNAP_COLLECTIONS        : Get the collections' snapshot
     *<dt>Sequoiadb.SDB_SNAP_COLLECTIONSPACES        : Get the collection spaces' snapshot
     *<dt>Sequoiadb.SDB_SNAP_DATABASE        : Get database's snapshot
     *<dt>Sequoiadb.SDB_SNAP_SYSTEM        : Get system's snapshot
     *<dt>Sequoiadb.SDB_SNAP_CATALOG        : Get catalog's snapshot
     *<dt>Sequoiadb.SDB_LIST_SHARDS        : Get shard list ( only applicable in sharding env )
     *<dt>Sequoiadb.SDB_LIST_STOREPROCEDURES           : Get stored procedure list ( only applicable in sharding env )
     *</dl>
	 * @param matcher
	 *            BSONObject
	 * @param selector
	 *            BSONObject
	 * @param orderBy
	 *            BSONObject
	 * @return The DBCursor of snapshot.
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor getSnapshot(int snapType, BSONObject matcher,
			BSONObject selector, BSONObject orderBy) {
		String command = SequoiadbConstants.SNAP_CMD;
		switch (snapType) {
		case SDB_SNAP_CONTEXTS:
			command += SequoiadbConstants.CONTEXTS;
			break;
		case SDB_SNAP_CONTEXTS_CURRENT:
			command += SequoiadbConstants.CONTEXTS_CUR;
			break;
		case SDB_SNAP_SESSIONS:
			command += SequoiadbConstants.SESSIONS;
			break;
		case SDB_SNAP_SESSIONS_CURRENT:
			command += SequoiadbConstants.SESSIONS_CUR;
			break;
		case SDB_SNAP_COLLECTIONS:
			command += SequoiadbConstants.COLLECTIONS;
			break;
		case SDB_SNAP_COLLECTIONSPACES:
			command += SequoiadbConstants.COLSPACES;
			break;
		case SDB_SNAP_DATABASE:
			command += SequoiadbConstants.DATABASE;
			break;
		case SDB_SNAP_SYSTEM:
			command += SequoiadbConstants.SYSTEM;
			break;
		case SDB_SNAP_CATALOG:
			command += SequoiadbConstants.CATA;
			break;
		default:
			throw new BaseException("SDB_INVALIDARG");
		}

		SDBMessage rtn = adminCommand(command, 0, 0, -1, -1, matcher, selector,
				orderBy, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC) {
				return null;
			} else {
				throw new BaseException(flags, matcher, selector, orderBy);
			}
		}
		return new DBCursor(rtn, this);

	}
	
	/**
	 * @fn void beginTransaction()
	 * @brief Begin the transaction
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void beginTransaction() {
		byte[] request = SDBMessageHelper.buildTransactionRequest(SequoiadbConstants.Operation.TRANS_BEGIN_REQ, endianConvert);
		connection.sendMessage(request);
		
		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtn = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtn.getFlags();
		if(flags != 0)
			throw new BaseException(flags);
	}
	
	/**
	 * @fn void commit()
	 * @brief Commit the transaction
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void commit() {
		byte[] request = SDBMessageHelper.buildTransactionRequest(
				SequoiadbConstants.Operation.TRANS_COMMIT_REQ, endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtn = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtn.getFlags();
		if(flags != 0)
			throw new BaseException(flags);
	}
	
	/**
	 * @fn void rollback()
	 * @brief Rollback the transaction
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void rollback() {
		byte[] request = SDBMessageHelper.buildTransactionRequest(
				SequoiadbConstants.Operation.TRANS_ROLLBACK_REQ, endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtn = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtn.getFlags();
		if(flags != 0)
			throw new BaseException(flags);
	}

	/**
	 * @fn void crtJSProcedure ( String code )
     * @brief Create a store procedures.
     * @param code The code of store procedures
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void crtJSProcedure ( String code )
	{
		// check the argument
		if ( null == code || code.equals("") ){
			throw new BaseException("SDB_INVALIDARG");
		}
		// build code type bson
		BSONObject newobj = new BasicBSONObject();
		Code codeObj = new Code( code );
		newobj.put(SequoiadbConstants.FIELD_NAME_FUNC, codeObj);
		newobj.put(SequoiadbConstants.FMP_FUNC_TYPE, FMP_FUNC_TYPE_JS);
		
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_CRT_PROCEDURES,
				                      0,0,0,-1,newobj,
				                      null,null,null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags);
		}
	}
	
	/**
	 * @fn void rmProcedures ( String name )
     * @brief Remove a store procedures.
     * @param name The name of store procedure to be removed
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void rmProcedures ( String name )
	{
		// check the argument
		if ( null == name || name.equals("") ) {
			throw new BaseException("SDB_INVALIDARG");
		}
		// append the name to a bson
		BSONObject newobj = new BasicBSONObject();
		newobj.put( SequoiadbConstants.FIELD_NAME_FUNC, name );
		
		SDBMessage rtn = adminCommand( SequoiadbConstants.CMD_NAME_RM_PROCEDURES,
				                       0, 0, 0, -1, newobj,
				                       null, null, null );
		int flags = rtn.getFlags();
		if (flags != 0)
			throw new BaseException(flags);
	}
	
	/**
	 * @fn void listProcedures ( BSONObject condition )
     * @brief List the store procedures.
     * @param condition The condition of list eg: {"name":"sum"}
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor listProcedures ( BSONObject condition )
	{
		return getList(SDB_LIST_STOREPROCEDURES, 0, 0, 0,
				       -1, condition, null, null, null);
	}
	
	/**
	 * @fn void backupOffline ( BSONObject options )
     * @brief Backup the whole database or specifed shard.
     * @param options Contains a series of backup configuration infomations. 
     *        Backup the whole cluster if null. The "options" contains 5 options as below. 
     *        All the elements in options are optional. 
     *        eg: {"GroupName":["shardName1", "shardName2"], "Path":"/opt/sequoiadb/backup", 
     *             "Name":"backupName", "Description":description, "EnsureInc":true, "OverWrite":true}
     *<dl>
     *<dt>GroupName   : The shards which to be backuped
     *<dt>Path        : The backup path, if not assign, use the backup path assigned in configuration file
     *<dt>Name        : The name for the backup
     *<dt>Description : The description for the backup
     *<dt>EnsureInc   : Whether excute increment synchronization, default to be false
     *<dt>OverWrite   : Whether overwrite the old backup file, default to be false
     *</dl>
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void backupOffline ( BSONObject options )
	{
		// check the optional argument
		if ( null != options ){
			for (String key : options.keySet() ){
				if (key.equals(SequoiadbConstants.FIELD_NAME_GROUPNAME)
						|| key.equals(SequoiadbConstants.FIELD_NAME_NAME)
						|| key.equals(SequoiadbConstants.FIELD_NAME_PATH)
						|| key.equals(SequoiadbConstants.FIELD_NAME_DESP)
						|| key.equals(SequoiadbConstants.FIELD_NAME_ENSURE_INC)
						|| key.equals(SequoiadbConstants.FIELD_NAME_OVERWRITE)){
					continue ;
				}
				else{
					throw new BaseException("SDB_INVALIDARG");
				}
			}
		}
		
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_BACKUP_OFFLINE,
				                      0,0,0,-1,options,
				                      null,null,null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags);
		}
	}
	
	/**
	 * @fn DBCursor listBackup ( BSONObject options, BSONObject condition,
			                     BSONObject selector, BSONObject orderBy )
     * @brief List the backups.
     * @param options Contains configuration infomations for remove backups, list all the backups in the default backup path if null.
     *        The "options" contains 3 options as below. All the elements in options are optional. 
     *        eg: {"GroupName":["shardName1", "shardName2"], "Path":"/opt/sequoiadb/backup", "Name":"backupName"}
     * <dl>
     * <dt>GroupName   : Assign the backups of specifed shards to be list
     * <dt>Path        : Assign the backups in specifed path to be list, if not assign, use the backup path asigned in the configuration file
     * <dt>Name        : Assign the backups with specifed name to be list
     * </dl>
     * @param condition The matching rule, return all the documents if null
     * @param selector The selective rule, return the whole document if null
     * @param orderBy The ordered rule, never sort if null
     * @return the DBCursor of backup or null while having no backup infonation. 
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor listBackup ( BSONObject options, BSONObject condition,
			                     BSONObject selector, BSONObject orderBy)
	{
		// check the optional argument
		if ( null != options ){
			for ( String key : options.keySet() ){
				if ( key.equals(SequoiadbConstants.FIELD_NAME_GROUPNAME)
						|| key.equals(SequoiadbConstants.FIELD_NAME_NAME)
						|| key.equals(SequoiadbConstants.FIELD_NAME_PATH) ){
					continue ;
				}
				else{
					throw new BaseException("SDB_INVALIDARG");
				}
			}
		}
		
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_LIST_BACKUP,
				                      0,0,0,-1,condition,
				                      selector,orderBy,options);
		DBCursor cursor = null;
		int flags = rtn.getFlags();
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC) {
				return cursor;
			} else {
				throw new BaseException(flags, condition, selector, orderBy, options);
			}
		}
		cursor = new DBCursor(rtn, this);
		return cursor;
	}

	/**
	 * @fn void removeBackup ( BSONObject options )
     * @brief Remove the backups.
     * @param options Contains configuration infomations for remove backups, remove all the backups in the default backup path if null.
     *                The "options" contains 3 options as below. All the elements in options are optional.
     *                eg: {"GroupName":["shardName1", "shardName2"], "Path":"/opt/sequoiadb/backup", "Name":"backupName"}
     *<dl>
     *<dt>GroupName   : Assign the backups of specifed shards to be remove
     *<dt>Path        : Assign the backups in specifed path to be remove, if not assign, use the backup path asigned in the configuration file
     *<dt>Name        : Assign the backups with specifed name to be remove
     *</dl>
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void removeBackup ( BSONObject options )
	{
		// check the optional argument
		if ( null != options ){
			for ( String key : options.keySet() ){
				if ( key.equals(SequoiadbConstants.FIELD_NAME_GROUPNAME)
						|| key.equals(SequoiadbConstants.FIELD_NAME_NAME)
						|| key.equals(SequoiadbConstants.FIELD_NAME_PATH) ){
					continue ;
				}
				else{
					throw new BaseException("SDB_INVALIDARG");
				}
			}
		}
		
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_REMOVE_BACKUP,
				                      0,0,0,-1,options,
				                      null,null,null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags);
		}
	}
	
	/**
	 * @fn boolean isEndianConvert()
	 * @brief Judge the endian of the physical computer
	 * @return Big-Endian for true while Little-Endian for false
	 */
	public boolean isEndianConvert() {
		return endianConvert;
	}
	
	/**
	 * @fn DBCursor listReplicaGroups()
	 * @brief List all the replica group.
	 * @return cursor of all collecionspace names
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use listShards instead of it.
	 * @see listShards
	 */
	public DBCursor listReplicaGroups() throws BaseException {
		try{
			return listShards();
		}catch(BaseException e){
			throw e;
		}
	}
	
	/**
	 * @fn ArrayList<String> getReplicaGroupNames()
	 * @brief Get all the replica groups' name.
	 * @return A list of all the replica groups' names.
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use getShardNames instead of it.
	 * @see getShardNames
	 */
	public ArrayList<String> getReplicaGroupNames() throws BaseException {
		try{
			return getShardNames();
		}catch(BaseException e){
			throw e;
		}
	}
	
	/**
	 * @fn List<String> getReplicaGroupsInfo()
	 * @brief Get the infomations of the replica groups.
	 * @return A list of informations of the replica groups.
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use getShardsInfo instead of it.
	 * @see getShardsInfo   
	 */
	public ArrayList<String> getReplicaGroupsInfo() throws BaseException {
		try{
			return getShardsInfo();
		}catch(BaseException e){
			throw e;
		}
	}
	
	/**
	 * @fn ReplicaGroup getReplicaGroup(String rgName)
	 * @brief Get replica group by name.
	 * @param rgName
	 *            replica group's name
	 * @return A replica group object or null for not exit.
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use getShard instead of it.
	 * @see getShard  
	 */
	public ReplicaGroup getReplicaGroup(String rgName)
			throws BaseException {
		BSONObject rg = getDetailByName(rgName);
		if (rg == null)
			return null;
		return new ReplicaGroup(this, rgName);
	}

	/**
	 * @fn ReplicaGroup getReplicaGroup(int rgId)
	 * @brief Get shard by id.
	 * @param rgId
	 *            replica group id
	 * @return A replica group object or null for not exit.
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use getShard instead of it.
	 * @see getShard
	 */
	public ReplicaGroup getReplicaGroup(int rgId) throws BaseException{
		BSONObject rg = getDetailById(rgId);
		if (rg == null)
			return null;
		return new ReplicaGroup(this, rgId);
	}

	/**
	 * @fn ReplicaGroup createReplicaGroup(String rgName)
	 * @brief Create shard by name.
	 * @param rgName
	 *            replica group's name
	 * @return A replica group object.
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use createShard instead of it.
	 * @see createShard
	 */
	public ReplicaGroup createReplicaGroup(String rgName)
			throws BaseException {
		BSONObject rg = new BasicBSONObject();
		rg.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, rgName);
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_CREATE_GROUP,
				0, 0, -1, -1, rg, null, null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, rgName);
		}
		return getReplicaGroup(rgName);
	}

	/**
	 * @fn void removeReplicaGroup(String rgName)
	 * @brief Remove replica group by name.
	 * @param rgName
	 *            replica group's name
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use removeShard instead of it.
	 * @see removeShard
	 */
	public void removeReplicaGroup(String rgName)
			throws BaseException {
		BSONObject rg = new BasicBSONObject();
		rg.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, rgName);
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_REMOVE_GROUP,
				0, 0, -1, -1, rg, null, null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, rgName);
		}
	}

	/**
	 * @fn void activateReplicaGroup(String rgName)
	 * @brief Active shard by name.
	 * @param rgName
	 *            replica group name
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use activateShard instead of it.
	 * @see activateShard
	 */
	public void activateReplicaGroup(String rgName)
			throws BaseException {
		BSONObject rg = new BasicBSONObject();
		rg.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, rgName);
		SDBMessage rtn = adminCommand(SequoiadbConstants.CMD_NAME_ACTIVE_GROUP,
				0, 0, -1, -1, rg, null, null, null);
		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, rgName);
		}
	}

	/**
	 * @fn void createReplicaCataGroup(String hostName, int port, String dbPath,
	 *     BSONObject configuration)
	 * @brief Create the replica Catalog group with the given options.
	 * @param hostName
	 *            The host name
	 * @param port
	 *            The port
	 * @param dbpath
	 *            The database path
	 * @param configure
	 *            The configure options
	 * @exception com.sequoiadb.exception.BaseException
	 * @deprecated This function will be deprecated in version 2.x, 
	 *             use createCataShard instead of it.
	 * @see createCataShard
	 */
	public void createReplicaCataGroup(String hostName, int port,
			String dbPath, Map<String, String> configure) {
		try{
			createCataShard(hostName, port, dbPath, configure);
		}catch(BaseException e){
			throw e;
		}
	}
	
	
	
	DBCursor getList(int listType, int flag, long reqID, long skipNum,
			long returnNum, BSONObject query, BSONObject selector,
			BSONObject order, BSONObject hint) throws BaseException {
		String command = "";
		switch (listType) {
		case SDB_LIST_CONTEXTS:
			command = SequoiadbConstants.CMD_NAME_LIST_CONTEXTS;
			break;
		case SDB_LIST_CONTEXTS_CURRENT:
			command = SequoiadbConstants.CMD_NAME_LIST_CONTEXTS_CURRENT;
			break;
		case SDB_LIST_SESSIONS:
			command = SequoiadbConstants.CMD_NAME_LIST_SESSIONS;
			break;
		case SDB_LIST_SESSIONS_CURRENT:
			command = SequoiadbConstants.CMD_NAME_LIST_SESSIONS_CURRENT;
			break;
		case SDB_LIST_COLLECTIONS:
			command = SequoiadbConstants.CMD_NAME_LIST_COLLECTIONS;
			break;
		case SDB_LIST_COLLECTIONSPACES:
			command = SequoiadbConstants.CMD_NAME_LIST_COLLECTIONSPACES;
			break;
		case SDB_LIST_STORAGEUNITS:
			command = SequoiadbConstants.CMD_NAME_LIST_STORAGEUNITS;
			break;
		case SDB_LIST_SHARDS:
			command = SequoiadbConstants.CMD_NAME_LIST_GROUPS;
			break;
		case SDB_LIST_STOREPROCEDURES:
			command = SequoiadbConstants.CMD_NAME_LIST_PROCEDURES;
			break;
		default:
			throw new BaseException("SDB_INVALIDARG");
		}

		SDBMessage rtn = adminCommand(command, flag, reqID, skipNum, returnNum,
				query, selector, order, hint);
		int flags = rtn.getFlags();
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC) {
				return null;
			} else {
				throw new BaseException(flags, query, selector, order, hint);
			}
		}
		return new DBCursor(rtn, this);

	}

	String getUserName() {
		return userName;
	}

	String getPassword() {
		return password;
	}

	BSONObject getDetailByName(String name) throws BaseException {
		BSONObject condition = new BasicBSONObject();
		condition.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, name);
		DBCursor shardsCursor = getList(Sequoiadb.SDB_LIST_SHARDS, 0, 0, -1,
				-1, condition, null, null, null);
		if (shardsCursor == null || !shardsCursor.hasNext())
			return null;
		return shardsCursor.getNext();
	}

	BSONObject getDetailById(int id) throws BaseException {
		BSONObject condition = new BasicBSONObject();
		condition.put(SequoiadbConstants.FIELD_NAME_GROUPID, id);
		DBCursor shardsCursor = getList(Sequoiadb.SDB_LIST_SHARDS, 0, 0, -1,
				-1, condition, null, null, null);
		if (shardsCursor == null || !shardsCursor.hasNext())
			return null;
		return shardsCursor.getNext();
	}

	private void initConnection() throws BaseException {
		ConfigOptions options = new ConfigOptions();
		connection = new ConnectionTCPImpl(serverAddress, options);
		connection.initialize();
	}

	private SDBMessage adminCommand(String commandString, int flag, long reqID,
			long skipNum, long returnNum, BSONObject query,
			BSONObject selector, BSONObject order, BSONObject hint)
			throws BaseException {
		// Admin command request
		// int reqId = 0;
		BSONObject dummyObj = new BasicBSONObject();
		SDBMessage sdbMessage = new SDBMessage();

		if (query == null)
			sdbMessage.setMatcher(dummyObj);
		else
			sdbMessage.setMatcher(query);
		if (selector == null)
			sdbMessage.setSelector(dummyObj);
		else
			sdbMessage.setSelector(selector);
		if (order == null)
			sdbMessage.setOrderBy(dummyObj);
		else
			sdbMessage.setOrderBy(order);
		if (hint == null)
			sdbMessage.setHint(dummyObj);
		else
			sdbMessage.setHint(hint);

		sdbMessage.setCollectionFullName(SequoiadbConstants.ADMIN_PROMPT
				+ commandString);

		sdbMessage.setVersion(1);
		sdbMessage.setW((short) 0);
		sdbMessage.setPadding((short) 0);
		sdbMessage.setFlags(flag);
		sdbMessage.setNodeID(SequoiadbConstants.ZERO_NODEID);
		// sdbMessage.setResponseTo(reqId);
		// reqId++;
		sdbMessage.setRequestID(reqID);
		sdbMessage.setSkipRowsCount(skipNum);
		sdbMessage.setReturnRowsCount(returnNum);

		byte[] request = SDBMessageHelper.buildQueryRequest(sdbMessage,
				endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);

		return rtnSDBMessage;
	}

	private SDBMessage createCS(String csName, int pageSize)
			throws BaseException {
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.CREATE_CMD + " "
				+ SequoiadbConstants.COLSPACE;
		BSONObject cObj = new BasicBSONObject();
		BSONObject dummyObj = new BasicBSONObject();
		SDBMessage sdbMessage = new SDBMessage();

		cObj.put(SequoiadbConstants.FIELD_NAME_NAME, csName);
		cObj.put(SequoiadbConstants.FIELD_NAME_PAGE_SIZE, pageSize);
		sdbMessage.setMatcher(cObj);
		sdbMessage.setCollectionFullName(commandString);

		sdbMessage.setVersion(0);
		sdbMessage.setW((short) 0);
		sdbMessage.setPadding((short) 0);
		sdbMessage.setFlags(0);
		sdbMessage.setNodeID(SequoiadbConstants.ZERO_NODEID);
		sdbMessage.setRequestID(0);
		sdbMessage.setSkipRowsCount(-1);
		sdbMessage.setReturnRowsCount(-1);
		sdbMessage.setSelector(dummyObj);
		sdbMessage.setOrderBy(dummyObj);
		sdbMessage.setHint(dummyObj);

		byte[] request = SDBMessageHelper.buildQueryRequest(sdbMessage,
				endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		if (endianConvert) {
			byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
		} else {
			byteBuffer.order(ByteOrder.BIG_ENDIAN);
		}
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);

		return rtnSDBMessage;
	}
	
	private boolean requestSysInfo() {
		byte[] request = SDBMessageHelper.buildSysInfoRequest();
		connection.sendMessage(request);
		boolean endianConvert = SDBMessageHelper
				.msgExtractSysInfoReply(connection.receiveSysInfoMsg(128));
		return endianConvert;
	}

}
