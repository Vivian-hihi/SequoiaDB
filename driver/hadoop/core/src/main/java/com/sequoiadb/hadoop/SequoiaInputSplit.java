package com.sequoiadb.hadoop;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapreduce.InputSplit;
import org.bson.BSONObject;
import org.bson.util.JSON;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;

public class SequoiaInputSplit extends InputSplit implements Writable {

	private static final Log _log = LogFactory.getLog(SequoiaInputSplit.class);
	private transient DBCollection dbc;
	private String inputConn;
	private String csName;
	private String cName;
	private BSONObject query;
	private BSONObject selector;
	private BSONObject orderBy;
	private BSONObject hint;
	private long skipRows = 0;
	private long returnRows;
	private transient DBCursor cursor;
	
	public String getInputConn() {
		return inputConn;
	}

	public String getCsName() {
		return csName;
	}

	public String getcName() {
		return cName;
	}

	public BSONObject getQuery() {
		return query;
	}

	public BSONObject getSelector() {
		return selector;
	}

	public BSONObject getOrderBy() {
		return orderBy;
	}

	public BSONObject getHint() {
		return hint;
	}

	public long getSkipRows() {
		return skipRows;
	}

	public long getReturnRows() {
		return returnRows;
	}

	public SequoiaInputSplit(String inputConn, String csName, String cName,
			BSONObject query, BSONObject selector, BSONObject orderBy,
			BSONObject hint, long skipRows, long returnRows) {
//		System.out.println("initial input split....");
		this.dbc = new Sequoiadb(inputConn, null, null).getCollectionSpace(csName)
				.getCollection(cName);
		this.inputConn = inputConn;
		this.csName = csName;
		this.cName = cName;
		this.query = query;
		this.selector = selector;
		this.orderBy = orderBy;
		this.hint = hint;
		this.skipRows = skipRows;
		this.returnRows = returnRows;
		getCursor();
	}

	public SequoiaInputSplit() {}
	
	public SequoiaInputSplit(com.sequoiadb.hadoop.SequoiaInputSplit split) {
		this(split.getInputConn(), split.getCsName(), split.getcName(), split.getQuery(), 
				split.getSelector(), split.getOrderBy(), split.getHint(), split.getSkipRows(), split.getReturnRows());
	}

	@Override
	public long getLength() throws IOException {
//		System.out.println("get length....");
		return Long.MAX_VALUE;
	}

	@Override
	public String[] getLocations() throws IOException {
//		System.out.println("get locations....");
		String[] a = new String[1];
		a[0] = this.inputConn.split(":")[0];
		return a;
	}

	@Override
	public void readFields(DataInput input) throws IOException {
//		System.out.println("read fields...");
		this.inputConn = input.readUTF();
		this.csName = input.readUTF();
		this.cName = input.readUTF();
		this.dbc = new Sequoiadb(inputConn, null, null).getCollectionSpace(csName)
				.getCollection(cName);
		this.query = (BSONObject) JSON.parse(input.readUTF());
		this.selector = (BSONObject) JSON.parse(input.readUTF());
		this.orderBy = (BSONObject) JSON.parse(input.readUTF());
		this.hint = (BSONObject) JSON.parse(input.readUTF());
		this.skipRows = input.readLong();
		this.returnRows = input.readLong();
	}

	@Override
	public void write(DataOutput out) throws IOException {
//		System.out.println("write fields....");
		out.writeUTF(this.inputConn);
		out.writeUTF(this.csName);
		out.writeUTF(this.cName);
		out.writeUTF(JSON.serialize(this.query));
		out.writeUTF(JSON.serialize(this.selector));
		out.writeUTF(JSON.serialize(this.orderBy));
		out.writeUTF(JSON.serialize(this.hint));
		out.writeLong(skipRows);
		out.writeLong(returnRows);
	}

	@Override
	public String toString() {
		StringBuffer sb = new StringBuffer();
		sb.append("SequoiaInputSplit{INPUTCONN=");
		sb.append(this.inputConn);
		sb.append(", query=");
		sb.append(this.query.toString());
		sb.append('}');
		return sb.toString();
	}

	public DBCursor getCursor() {
//		System.out.println("get cursor...");
		if (cursor == null)
			cursor = dbc.query(query, selector, orderBy, hint, skipRows,
					returnRows);
		return cursor;
	}

}