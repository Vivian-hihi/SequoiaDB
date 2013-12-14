package com.sequoiadb.hadoop ;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import com.mongodb.hadoop.io.BSONWritable;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;


public class SequoiaRecordWriter<K, V> extends RecordWriter<K, V> {
	
	private final DBCollection dbc;
	private final TaskAttemptContext context;
	private static final Log _log = LogFactory.getLog(SequoiaRecordWriter.class);
	public SequoiaRecordWriter(JobConf conf, TaskAttemptContext context) throws Exception {
//		System.out.println("initial record writer...");
		String outputConn = conf.get(SequoiaConfigUtil.OUTPUT_CONN);
		String outputCsName = conf.get(SequoiaConfigUtil.OUTPUT_COLLECTIONSPACE);
		String outputCName = conf.get(SequoiaConfigUtil.OUTPUT_COLLECTION);
		String username = conf.get(SequoiaConfigUtil.OUTPUT_USERNAME);
		String password = conf.get(SequoiaConfigUtil.OUTPUT_PASSWORD);
		this.dbc = new Sequoiadb(outputConn, username, password).getCollectionSpace(outputCsName).getCollection(outputCName); 
		this.context = context;
	}
	
	@Override
	public void close(TaskAttemptContext context) throws IOException,
			InterruptedException {
//		System.out.println("writer disconnect....");
		dbc.getSequoiadb().disconnect();
	}

	@Override
	public void write(K key, V value) throws IOException, InterruptedException {
		BSONObject obj = new BasicBSONObject();
		if (_log.isTraceEnabled()) _log.trace( "Writing out data {k: " + key + ", value:  " + value);
//		System.out.println("write: key:" + ((BSONWritable)key).getBson());
		if(key != null && value != null) {
			if(key instanceof BSONWritable) {
				obj.putAll(((BSONWritable)key).getBson());
			}
			else if(key instanceof BSONObject) {
				obj.putAll((BSONObject)key);
			}
			else {
				obj.put("_id", key.toString());
			}
			if(value instanceof BSONWritable) {
				obj.putAll(((BSONWritable)value).getBson());
			}
			else if (value instanceof BSONObject) {
				obj.putAll((BSONObject)value);
			}
			else {
				obj.put("value", value.toString());
			}
		} else if(key == null) {
			if(value instanceof BSONObject)
				dbc.insert((BSONObject)value);
			else if (value instanceof BSONWritable)
				dbc.insert(((BSONWritable)value).getBson());
			else {
				obj.put("value", value.toString());
			}
		} else if (value == null) {
			if(key instanceof BSONWritable) {
				obj.putAll(((BSONWritable)key).getBson());
			}
			else if(key instanceof BSONObject) {
				obj.putAll((BSONObject)key);
			}
			else {
				obj.put("_id", key.toString());
			}
		}
		dbc.insert(obj);
	}
	
	public TaskAttemptContext getContext() {
		return context;
	}

	
}