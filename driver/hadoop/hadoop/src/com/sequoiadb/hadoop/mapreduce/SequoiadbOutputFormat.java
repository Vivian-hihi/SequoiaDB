package com.sequoiadb.hadoop.mapreduce;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configurable;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.*;
import org.codehaus.jackson.map.DeserializerFactory.Config;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.hadoop.io.SequoiadbWriter;
import com.sequoiadb.hadoop.util.SdbConnAddr;
import com.sequoiadb.hadoop.util.SequoiadbConfigUtil;

public class SequoiadbOutputFormat extends OutputFormat implements Configurable{
	private static final Log log = LogFactory
			.getLog(SequoiadbInputFormat.class);

	private  String collectionSpaceName;
	private  String collectionName;
	private	 SdbConnAddr sdbConnAddr;
	
	public SequoiadbOutputFormat(){

	}
	
	@Override
	public void checkOutputSpecs(JobContext jobContext){	
			Sequoiadb sequoiadb = new Sequoiadb(sdbConnAddr.getHost(), sdbConnAddr.getPort(), null, null);
			if(!sequoiadb.isCollectionSpaceExist(collectionSpaceName)){
				CollectionSpace cs =sequoiadb.createCollectionSpace(collectionSpaceName);
				cs.createCollection(collectionName);
			}else{
				CollectionSpace cs=sequoiadb.getCollectionSpace(collectionSpaceName);
				if(!cs.isCollectionExist(collectionName)){
					cs.createCollection(collectionName);
				}
			}
			sequoiadb.disconnect();
	}

	@Override
	public OutputCommitter getOutputCommitter(
			TaskAttemptContext taskAttemptContext) throws IOException,
			InterruptedException {
		System.out.println("OutputCommitter");
		return new SequoiadbOutputCommitter();
	}

	@Override
	public RecordWriter getRecordWriter(TaskAttemptContext taskAttemptContext)
			throws IOException, InterruptedException {
		log.info("getRecordWriter");
		return new SequoiadbWriter(collectionSpaceName,collectionName,sdbConnAddr);
	}
	
	private Configuration conf;
	
	@Override
	public Configuration getConf() {
		// TODO Auto-generated method stub
		return conf;
	}

	@Override
	public void setConf(Configuration configuration) {
		this.conf=configuration;
		this.collectionName=SequoiadbConfigUtil.getOutCollectionName(conf);
		this.collectionSpaceName=SequoiadbConfigUtil.getOutCollectionSpaceName(conf);
		String url=SequoiadbConfigUtil.getOutputURL(conf);
		this.sdbConnAddr=new SdbConnAddr(url);
	}

}
