package com.sequoiadb.base;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.net.IConnection;
import com.sequoiadb.util.SDBMessageHelper;

/**
 * @class DBCursor
 * @brief the cursor when query sth from DB
 */
public class DBCursor {
	private long reqId;
	private SDBMessage sdbMessage;
	private DBCollection dbc;
	private IConnection connection;
	private BSONObject current;
	private List<BSONObject> list;

	private byte[] currentRaw;
	private List<byte[]> listRaw;
	private int index;
	private boolean hasNext;
	private byte times;
	private long contextId;
	boolean endianConvert;

	DBCursor() {
		hasNext = false;
		sdbMessage = null;
		connection = null;
		dbc = null;
		list = null;
		listRaw = null;
		index = -1;
		reqId = 0;
		contextId = -1;
		endianConvert = false;
	}

	DBCursor(SDBMessage rtnSDBMessage, DBCollection dbc) {
		this.dbc = dbc;
		endianConvert = dbc.getSequoiadb().endianConvert;
		connection = dbc.getConnection();
		sdbMessage = rtnSDBMessage;
		sdbMessage.setNodeID(SequoiadbConstants.ZERO_NODEID);
		sdbMessage.setReturnRowsCount2(-1); // return data count
		list = new ArrayList<BSONObject>();
		listRaw = new ArrayList<byte[]>();
		reqId = rtnSDBMessage.getRequestID();
		contextId = rtnSDBMessage.getContextIDList().get(0);
		hasNext = false;
		current = null;
		times = 0;
		index = -1;
	}

	DBCursor(SDBMessage rtnSDBMessage, Sequoiadb sdb) {
		this.connection = sdb.getConnection();
		sdbMessage = rtnSDBMessage;
		sdbMessage.setNodeID(SequoiadbConstants.ZERO_NODEID);
		sdbMessage.setReturnRowsCount2(-1); // return data count
		list = new ArrayList<BSONObject>();
		listRaw = new ArrayList<byte[]>();
		reqId = rtnSDBMessage.getRequestID();
		contextId = rtnSDBMessage.getContextIDList().get(0);
		hasNext = false;
		current = null;
		times = 0;
		index = -1;
		endianConvert = sdb.endianConvert;
	}

	/**
	 * @fn boolean hasNext()
	 * @brief Judge whether next data exists
	 * @return whether next data exists
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public boolean hasNext() throws BaseException {
		if (connection == null)
			return hasNext;
		if (times > 0 || sdbMessage == null)
			return hasNext;
		if (list == null || index >= (list.size() - 1)) {
			getListFromDB(true);
			index = -1;
		}
		if (list == null || list.size() == 0) {
			hasNext = false;
		} else {
			hasNext = true;
		}
		++times;
		return hasNext;
	}

	/**
	 * @fn boolean hasNext()
	 * @brief judge whether next data exists
	 * @return whether next data exists
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public boolean hasNextRaw() throws BaseException {
		if (connection == null)
			return hasNext;
		if (times > 0 || sdbMessage == null)
			return hasNext;
		if (listRaw == null || index >= (listRaw.size() - 1)) {
			getListFromDB(false);
			index = -1;
		}
		if (listRaw == null || listRaw.size() == 0) {
			hasNext = false;
		} else {
			hasNext = true;
		}
		++times;
		return hasNext;
	}

	/**
	 * @fn BSONObject getNext()
	 * @brief Get next data
	 * @return next data
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public BSONObject getNext() throws BaseException {
		if (connection == null)
			return null;
		if (times == 0)
			hasNext();
		if (hasNext) {
			++index;
			current = list.get(index);
			times = 0;
			return current;
		}
		return null;
	}

	/**
	 * @fn BSONObject getNext()
	 * @brief get next data
	 * @return next data
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public byte[] getNextRaw() throws BaseException {
		if (connection == null)
			return null;
		if (times == 0)
			hasNextRaw();
		if (hasNext) {
			++index;
			currentRaw = listRaw.get(index);
			times = 0;
			return currentRaw;
		}
		return null;
	}

	/**
	 * @fn void updateCurrent(BSONObject modifier, BSONObject hint)
	 * @brief update current data
	 * @param modifier
	 *            the modify rule
	 * @param hint
	 *            update by hint
	 * @exception com.sequoiadb.exception.BaseException
	 */
	/*
	public void updateCurrent(BSONObject modifier, BSONObject hint)
			throws BaseException {
		if (dbc != null && current != null) {
			BSONObject query = new BasicBSONObject();
			query.put("_id", (ObjectId) current.get("_id"));
			dbc.update(query, modifier, hint);
			DBCursor cursor = dbc.query(query, null, null, hint);
			current = cursor.getNext();
		}
	}*/

	/*
	 * @fn void deleteCurrent()
	 * @brief delete current data in DB
	 * @exception com.sequoiadb.exception.BaseException
	 */
	/*
	public void deleteCurrent() throws BaseException {
		if (dbc != null && current != null) {
			BSONObject query = new BasicBSONObject();
			query.put("_id", (ObjectId) current.get("_id"));
			dbc.delete(query);
		}
		current = null;
	}
	*/

	/**
	 * @fn BSONObject getCurrent()
	 * @brief Get current data
	 * @return the current data
	 */
	public BSONObject getCurrent() {
		return current;
	}

	/**
	 * @fn void close()
	 * @brief Close DBCursor
	 */
	public void close() {
		killCursor();
		sdbMessage = null;
		dbc = null;
		hasNext = false;
		current = null;
		list = null;
		listRaw = null;
	}

	private void getListFromDB(boolean decode) {
		if (connection == null || contextId == -1)
			throw new BaseException("SDB_NOT_CONNECTED");

		if (decode) {
			list.clear();
		} else {
			listRaw.clear();
		}
		
		sdbMessage.setRequestID(reqId);
		byte[] request = SDBMessageHelper.buildGetMoreRequest(sdbMessage,
				endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtnSDBMessage = null;

		if (decode) {
			rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);
		} else {
			rtnSDBMessage = SDBMessageHelper.msgExtractReplyRaw(byteBuffer);
		}

		int flags = rtnSDBMessage.getFlags();
		if (flags == SequoiadbConstants.SDB_DMS_EOC
				|| contextId != rtnSDBMessage.getContextIDList().get(0)) {
			hasNext = false;
			index = -1;
			current = null;
			list = null;
			listRaw = null;
		} else if (flags != 0) {
			throw new BaseException(flags);
		} else {
			reqId = rtnSDBMessage.getRequestID();
			if (decode) {
				list = rtnSDBMessage.getObjectList();
			} else {
				listRaw = rtnSDBMessage.getObjectListRaw();
			}
		}
	}

	private void killCursor() {
		if (connection == null && contextId == -1)
			return;
		long[] contextIds = new long[] { contextId };
		byte[] request = SDBMessageHelper.buildKillCursorMsg(0, contextIds,
				endianConvert);
		connection.sendMessage(request);

		ByteBuffer byteBuffer = connection.receiveMessage(endianConvert);
		SDBMessage rtnSDBMessage = SDBMessageHelper.msgExtractReply(byteBuffer);
		assert (rtnSDBMessage.getFlags() == 0);

		connection = null;
		contextId = -1;
	}
}
