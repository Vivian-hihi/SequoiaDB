package com.sequoiadb.hadoop.io;


import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.hadoop.util.SdbConnAddr;

public class SequoiadbWriter<K, V> extends RecordWriter<K, V> {
	private static final Log log = LogFactory.getLog(SequoiadbWriter.class);

	private DBCollection dbCollection;
	private Sequoiadb sequoiadb;
	private List<BSONObject> lstBsonBuffer = null;
	private int bulkNum;

	public SequoiadbWriter(String collectionSpaceName, String collectionName,
			SdbConnAddr sdbConnAddr, int bulkNum) {
		super();
		this.sequoiadb = new Sequoiadb(sdbConnAddr.getHost(),
				sdbConnAddr.getPort(), null, null);
		log.info("collectionSpaceName:"+collectionSpaceName+"---collectionName:"+collectionName);
		
		CollectionSpace space=null;
		if(sequoiadb.isCollectionSpaceExist(collectionSpaceName)){
			space = sequoiadb.getCollectionSpace(collectionSpaceName);	
		}else{
			sequoiadb.createCollectionSpace(collectionSpaceName);
		}
		
		if(space.isCollectionExist(collectionName)){
			this.dbCollection=space.getCollection(collectionName);
		}else{
			this.dbCollection=space.createCollection(collectionName);
		}
		
		this.lstBsonBuffer = new ArrayList<BSONObject>(bulkNum);
		this.bulkNum = bulkNum;

	}

	@Override
	public void close(TaskAttemptContext arg0) throws IOException,
			InterruptedException {
		if(lstBsonBuffer.size()>0){
			this.dbCollection.bulkInsert(lstBsonBuffer, DBCollection.FLG_INSERT_CONTONDUP);
			lstBsonBuffer.clear();
		}
		if (this.sequoiadb != null) {
			this.sequoiadb.disconnect();
		}
	}

	@Override
	public void write(K key, V value) throws IOException, InterruptedException {
		log.info("write");
		BSONObject bson = null;

		if (value != null) {
			if (value instanceof BSONWritable) {
				bson = ((BSONWritable) value).getBson();
			} else if (value instanceof BSONObject) {
				bson = (BSONObject) value;
			} else {
				try {
					bson = BasicBSONObject.typeToBson(value);
				} catch (Exception e) {
					// TODO Auto-generated catch block
					log.error("Failed convert value to bson", e);
				}
			}
		}

		if (key != null && !(key instanceof NullWritable)) {
			if (key instanceof Text) {
				bson.put("_id", new ObjectId(((Text) key).toString()));
			} else if (key instanceof ObjectId) {
				bson.put("_id", key);
			} else {
				bson.put("_id", new ObjectId(key.toString()));
			}
		}
		
		if (lstBsonBuffer.size() < bulkNum) {
			log.info(bson);
			lstBsonBuffer.add(bson);
		} else {
			this.dbCollection.bulkInsert(lstBsonBuffer, DBCollection.FLG_INSERT_CONTONDUP);
			lstBsonBuffer.clear();
		}
	}
}
