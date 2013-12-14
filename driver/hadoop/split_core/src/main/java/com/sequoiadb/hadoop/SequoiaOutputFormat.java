package com.sequoiadb.hadoop;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapreduce.JobContext;
import org.apache.hadoop.mapreduce.OutputCommitter;
import org.apache.hadoop.mapreduce.OutputFormat;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;

public class SequoiaOutputFormat<K, V> extends OutputFormat<K, V> {

	private static final Log _log = LogFactory
			.getLog(SequoiaOutputFormat.class);

	@Override
	public void checkOutputSpecs(JobContext context) throws IOException,
			InterruptedException {
		// System.out.println("check outputSpecs ");
		Configuration conf = context.getConfiguration();
		String outputConn = conf.get(SequoiaConfigUtil.OUTPUT_CONN);
		String outputCsName = conf
				.get(SequoiaConfigUtil.OUTPUT_COLLECTIONSPACE);
		String outputCName = conf.get(SequoiaConfigUtil.OUTPUT_COLLECTION);
		String username = conf.get(SequoiaConfigUtil.INPUT_USERNAME);
		String password = conf.get(SequoiaConfigUtil.INPUT_PASSWORD);
		Sequoiadb sdb = new Sequoiadb(outputConn, username, password);
		CollectionSpace cs = null;
		if(!sdb.isCollectionSpaceExist(outputCsName))
			sdb.createCollectionSpace(outputCsName);
		cs = sdb.getCollectionSpace(outputCsName);
		if(!cs.isCollectionExist(outputCName))
			cs.createCollection(outputCName);
		sdb.disconnect();
	}

	@Override
	public OutputCommitter getOutputCommitter(TaskAttemptContext context)
			throws IOException, InterruptedException {
		return new SequoiaOutputCommitter();
	}

	@Override
	public RecordWriter<K, V> getRecordWriter(TaskAttemptContext context)
			throws IOException, InterruptedException {
		// System.out.println("Get record writer....");
		try {
			return new SequoiaRecordWriter<K, V>(
					(JobConf) context.getConfiguration(), context);
		} catch (IOException e) {
			throw e;
		} catch (InterruptedException e) {
			throw e;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}

	public SequoiaOutputFormat() {
	}

}
