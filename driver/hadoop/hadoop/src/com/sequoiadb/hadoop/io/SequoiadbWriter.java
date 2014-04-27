package com.sequoiadb.hadoop.io;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.hadoop.util.SdbConnAddr;

public class SequoiadbWriter<K, V> extends RecordWriter<K, V> {
	private static final Log log = LogFactory.getLog(SequoiadbWriter.class);

	private DBCollection dbCollection;
	private Sequoiadb sequoiadb;

	public SequoiadbWriter(String collectionSpaceName, String collectionName,
			SdbConnAddr sdbConnAddr) {
		super();
		this.sequoiadb = new Sequoiadb(sdbConnAddr.getHost(),
				sdbConnAddr.getPort(), null, null);
		this.dbCollection = sequoiadb.getCollectionSpace(collectionSpaceName)
				.getCollection(collectionName);
		
	}

	@Override
	public void close(TaskAttemptContext arg0) throws IOException,
			InterruptedException {
		if(this.sequoiadb!=null){
			this.sequoiadb.disconnect();
		}
	}
	
	

	@Override
	public void write(K key, V value) throws IOException, InterruptedException {
		log.info("write");
		BSONObject bson=new BasicBSONObject();
		if(key!=null && value!=null){
			if(key instanceof BSONWritable){
				bson.put("_id",((BSONWritable)key).getBson());
			}else if (key instanceof BSONObject){
				bson.put("_id",key);
			}else{
				bson.put("_id",BSONWritable.toBSON(key));
			}
			
			if(value instanceof BSONWritable){
				bson.putAll(((BSONWritable)value).getBson());
			}else if(value instanceof BSONObject){
				bson.putAll(bson);
			}else{
				bson.put("value",value.toString());
			}
			log.info(bson);
			this.dbCollection.insert(bson);
		}else if(key==null){
			if(value instanceof BSONWritable){
				bson.putAll(((BSONWritable)value).getBson());
			}else if(value instanceof BSONObject){
				bson.putAll(bson);
			}else{
				bson.put("value",value.toString());
			}
			log.info(bson);
			this.dbCollection.insert(bson);
		}
		
	}

}
