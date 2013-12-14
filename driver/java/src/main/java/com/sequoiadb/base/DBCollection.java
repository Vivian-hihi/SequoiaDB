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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

import org.apache.mina.core.buffer.IoBuffer;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.net.IConnection;
import com.sequoiadb.util.SDBMessageHelper;

/**
 * @class DBCollection
 * @brief Database operation interfaces of collection
 */
public class DBCollection {
	private String name;
	private Sequoiadb sequoiadb;
	private CollectionSpace collectionSpace;
	private IConnection connection;
	private String csName;
	private String collectionFullName;

	private IoBuffer bulk_buffer = null;
	private IoBuffer insert_buffer = null;
	private static final int DEF_BUFFER_LENGTH = 2 * 1024 * 1024;


	public IConnection getConnection() {
		return connection;
	}
	
	public void setConnection(IConnection connection) {
		this.connection = connection;
	}

	/**
	 * @memberof FLG_INSERT_CONTONDUP 0x00000001
	 * @brief The flags represent whether bulk insert continue when hitting
	 *        index key duplicate error
	 */
	public final static int FLG_INSERT_CONTONDUP = 0x00000001;

	/**
	 * @fn String getName()
	 * @brief Return the name of current collection
	 * @return The collection name
	 */
	public String getName() {
		return name;
	}

	/**
	 * @fn String getFullName()
	 * @brief Get the full name of specified collection in current collection
	 *        space
	 * @return The full name of specified collection
	 */
	public String getFullName() {
		return collectionFullName;
	}

	/**
	 * @fn String getCSName()
	 * @brief Get the full name of specified collection in current collection
	 *        space
	 * @return The full name of specified collection
	 */
	public String getCSName() {
		return csName;
	}

	/**
	 * @fn Sequoiadb getSequoiadb()
	 * @brief Return the Sequoiadb handle of current collection
	 * @return Sequoiadb object
	 */
	public Sequoiadb getSequoiadb() {
		return sequoiadb;
	}

	/**
	 * @fn CollectionSpace getCollectionSpace()
	 * @brief Return the Collection Space handle of current collection
	 * @return CollectionSpace object
	 */
	public CollectionSpace getCollectionSpace() {
		return collectionSpace;
	}

	/**
	 * @fn DBCollection(Sequoiadb sequoiadb, CollectionSpace cs, String name)
	 * @brief Constructor
	 * @param sequoiadb
	 *            Sequoiadb handle
	 * @param cs
	 *            CollectionSpace handle
	 * @param name
	 *            Collection name
	 */
	DBCollection(Sequoiadb sequoiadb, CollectionSpace cs, String name) {
		this.name = name;
		this.sequoiadb = sequoiadb;
		this.collectionSpace = cs;
		this.csName = cs.getName();
		this.collectionFullName = csName + "." + name;
		this.connection = sequoiadb.getConnection();
	}

	/**
	 * @fn ObjectId insert(BSONObject obj)
	 * @brief Insert a BSONObject into current collection
	 * @param insertor
	 *            The Bson object of insertor, can't be null
	 * @return ObjectId
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public ObjectId insert(BSONObject insertor) throws BaseException {
		if (insertor == null)
			throw new BaseException("SDB_INVALIDARG");
		
		if (this.insert_buffer == null) {
			this.insert_buffer = IoBuffer.allocate(DEF_BUFFER_LENGTH);
			this.insert_buffer.setAutoExpand(true);
			if (sequoiadb.endianConvert) {
				insert_buffer.order(ByteOrder.LITTLE_ENDIAN);
			} else {
				insert_buffer.order(ByteOrder.BIG_ENDIAN);
			}
		} else {
			insert_buffer.position(0);
		}
		
		
		Object tmp = insertor.get(SequoiadbConstants.OID);
		ObjectId objId;
		if (tmp != null) {
			objId = (ObjectId) tmp;
		} else {
			objId = ObjectId.get();
			insertor.put(SequoiadbConstants.OID, objId);
		}
		
		int message_length = SDBMessageHelper.buildInsertRequest(insert_buffer, collectionFullName, insertor);
		connection.sendMessage(insert_buffer.array(), message_length);
		
		ByteBuffer byteBuffer = connection.receiveMessage(sequoiadb.endianConvert);
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, insertor.toString());
		}
		return objId;
	}

	/**
	 * @fn ObjectId insert(String insertor)
	 * @brief Insert a string of BSONObject into current collection
	 * @param insertor
	 *            The string of insertor
	 * @return ObjectId
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public ObjectId insert(String insertor) throws BaseException {
		BSONObject in = null;
		if (insertor != null)
			in = (BSONObject) JSON.parse(insertor);
		return insert(in);
	}

	/**
	 * @fn <T> void save(T type)
	 * @brief Insert an object into current collection
	 * @param type
	 *            The object of insertor, can't be null
	 * @exception com.sequoiadb.exception.BaseException
	 * @note when save include update shardingKey field, the shardingKey modify action is not take effect, but the other
	 *       field update is take effect.
	 *       Because of current version is not support update shardingKey field.
	 */
	public <T> void save(T type) throws BaseException {
		BSONObject obj;
		try {
			obj = BasicBSONObject.typeToBson(type);
		} catch (Exception e) {
			throw new BaseException(e.toString());
		}

		Object id = obj.get(SequoiadbConstants.OID);
		if (id == null || (id instanceof ObjectId && ((ObjectId) id).isNew())) {
			if (id != null && id instanceof ObjectId)
				((ObjectId) id).notNew();

			insert(obj);
		} else {
			BSONObject matcher = new BasicBSONObject();
			matcher.put(SequoiadbConstants.OID, id);
			BSONObject modifer = new BasicBSONObject();
			modifer.put("$set", obj);

			upsert(matcher, modifer, null);
		}
	}

	/**
	 * @fn void bulkInsert(List<BSONObject> insertor, int flag)
	 * @brief Insert a bulk of bson objects into current collection
	 * @param insertor
	 *            The Bson object of insertor list, can't be null
	 * @param flag
	 *            FLG_INSERT_CONTONDUP or 0
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void bulkInsert(List<BSONObject> insertor, int flag)
			throws BaseException {

		if (flag != 0 && flag != FLG_INSERT_CONTONDUP)
			throw new BaseException("SDB_INVALIDARG");
		if (insertor == null || insertor.size() == 0)
			throw new BaseException("SDB_INVALIDARG");

		if (this.bulk_buffer == null) {
			this.bulk_buffer = IoBuffer.allocate(DEF_BUFFER_LENGTH);
			this.bulk_buffer.setAutoExpand(true);
			if (sequoiadb.endianConvert) {
				bulk_buffer.order(ByteOrder.LITTLE_ENDIAN);
			} else {
				bulk_buffer.order(ByteOrder.BIG_ENDIAN);
			}
		} else {
			bulk_buffer.position(0);
		}

		int messageLength = SDBMessageHelper.buildBulkInsertRequest(
				bulk_buffer, collectionFullName, insertor, flag);

		connection.sendMessage(bulk_buffer.array(), messageLength);
		
		ByteBuffer byteBuffer = connection.receiveMessage(sequoiadb.endianConvert);
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0)
			throw new BaseException(flags, insertor);

	}

	/**
	 * @fn void delete(BSONObject matcher)
	 * @brief Delete the matching BSONObject of current collection
	 * @param matcher
	 *            The matching condition
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void delete(BSONObject matcher) throws BaseException {
		delete(matcher, null);
	}

	/**
	 * @fn void delete(String matcher)
	 * @brief Delete the matching of current collection
	 * @param matcher
	 *            The matching condition
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void delete(String matcher) throws BaseException {
		BSONObject ma = null;
		if (matcher != null)
			ma = (BSONObject) JSON.parse(matcher);
		delete(ma, null);
	}

	/**
	 * @fn void delete(String matcher, String hint)
	 * @brief Delete the matching bson's string of current collection
	 * @param matcher
	 *            The matching condition
	 * @param hint
	 *            Hint
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void delete(String matcher, String hint) {
		BSONObject ma = null;
		BSONObject hi = null;
		if (matcher != null)
			ma = (BSONObject) JSON.parse(matcher);
		if (hint != null)
			hi = (BSONObject) JSON.parse(hint);
		delete(ma, hi);
	}

	/**
	 * @fn void delete(BSONObject matcher, BSONObject hint)
	 * @brief Delete the matching BSONObject of current collection
	 * @param matcher
	 *            The matching condition
	 * @param hint
	 *            Hint
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void delete(BSONObject matcher, BSONObject hint)
			throws BaseException {
		BSONObject dummy = new BasicBSONObject();
		if (matcher == null)
			matcher = dummy;
		if (hint == null)
			hint = dummy;
		// Delete
		// long reqId = 0;
		SDBMessage sdbMessage = new SDBMessage();

		sdbMessage.setVersion(0);
		sdbMessage.setW((short) 0);
		sdbMessage.setPadding((short) 0);
		sdbMessage.setFlags(0);
		sdbMessage.setNodeID(SequoiadbConstants.ZERO_NODEID);
		// sdbMessage.setResponseTo(reqId);
		// reqId++;
		sdbMessage.setCollectionFullName(collectionFullName);
		sdbMessage.setRequestID(0);
		sdbMessage.setMatcher(matcher);
		sdbMessage.setHint(hint);

		byte[] request = SDBMessageHelper.buildDeleteRequest(sdbMessage,
				sequoiadb.endianConvert);
		connection.sendMessage(request);
		
		ByteBuffer byteBuffer = connection.receiveMessage(sequoiadb.endianConvert);
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, matcher, hint);
		}
	}

	/**
	 * @fn void update(DBQuery query)
	 * @brief Update the document of current collection
	 * @param query
	 *            DBQuery with matching condition, updating rule and hint
	 * @exception com.sequoiadb.exception.BaseException
	 * @note when save include update shardingKey field, the shardingKey modify action is not take effect, but the other
	 *       field update is take effect.
	 *       Because of current version is not support update shardingKey field.
	 */
	public void update(DBQuery query) throws BaseException {
		_update(0, query.getMatcher(), query.getModifier(), query.getHint());
	}

	/**
	 * @fn void update(BSONObject matcher, BSONObject modifier, BSONObject hint)
	 * @brief Update the BSONObject of current collection
	 * @param matcher
	 *            The matching condition
	 * @param modifier
	 *            The updating rule
	 * @param hint
	 *            Hint
	 * @exception com.sequoiadb.exception.BaseException
	 * @note when save include update shardingKey field, the shardingKey modify action is not take effect, but the other
	 *       field update is take effect.
	 *       Because of current version is not support update shardingKey field.
	 */
	public void update(BSONObject matcher, BSONObject modifier, BSONObject hint)
			throws BaseException {
		_update(0, matcher, modifier, hint);
	}

	/**
	 * @fn void update(String matcher, String modifier, String hint)
	 * @brief Update the BSONObject of current collection
	 * @param matcher
	 *            The matching condition
	 * @param modifier
	 *            The updating rule
	 * @param hint
	 *            Hint
	 * @exception com.sequoiadb.exception.BaseException
	 * @note when save include update shardingKey field, the shardingKey modify action is not take effect, but the other
	 *       field update is take effect.
	 *       Because of current version is not support update shardingKey field.
	 */
	public void update(String matcher, String modifier, String hint)
			throws BaseException {
		BSONObject ma = null;
		BSONObject mo = null;
		BSONObject hi = null;
		if (matcher != null)
			ma = (BSONObject) JSON.parse(matcher);
		if (modifier != null)
			mo = (BSONObject) JSON.parse(modifier);
		if (hint != null)
			hi = (BSONObject) JSON.parse(hint);
		_update(0, ma, mo, hi);
	}

	/**
	 * @fn void upsert(BSONObject matcher, BSONObject modifier, BSONObject hint)
	 * @brief Update the BSONObject of current collection, insert if no matching
	 * @param matcher
	 *            The matching condition
	 * @param modifier
	 *            The updating rule
	 * @param hint
	 *            Hint
	 * @exception com.sequoiadb.exception.BaseException
	 * @note when save include update shardingKey field, the shardingKey modify action is not take effect, but the other
	 *       field update is take effect.
	 *       Because of current version is not support update shardingKey field.
	 */
	public void upsert(BSONObject matcher, BSONObject modifier, BSONObject hint)
			throws BaseException {
		_update(SequoiadbConstants.FLG_UPDATE_UPSERT, matcher, modifier, hint);
	}

	/**
	 * @fn DBCursor query()
	 * @brief Query all datas of current collection
	 * @return DBCursor in current collection
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query() {
		return query("", "", "", "", 0, -1);
	}

	/**
	 * @fn DBCursor query(DBQuery query)
	 * @brief Find datas of current collection with DBQuery
	 * @param query
	 *            DBQuery with matching condition, selector, order rule, hint,
	 *            skipRowsCount and returnRowsCount
	 * @return DBCursor of the datas
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query(DBQuery query) throws BaseException {
		if (query == null)
			return query();
		return query(query.getMatcher(), query.getSelector(),
				query.getOrderBy(), query.getHint(), query.getSkipRowsCount(),
				query.getReturnRowsCount(), 0);
	}

	/**
	 * @fn DBCursor query(BSONObject query, BSONObject selector, BSONObject
	 *     orderBy, BSONObject hint)
	 * @brief Find datas of current collection
	 * @param query
	 *            The matching condition
	 * @param selector
	 *            The selective rule
	 * @param orderBy
	 *            The ordered rule
	 * @param hint
	 *            Hint
	 * @return DBCursor of datas
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query(BSONObject query, BSONObject selector,
			BSONObject orderBy, BSONObject hint) throws BaseException {
		return query(query, selector, orderBy, hint, 0, -1, 0);
	}

	/**
	 * @fn DBCursor query(BSONObject query, BSONObject selector, BSONObject
	 *     orderBy, BSONObject hint)
	 * @brief Find datas of current collection
	 * @param query
	 *            The matching condition
	 * @param selector
	 *            The selective rule
	 * @param orderBy
	 *            The ordered rule
	 * @param hint
	 *            Hint
	 * @return DBCursor of datas
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query(BSONObject query, BSONObject selector,
			BSONObject orderBy, BSONObject hint, int flag) throws BaseException {
		return query(query, selector, orderBy, hint, 0, -1, flag);
	}
	
	/**
	 * @fn DBCursor query(String query, String selector, String orderBy, String
	 *     hint)
	 * @brief Find datas of current collection
	 * @param query
	 *            The matching condition
	 * @param selector
	 *            The selective rule
	 * @param orderBy
	 *            The ordered rule
	 * @param hint
	 *            Hint
	 * @param flag
	 *            The flag 0: bson, 1:raw byte
	 * @return DBCursor of datas
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query(String query, String selector, String orderBy,
			String hint) throws BaseException {
		return query(query, selector, orderBy, hint, 0);
	}
	/**
	 * @fn DBCursor query(String query, String selector, String orderBy, String
	 *     hint)
	 * @brief Find datas of current collection
	 * @param query
	 *            The matching condition
	 * @param selector
	 *            The selective rule
	 * @param orderBy
	 *            The ordered rule
	 * @param hint
	 *            Hint
	 * @return DBCursor of datas
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query(String query, String selector, String orderBy,
			String hint, int flag) throws BaseException {
		BSONObject qu = null;
		BSONObject se = null;
		BSONObject or = null;
		BSONObject hi = null;
		if (query != null)
			qu = (BSONObject) JSON.parse(query);
		if (selector != null)
			se = (BSONObject) JSON.parse(selector);
		if (orderBy != null && !orderBy.equals(""))
			or = (BSONObject) JSON.parse(orderBy);
		if (hint != null)
			hi = (BSONObject) JSON.parse(hint);
		return query(qu, se, or, hi, 0, -1, flag);
	}

	/**
	 * @fn DBCursor query(String query, String selector, String orderBy,
			String hint, long skipRows, long returnRows)
	 * @brief Find datas of current collection
	 * @param query
	 *            The matching condition
	 * @param selector
	 *            The selective rule
	 * @param orderBy
	 *            The ordered rule
	 * @param hint
	 *            Hint
	 * @param skipRows
	 *            The rows to be skipped
	 * @param returnRows
	 *            The rows to return
	 * @return DBCursor of datas
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query(String query, String selector, String orderBy,
			String hint, long skipRows, long returnRows) throws BaseException {
		BSONObject qu = null;
		BSONObject se = null;
		BSONObject or = null;
		BSONObject hi = null;
		if (query != null)
			qu = (BSONObject) JSON.parse(query);
		if (selector != null)
			se = (BSONObject) JSON.parse(selector);
		if (orderBy != null)
			or = (BSONObject) JSON.parse(orderBy);
		if (hint != null)
			hi = (BSONObject) JSON.parse(hint);
		return query(qu, se, or, hi, skipRows, returnRows, 0);
	}
	
	
	/**
	 * @fn DBCursor query(BSONObject query, BSONObject selector, BSONObject
	 *     orderBy, BSONObject hint, long skipRows, long returnRows)
	 * @brief Find datas of current collection
	 * @param query
	 *            The matching condition
	 * @param selector
	 *            The selective rule
	 * @param orderBy
	 *            The ordered rule
	 * @param hint
	 *            Hint
	 * @param skipRows
	 *            The rows to be skipped
	 * @param returnRows
	 *            The rows to return
	 * @return DBCursor of datas
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query(BSONObject query, BSONObject selector,
			BSONObject orderBy, BSONObject hint, long skipRows, long returnRows) {
		return query(query, selector, orderBy, hint, skipRows, returnRows, 0);
	}

	/**
	 * @fn DBCursor query(BSONObject query, BSONObject selector, BSONObject
	 *     orderBy, BSONObject hint, long skipRows, long returnRows)
	 * @brief Find datas of current collection
	 * @param query
	 *            The matching condition
	 * @param selector
	 *            The selective rule
	 * @param orderBy
	 *            The ordered rule
	 * @param hint
	 *            Hint
	 * @param skipRows
	 *            The rows to be skipped
	 * @param returnRows
	 *            The rows to return
	 * @param flag
	 *            The flag to use which form for record data
	 *            0: bson stream
	 *            1: binary data stream, form: col1|col2|col3
	 * @return DBCursor of datas or null
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor query(BSONObject query, BSONObject selector,
			BSONObject orderBy, BSONObject hint, long skipRows, long returnRows, int flag) {
		BSONObject dummy = new BasicBSONObject();
		if (query == null)
			query = dummy;
		if (selector == null)
			selector = dummy;
		if (orderBy == null)
			orderBy = dummy;
		if (hint == null)
			hint = dummy;
		if (returnRows == 0)
			returnRows = -1;
		SDBMessage rtnSDBMessage = adminCommand(collectionFullName, query,
				selector, orderBy, hint, skipRows, returnRows, flag);
		DBCursor cursor = null;
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC) {
				return cursor;
			} else {
				throw new BaseException(flags, query, selector, orderBy, hint,
						skipRows, returnRows);
			}
		}
		cursor = new DBCursor(rtnSDBMessage, this);
		return cursor;
	}

	/**
	 * @fn DBCursor getIndexes()
	 * @brief Get all the indexes of current collection
	 * @return DBCursor of indexes
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor getIndexes() throws BaseException {
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.GET_INXES;
		BSONObject dummyObj = new BasicBSONObject();
		BSONObject obj = new BasicBSONObject();
		obj.put(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);

		SDBMessage rtn = adminCommand(commandString, dummyObj, dummyObj,
				dummyObj, obj, -1, -1, 0);
		int flags = rtn.getFlags();
		DBCursor cursor = null;
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC) {
				return cursor;
			} else {
				throw new BaseException(flags);
			}
		}
		cursor = new DBCursor(rtn, sequoiadb);
		return cursor;
	}

	/**
	 * @fn DBCursor getIndex(String name)
	 * @brief Get all of or one of the indexes in current collection
	 * @param name
	 *            The index name, returns all of the indexes if this parameter
	 *            is null
	 * @return dbCursor of indexes
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor getIndex(String name) throws BaseException {
		if (name == null)
			return getIndexes();
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.GET_INXES;
		BSONObject dummyObj = new BasicBSONObject();
		BSONObject condition = new BasicBSONObject();
		BSONObject obj = new BasicBSONObject();
		obj.put(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
		condition.put(SequoiadbConstants.IXM_INDEXDEF + "."
				+ SequoiadbConstants.IXM_NAME, name);

		SDBMessage rtn = adminCommand(commandString, condition, dummyObj,
				dummyObj, obj, -1, -1, 0);
		int flags = rtn.getFlags();
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC) {
				return null;
			} else {
				throw new BaseException(flags);
			}
		}
		return new DBCursor(rtn, this);
	}

	/**
	 * @fn void createIndex(String name, BSONObject key, boolean isUnique,
	 *     boolean enforced)
	 * @brief Create a index with name and key
	 * @param name
	 *            The index name
	 * @param key
	 *            The index key, like: {"key":1/-1}, ASC(1)/DESC(-1)
	 * @param isUnique
	 *            Whether the index elements are unique or not
	 * @param enforced
	 *            Whether the index is enforced unique This element is
	 *            meaningful when isUnique is set to true
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void createIndex(String name, BSONObject key, boolean isUnique,
			boolean enforced) throws BaseException {
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.CREATE_INX;
		BSONObject obj = new BasicBSONObject();
		BSONObject dummyObj = new BasicBSONObject();
		BSONObject createObj = new BasicBSONObject();
		obj.put(SequoiadbConstants.IXM_KEY, key);
		obj.put(SequoiadbConstants.IXM_NAME, name);
		obj.put(SequoiadbConstants.IXM_UNIQUE, isUnique);
		obj.put(SequoiadbConstants.IXM_ENFORCED, enforced);
		createObj.put(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
		createObj.put(SequoiadbConstants.FIELD_INDEX, obj);

		SDBMessage rtn = adminCommand(commandString, createObj, dummyObj,
				dummyObj, dummyObj, -1, -1, 0);

		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, name, key, isUnique);
		}
	}

	/**
	 * @fn void createIndex(String name, String key, boolean isUnique, boolean
	 *     enforced)
	 * @brief Create a index with name and key
	 * @param name
	 *            The index name
	 * @param key
	 *            The index key, like: {"key":1/-1}, ASC(1)/DESC(-1)
	 * @param isUnique
	 *            Whether the index elements are unique or not
	 * @param enforced
	 *            Whether the index is enforced unique This element is
	 *            meaningful when isUnique is set to true
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void createIndex(String name, String key, boolean isUnique,
			boolean enforced) throws BaseException {
		BSONObject k = null;
		if (key != null)
			k = (BSONObject) JSON.parse(key);
		createIndex(name, k, isUnique, enforced);
	}

	/**
	 * @fn void dropIndex(String name)
	 * @brief Remove the named index of current collection
	 * @param name
	 *            The index name
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void dropIndex(String name) throws BaseException {
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.DROP_INX;
		BSONObject dummyObj = new BasicBSONObject();
		BSONObject dropObj = new BasicBSONObject();
		BSONObject index = new BasicBSONObject();
		index.put("", name);
		dropObj.put(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
		dropObj.put(SequoiadbConstants.FIELD_INDEX, index);

		SDBMessage rtn = adminCommand(commandString, dropObj, dummyObj,
				dummyObj, dummyObj, -1, -1, 0);

		int flags = rtn.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, name);
		}
	}
	
	/**
	 * @fn long getCount(String condition)
	 * @brief Get the count of records in current collection.
	 * @return The count of matching BSONObjects
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public long getCount() throws BaseException {
		return getCount("");
	}
	
	/**
	 * @fn long getCount(String condition)
	 * @brief Get the count of matching in current collection
	 * @param condition
	 *            The matching rule
	 * @return The count of matching BSONObjects
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public long getCount(String condition) throws BaseException {
		BSONObject con = null;
		if (condition != null)
			con = (BSONObject) JSON.parse(condition);
		return getCount(con);
	}
	
	/**
	 * @fn long getCount(BSONObject condition)
	 * @brief Get the count of matching BSONObject in current collection
	 * @param condition
	 *            The matching rule
	 * @return The count of matching BSONObjects
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public long getCount(BSONObject condition) throws BaseException {
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.GET_COUNT;
		BSONObject dummyObj = new BasicBSONObject();
		BSONObject newobj = new BasicBSONObject();
		newobj.put(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
		SDBMessage rtnSDBMessage = adminCommand(commandString, condition,
				dummyObj, dummyObj, newobj, -1, -1, 0);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0)
			throw new BaseException(flags, condition);

		List<BSONObject> rtn = getMoreCommand(rtnSDBMessage);
		return Long.valueOf(rtn.get(0).get(SequoiadbConstants.FIELD_TOTAL)
				.toString());
	}
	
	/**
	 * @fn long getCount(BSONObject condition)
	 * @brief Get the count of matching BSONObject in current collection
	 * @param condition
	 *            The matching rule
	 * @param hint
	 *            The hint, automatically match the optimal hint if null
	 * @return The count of matching BSONObjects
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public long getCount(BSONObject condition, BSONObject hint) throws BaseException {
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.GET_COUNT;
		BSONObject dummyObj = new BasicBSONObject();
		BSONObject newobj = new BasicBSONObject();
		newobj.put(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
		if (null != hint){
			try{
				newobj.put(SequoiadbConstants.FIELD_NAME_HINT, hint);
			}catch(Exception e){
				throw new BaseException("SDB_SYS", hint);
			}
		}
		SDBMessage rtnSDBMessage = adminCommand(commandString, condition,
				dummyObj, dummyObj, newobj, -1, -1, 0);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0)
			throw new BaseException(flags, condition, hint);

		List<BSONObject> rtn = getMoreCommand(rtnSDBMessage);
		return Long.valueOf(rtn.get(0).get(SequoiadbConstants.FIELD_TOTAL)
				.toString());
	}

	/*
	 * @fn void rename(String newName)
	 * @brief rename the current collection by new name
	 * @param newName
	 *            The new name of current DBCollection
	 * @exception com.sequoiadb.exception.BaseException
	 */
	/*
	public void rename(String newName) throws BaseException {
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.RENAME_COLLECTION;
		BSONObject dummyObj = new BasicBSONObject();
		BSONObject obj = new BasicBSONObject();
		obj.put(SequoiadbConstants.FIELD_NAME_COLLECTIONSPACE, csName);
		obj.put(SequoiadbConstants.FIELD_NAME_OLDNAME, this.name);
		obj.put(SequoiadbConstants.FIELD_NAME_NEWNAME, newName);
		SDBMessage rtnSDBMessage = adminCommand(commandString, obj, dummyObj,
				dummyObj, dummyObj, -1, -1);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, newName);
		}
		this.name = newName;
		this.collectionFullName = csName + "." + this.name;
	}*/

	/**
	 * @fn void split(String sourceGroupName, String destGroupName,
	 * 				 BSONObject splitCondition, BSONObject splitEndCondition)
	 * @brief Split the specified collection from source group to target group by range.
	 * @param sourceGroupName
	 *            the source group name
	 * @param destGroupName
	 *            the destination group name
     * @param splitCondition
	 *            the split condition
     * @param splitEndCondition
	 *            the split end condition or null
	 *            eg:If we create a collection with the option {ShardingKey:{"age":1},ShardingType:"Hash",Partition:2^10},
     *				 we can fill {age:30} as the splitCondition, and fill {age:60} as the splitEndCondition. when split, 
     *			 	 the targe group will get the records whose age's hash value are in [30,60). If splitEndCondition is null,
     *			 	 they are in [30,max).
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void split(String sourceGroupName, String destGroupName,
			BSONObject splitCondition, BSONObject splitEndCondition) {
		// check arguments
		if((null == sourceGroupName || sourceGroupName.equals("")) ||
		   (null == destGroupName || destGroupName.equals("")) ||
		    null == splitCondition){
			 throw new BaseException("SDB_INVALIDARG");
		  }
		BSONObject obj = new BasicBSONObject();
		obj.put(SequoiadbConstants.FIELD_NAME_NAME, collectionFullName);
		obj.put(SequoiadbConstants.FIELD_NAME_SOURCE, sourceGroupName);
		obj.put(SequoiadbConstants.FIELD_NAME_TARGET, destGroupName);
		obj.put(SequoiadbConstants.FIELD_NAME_SPLITQUERY, splitCondition);
		if(null != splitEndCondition)
			obj.put(SequoiadbConstants.FIELD_NAME_SPLITENDQUERY, splitEndCondition);
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.CMD_NAME_SPLIT;
		BSONObject dummy = new BasicBSONObject();
		SDBMessage rtnSDBMessage = adminCommand(commandString, obj, dummy,
				dummy, dummy, -1, -1, 0);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, sourceGroupName, destGroupName,
					splitCondition, splitEndCondition);
		}
	}

	/**
	 * @fn void split(String sourceGroupName, String destGroupName, double percent)
	 * @brief Split the specified collection from source group to target group by percent.
	 * @param sourceGroupName
	 *            the source group name
	 * @param destGroupName
	 *            the destination group name
     * @param percent
	 *            the split percent, Range:(0,100]
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void split(String sourceGroupName, String destGroupName,
			double percent) {
		// check arguments
		if((null == sourceGroupName || sourceGroupName.equals("")) ||
		   (null == destGroupName || destGroupName.equals("")) ||
		   (percent <= 0.0 || percent > 100.0)){
			 throw new BaseException("SDB_INVALIDARG");
		  }
		BSONObject obj = new BasicBSONObject();
		obj.put(SequoiadbConstants.FIELD_NAME_NAME, collectionFullName);
		obj.put(SequoiadbConstants.FIELD_NAME_SOURCE, sourceGroupName);
		obj.put(SequoiadbConstants.FIELD_NAME_TARGET, destGroupName);
		obj.put(SequoiadbConstants.FIELD_NAME_SPLITPERCENT, percent);
		String commandString = SequoiadbConstants.ADMIN_PROMPT
				+ SequoiadbConstants.CMD_NAME_SPLIT;
		BSONObject dummy = new BasicBSONObject();
		SDBMessage rtnSDBMessage = adminCommand(commandString, obj, dummy,
				dummy, dummy, -1, -1, 0);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0) {
			throw new BaseException(flags, sourceGroupName, destGroupName,
					percent);
		}
	}
	
	/**
	 * @fn DBCursor aggregate(List<BSONObject> obj)
	 * @brief Execute aggregate operation in current collection
	 * @param obj
	 *            The Bson object of rule list, can't be null
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public DBCursor aggregate(List<BSONObject> obj)
			throws BaseException {

		if (obj == null || obj.size() == 0)
			throw new BaseException("SDB_INVALIDARG");

		if (this.bulk_buffer == null) {
			this.bulk_buffer = IoBuffer.allocate(DEF_BUFFER_LENGTH);
			this.bulk_buffer.setAutoExpand(true);
			if (sequoiadb.endianConvert) {
				bulk_buffer.order(ByteOrder.LITTLE_ENDIAN);
			} else {
				bulk_buffer.order(ByteOrder.BIG_ENDIAN);
			}
		} else {
			bulk_buffer.position(0);
		}

		int messageLength = SDBMessageHelper.buildAggrRequest(
				bulk_buffer, collectionFullName, obj);

		connection.sendMessage(bulk_buffer.array(), messageLength);
		
		ByteBuffer byteBuffer = connection.receiveMessage(sequoiadb.endianConvert);
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);
		DBCursor cursor = null;
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC) {
				return cursor;
			} else {
				throw new BaseException(flags, obj);
			}
		}
		cursor = new DBCursor(rtnSDBMessage, this);
		return cursor;
	}
	
	/**
	 * @fn DBCursor getQueryMeta(BSONObject query, BSONObject
	 *     orderBy, BSONObject hint, long skipRows, long returnRows)
	 * @brief Get index blocks' or data blocks' infomation for concurrent query
	 * @param query
	 *            The matching condition
	 * @param orderBy
	 *            The ordered rule
	 * @param hint
	 *            One of the indexs of current collection,
	 *            using default index to query if not provided
     *            eg:{"":"ageIndex"}
	 * @param skipRows
	 *            The rows to be skipped
	 * @param returnRows
	 *            The rows to return
	 * @param flag
	 *            The flag to use which form for record data
	 *            0: bson stream
	 *            1: binary data stream, form: col1|col2|col3
	 * @return DBCursor of datas
	 * @exception com.sequoiadb.exception.BaseException
     * 
	 */
	public DBCursor getQueryMeta(BSONObject query,BSONObject orderBy,
			                     BSONObject hint,long skipRows, 
			                     long returnRows, int flag) {
		BSONObject dummy = new BasicBSONObject();
		if (query == null)
			query = dummy;
		if (orderBy == null)
			orderBy = dummy;
		if (hint == null)
			hint = dummy;
		if (returnRows == 0)
			returnRows = -1;
	    BSONObject hint1 = new BasicBSONObject();
	    hint1.put("Collection", this.collectionFullName);
		String command = SequoiadbConstants.ADMIN_PROMPT+SequoiadbConstants.GET_QUERYMETA;
		SDBMessage rtnSDBMessage = adminCommand(command, query, hint, orderBy, hint1,
				                                skipRows, returnRows, flag);
		DBCursor cursor = null;
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0) {
			if (flags == SequoiadbConstants.SDB_DMS_EOC) {
				return cursor;
			} else {
				throw new BaseException(flags, query, hint, orderBy, hint1,
						skipRows, returnRows);
			}
		}
		cursor = new DBCursor(rtnSDBMessage, this);
		return cursor;
	}
	
	/*
	 * @fn void attachCollection ( String subClFullName,BSONObject options )
	 * @brief Attach the specified collection.
	 * @param subClFullName
	 *            The full name of the subcollection
	 * @param options
	 *            The low boudary and up boudary
	 *            eg: {"LowBound":{a:1},"UpBound":{a:100}}
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void attachCollection( String subClFullName,BSONObject options ) {
		// check arguments
		if ( null == subClFullName || subClFullName.equals("") ||
		     null == options ||
		     null == collectionFullName || collectionFullName.equals("") ) {
			throw new BaseException("SDB_INVALIDARG");
		}
		// command
		String command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.CMD_NAME_ATTACH_CL;
		// build condition
		BSONObject newobj = new BasicBSONObject();
		newobj.put(SequoiadbConstants.FIELD_NAME_NAME, collectionFullName);
		newobj.put(SequoiadbConstants.FIELD_NAME_SUBCLNAME, subClFullName);
		for (String key : options.keySet()){
			newobj.put(key, options.get(key));
		}
		// build message/send/receive/extract
		SDBMessage rtnSDBMessage = adminCommand ( command, newobj, null, null, null,
				      			                  0, -1, 0 );
		int flags = rtnSDBMessage.getFlags();
		if (0 != flags){
			throw new BaseException(flags, subClFullName, options);
		}
	}
	
	/*
	 * @fn void detachCollection ( String subClFullName )
	 * @brief Dettach the specified collection.
	 * @param subClFullName
	 *            The full name of the subcollection
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public void detachCollection( String subClFullName ) {
		// check arguments
		if ( null == subClFullName || subClFullName.equals("") ||
		     null == collectionFullName || collectionFullName.equals("") ) {
			throw new BaseException("SDB_INVALIDARG");
		}
		// command
		String command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.CMD_NAME_DETACH_CL;
		// build condition
		BSONObject newobj = new BasicBSONObject();
		newobj.put(SequoiadbConstants.FIELD_NAME_NAME, collectionFullName);
		newobj.put(SequoiadbConstants.FIELD_NAME_SUBCLNAME, subClFullName);
		// build message/send/receive/extract
		SDBMessage rtnSDBMessage = adminCommand ( command, newobj, null, null, null,
				      			                  0, -1, 0 );
		int flags = rtnSDBMessage.getFlags();
		if (0 != flags){
			throw new BaseException(flags, subClFullName);
		}
	}
	
	private SDBMessage adminCommand(String commandString, BSONObject query,
			BSONObject selector, BSONObject orderBy, BSONObject hint,
			long skipRows, long returnRows, int flag) throws BaseException {
		// Admin command request
		// int reqId = 0;
		SDBMessage sdbMessage = new SDBMessage();
		BSONObject dummy = new BasicBSONObject();
		if (query == null)
			query = dummy;
		if (selector == null)
			selector = dummy;
		if (orderBy == null)
			orderBy = dummy;
		if (hint == null)
			hint = dummy;

		sdbMessage.setCollectionFullName(commandString);

		sdbMessage.setVersion(0);
		sdbMessage.setW((short) 1);
		sdbMessage.setPadding((short) 0);
		sdbMessage.setFlags(flag);
		sdbMessage.setNodeID(SequoiadbConstants.ZERO_NODEID);
		// sdbMessage.setResponseTo(reqId);
		// reqId++;
		sdbMessage.setRequestID(0);
		sdbMessage.setSkipRowsCount(skipRows);
		sdbMessage.setReturnRowsCount(returnRows);
		sdbMessage.setMatcher(query);
		sdbMessage.setSelector(selector);
		sdbMessage.setOrderBy(orderBy);
		sdbMessage.setHint(hint);

		byte[] request = SDBMessageHelper.buildQueryRequest(sdbMessage,
				sequoiadb.endianConvert);
		connection.sendMessage(request);
		
		ByteBuffer byteBuffer = connection.receiveMessage(sequoiadb.endianConvert);
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);

		return rtnSDBMessage;
	}
	
	private List<BSONObject> getMoreCommand(SDBMessage rtnSDBMessage)
			throws BaseException {
		// GetMore request
		long reqId = rtnSDBMessage.getRequestID();
		List<Long> contextIds = rtnSDBMessage.getContextIDList();
		List<BSONObject> fullList = new ArrayList<BSONObject>();
		boolean hasMore = true;
		while (hasMore) {
			SDBMessage sdbMessage = new SDBMessage();
			sdbMessage.setNodeID(SequoiadbConstants.ZERO_NODEID);
			// sdbMessage.setResponseTo(reqId);
			// reqId++;
			sdbMessage.setContextIDList(contextIds);
			sdbMessage.setRequestID(reqId);
			sdbMessage.setReturnRowsCount2(-1);

			byte[] request = SDBMessageHelper.buildGetMoreRequest(sdbMessage,
					sequoiadb.endianConvert);
			connection.sendMessage(request);
			
			ByteBuffer byteBuffer = connection.receiveMessage(sequoiadb.endianConvert);
			rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);

			int flags = rtnSDBMessage.getFlags();
			if (flags != 0) {
				if (flags == SequoiadbConstants.SDB_DMS_EOC) {
					hasMore = false;
				} else {
					throw new BaseException(flags);
				}
			} else {
				reqId = rtnSDBMessage.getRequestID();
				List<BSONObject> objList = rtnSDBMessage.getObjectList();
				fullList.addAll(objList);
			}
		}

		return fullList;
	}

	private void _update(int flag, BSONObject matcher, BSONObject modifier,
			BSONObject hint) throws BaseException {
		BSONObject dummy = new BasicBSONObject();
		if (matcher == null)
			matcher = dummy;
		if (modifier == null)
			modifier = dummy;
		if (hint == null)
			hint = dummy;
		SDBMessage sdbMessage = new SDBMessage();
		sdbMessage.setVersion(0);
		sdbMessage.setW((short) 0);
		sdbMessage.setPadding((short) 0);
		sdbMessage.setFlags(flag);
		sdbMessage.setNodeID(SequoiadbConstants.ZERO_NODEID);
		sdbMessage.setCollectionFullName(collectionFullName);
		sdbMessage.setRequestID(0);
		sdbMessage.setMatcher(matcher);
		sdbMessage.setModifier(modifier);
		sdbMessage.setHint(hint);

		byte[] request = SDBMessageHelper.buildUpdateRequest(sdbMessage,
				sequoiadb.endianConvert);
		connection.sendMessage(request);
		
		ByteBuffer byteBuffer = connection.receiveMessage(sequoiadb.endianConvert);
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);
		int flags = rtnSDBMessage.getFlags();
		if (flags != 0)
			throw new BaseException(flags, matcher, modifier, hint);
	}

}
