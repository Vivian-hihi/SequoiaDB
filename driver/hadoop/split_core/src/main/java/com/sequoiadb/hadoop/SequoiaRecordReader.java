package com.sequoiadb.hadoop;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import com.mongodb.hadoop.io.BSONWritable;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;

public class SequoiaRecordReader extends
		RecordReader<BSONWritable, BSONWritable> {

	private static final Log _log = LogFactory
			.getLog(SequoiaRecordReader.class);
	private DBCursor cursor;
	private BSONObject current;
	private static String inputKey;

	@Override
	public void initialize(InputSplit split, TaskAttemptContext context)
			throws IOException, InterruptedException {
		// System.out.println("Initial reader....");
		SequoiaInputSplit seqSplit = (SequoiaInputSplit) split;

		Configuration _conf = context.getConfiguration();
		inputKey = SequoiaConfigUtil.getInputKey(_conf);

		String hostname = seqSplit.getHostname();
		int port = seqSplit.getPort();
		String username = SequoiaConfigUtil.getInputUsername(_conf);
		String password = SequoiaConfigUtil.getInputPassword(_conf);
		String cs = SequoiaConfigUtil.getInputCollectionspace(_conf);
		String cl = SequoiaConfigUtil.getInputCollection(_conf);
		BSONObject query = SequoiaConfigUtil.getInputQuery(_conf);
		BSONObject selector = SequoiaConfigUtil.getInputFields(_conf);
		BSONObject orderBy = SequoiaConfigUtil.getInputSort(_conf);
		BSONObject hint = SequoiaConfigUtil.getInputHint(_conf);
		long skipRows = SequoiaConfigUtil.getInputSkip(_conf);
		long returnRows = SequoiaConfigUtil.getInputReturn(_conf);
		
		cursor = new Sequoiadb(hostname, port, username, password)
				.getCollectionSpace(cs)
				.getCollection(cl)
				.query(query, selector, orderBy, hint, skipRows,
						returnRows);
	}

	@Override
	public void close() throws IOException {
		// System.out.println("close reader....");
		if(cursor != null)
			cursor.close();
	}

	public BSONWritable createKey() {
		return new BSONWritable();
	}

	public BSONWritable createValue() {
		return new BSONWritable();
	}

	@Override
	public BSONWritable getCurrentKey() throws IOException,
			InterruptedException {
		// System.out.println("get current key.....");
		return new BSONWritable(new BasicBSONObject(inputKey,
				current.get(inputKey)));
	}

	@Override
	public BSONWritable getCurrentValue() throws IOException,
			InterruptedException {
		// System.out.println("get current value.....");
		current.removeField(inputKey);
		return new BSONWritable(current);
	}

	@Override
	public float getProgress() throws IOException, InterruptedException {
		// System.out.println("get progress.....");
		try {
			if (cursor.hasNext())
				return 0.0f;
			else
				return 1.0f;
		} catch (BaseException e) {
			return 1.0f;
		}
	}

	@Override
	public boolean nextKeyValue() throws IOException, InterruptedException {
		// System.out.println("next key value....");
		try {
			if(cursor == null)
				return false;
			if (!cursor.hasNext())
				return false;
			current = cursor.getNext();
			return true;
		} catch (BaseException e) {
			return false;
		}
	}

}