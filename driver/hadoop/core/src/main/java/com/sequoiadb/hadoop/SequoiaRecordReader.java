package com.sequoiadb.hadoop;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import com.mongodb.hadoop.io.BSONWritable;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.exception.BaseException;

public class SequoiaRecordReader extends
		RecordReader<BSONWritable, BSONWritable> {

	private static final Log _log = LogFactory.getLog(SequoiaRecordReader.class);
	private final DBCursor cursor;
	private BSONObject current;
	private static String inputKey;
	private float total;

	public SequoiaRecordReader(SequoiaInputSplit split) {
		cursor = split.getCursor(); // ****add to SequoiaInputSplit.java
	}

	@Override
	public void close() throws IOException {
//		System.out.println("close reader....");
		if (cursor != null)
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
//		System.out.println("get current key.....");
//		System.out.println(current);
		return new BSONWritable(new BasicBSONObject(inputKey, current.get(inputKey)));
	}

	@Override
	public BSONWritable getCurrentValue() throws IOException,
			InterruptedException {
//		System.out.println("get current value.....");
		return new BSONWritable(current);
	}

	@Override
	public float getProgress() throws IOException, InterruptedException {
//		System.out.println("get progress.....");
		try {
			if(cursor.hasNext())
				return 0.0f;
			else
				return 1.0f;
		} catch(BaseException e) {
			return 1.0f;
		}
	}

	@Override
	public void initialize(InputSplit split, TaskAttemptContext context)
			throws IOException, InterruptedException {
//		System.out.println("Initial reader....");
		total = 1.0f;
		inputKey = new SequoiaConfiguration(context.getConfiguration()).getInputKey();
//		System.out.println(inputKey);
	}

	@Override
	public boolean nextKeyValue() throws IOException, InterruptedException {
//		System.out.println("next key value....");
		try {
			if(!cursor.hasNext())
				return false;
			current = cursor.getNext();
			return true;
		} catch(BaseException e) {
			return false;
		}
	}

}