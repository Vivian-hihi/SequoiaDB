package com.sequoiadb.hadoop;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.InputSplit;
import org.bson.BSONObject;
import org.bson.types.BasicBSONList;

import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.ReplicaNode;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.util.SDBMessageHelper;

public class SequoiaConfigUtil {

	public static final String INPUT_CONN = "sequoia.input.conn";
	public static final String OUTPUT_CONN = "sequoia.output.conn";

	public static final String INPUT_USERNAME = "sequoia.conn.input.username";
	public static final String INPUT_PASSWORD = "sequoia.conn.input.password";
	public static final String OUTPUT_USERNAME = "sequoia.conn.output.username";
	public static final String OUTPUT_PASSWORD = "sequoia.conn.output.password";

	/** collectionspace name for input */
	public static final String INPUT_COLLECTIONSPACE = "sequoia.input.collectionspace";
	/** Collection name for input */
	public static final String INPUT_COLLECTION = "sequoia.input.collection";

	/** Input key for mapper */
	public static final String INPUT_KEY = "sequoia.input.key";
	/** Query condition for mapper */
	public static final String INPUT_QUERY = "sequoia.input.query";
	/** Query select for mapper */
	public static final String INPUT_FIELDS = "sequoia.input.fields";
	/** Query sort for mapper */
	public static final String INPUT_SORT = "sequoia.input.sort";
	public static final String INPUT_HINT = "sequoia.inpu.hint";
	public static final String INPUT_SKIP = "sequoia.input.skip";
	public static final String INPUT_RETURN = "sequoia.input.return";
	/** Query limit for mapper */
	public static final String INPUT_LIMIT = "sequoia.input.limit";
	/** Query skip for mapper */
	public static final String INPUT_SPLIT_SIZE = "sequoia.input.split_size";

	/** output collectionspace name for output */
	public static final String OUTPUT_COLLECTIONSPACE = "sequoia.output.collectionspace";
	/** Collection name for output */
	public static final String OUTPUT_COLLECTION = "sequoia.output.collection";

	public static void setInputConn(Configuration _conf, String inputConn) {
		_conf.set(INPUT_CONN, inputConn);
	}

	public static String getInputConn(Configuration _conf) {
		return _conf.get(INPUT_CONN);
	}

	public static void setOutputConn(Configuration _conf, String outPutConn) {
		_conf.set(OUTPUT_CONN, outPutConn);
	}

	public static String getOutputConn(Configuration _conf) {
		return _conf.get(OUTPUT_CONN);
	}

	public static void setInputUsername(Configuration _conf, String username) {
		_conf.set(INPUT_USERNAME, username);
	}

	public static String getInputUsername(Configuration _conf) {
		return _conf.get(INPUT_USERNAME);
	}
	
	public static void setOutputUsername(Configuration _conf, String username) {
		_conf.set(OUTPUT_USERNAME, username);
	}

	public static String getOutputUsername(Configuration _conf) {
		return _conf.get(OUTPUT_USERNAME);
	}

	public static void setInputPassword(Configuration _conf, String password) {
		_conf.set(INPUT_PASSWORD, password);
	}

	public static String getInputPassword(Configuration _conf) {
		return _conf.get(INPUT_PASSWORD);
	}
	
	public static void setOutputPassword(Configuration _conf, String password) {
		_conf.set(OUTPUT_PASSWORD, password);
	}
	
	public static String getOutputPassword(Configuration _conf) {
		return _conf.get(OUTPUT_PASSWORD);
	}

	public static void setInputCollectionspace(Configuration _conf,
			String csName) {
		_conf.set(INPUT_COLLECTIONSPACE, csName);
	}

	public static String getInputCollectionspace(Configuration _conf) {
		return _conf.get(INPUT_COLLECTIONSPACE, null);
	}

	public static void setOutputCollectionspace(Configuration _conf,
			String csName) {
		_conf.set(OUTPUT_COLLECTIONSPACE, csName);
	}

	public static String getOutputCollectionspace(Configuration _conf) {
		return _conf.get(OUTPUT_COLLECTIONSPACE, null);
	}

	public static void setInputCollection(Configuration _conf, String collection) {
		_conf.set(INPUT_COLLECTION, collection);
	}

	public static String getInputCollection(Configuration _conf) {
		return _conf.get(INPUT_COLLECTION, null);
	}

	public static void setOutputCollection(Configuration _conf,
			String collection) {
		_conf.set(OUTPUT_COLLECTION, collection);
	}

	public static String getOutputCollection(Configuration _conf) {
		return _conf.get(OUTPUT_COLLECTION, null);
	}

	public static void setInputKey(Configuration _conf, String key) {
		_conf.set(INPUT_KEY, key);
	}

	public static String getInputKey(Configuration _conf) {
		// _id is the default key
		return _conf.get(INPUT_KEY, "_id");
	}

	public static void setInputQuery(Configuration _conf, BSONObject query) {
		_conf.set(INPUT_QUERY, query.toString());
	}

	public static BSONObject getInputQuery(Configuration _conf) {
		if (_conf.get(INPUT_QUERY, null) != null)
			return SDBMessageHelper.fromJson(_conf.get(INPUT_QUERY));
		return null;
	}

	public static void setInputFields(Configuration _conf, BSONObject fields) {
		_conf.set(INPUT_FIELDS, fields.toString());
	}

	public static BSONObject getInputFields(Configuration _conf) {
		if (_conf.get(INPUT_FIELDS, null) != null)
			return SDBMessageHelper.fromJson(_conf.get(INPUT_FIELDS));
		return null;
	}

	public static void setInputSort(Configuration _conf, BSONObject sort) {
		_conf.set(INPUT_SORT, sort.toString());
	}

	public static BSONObject getInputSort(Configuration _conf) {
		if (_conf.get(INPUT_SORT, null) != null)
			return SDBMessageHelper.fromJson(_conf.get(INPUT_SORT));
		return null;
	}

	public static void setInputHint(Configuration _conf, BSONObject hint) {
		_conf.set(INPUT_HINT, hint.toString());
	}

	public static BSONObject getInputHint(Configuration _conf) {
		if (_conf.get(INPUT_HINT, null) != null)
			return SDBMessageHelper.fromJson(_conf.get(INPUT_HINT));
		return null;
	}

	public static void setInputSkip(Configuration _conf, long skip) {
		_conf.setLong(INPUT_SKIP, skip);
	}

	public static long getInputSkip(Configuration _conf) {
		return _conf.getLong(INPUT_SKIP, 0);
	}

	public static void setInputReturn(Configuration _conf, long re) {
		_conf.setLong(INPUT_RETURN, re);
	}

	public static long getInputReturn(Configuration _conf) {
		return _conf.getLong(INPUT_RETURN, -1);
	}

	public static void setInputLimit(Configuration _conf, String limit) {
		_conf.set(INPUT_LIMIT, limit);
	}

	public static String getInputLimit(Configuration _conf) {
		return _conf.get(INPUT_LIMIT);
	}

	public static List<InputSplit> getSplits(SequoiaConfiguration sconf) {
		List<InputSplit> splits = new ArrayList<InputSplit>();
		String clFullName = sconf.getInputCollectionspace() + "."
				+ sconf.getInputCollection();
		String condition = "{'Name':'" + clFullName + "'}";
		String conn = sconf.getInputConn();
		String username = sconf.getInputUsername();
		String password = sconf.getInputPassword();
		Sequoiadb sdb = new Sequoiadb(conn, username, password);
		try {
			DBCursor cursor = sdb.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONS,
					condition, null, null);
			if (cursor.hasNext()) {
				BSONObject obj = cursor.getNext();
				BasicBSONList list = (BasicBSONList) obj.get("CataInfo");
				if(list == null) {
					String[] connStr = conn.split(":");
					splits.add(new SequoiaInputSplit(connStr[0], Integer.parseInt(connStr[1])));
				}
				int groupId;
				ReplicaNode node = null;
				Iterator<Object> ite = list.iterator();
				while (ite.hasNext()) {
					obj = (BSONObject) ite.next();
					groupId = Integer.parseInt(obj.get("GroupID").toString());
					node = sdb.getReplicaGroup(groupId).getSlave();
					splits.add(new SequoiaInputSplit(node.getHostName(), node
							.getPort()));
				}
			}
		} catch (Exception e) {
			sdb.disconnect();
			e.printStackTrace();
		}
		return splits;
	}

}
