package com.sequoiadb.hadoop ;

import java.io.IOException;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.InputFormat;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.JobContext;
import org.apache.hadoop.mapreduce.RecordReader;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

import com.mongodb.hadoop.io.BSONWritable;


public class SequoiaInputFormat extends InputFormat<BSONWritable, BSONWritable> {

	private static final Log _log = LogFactory.getLog(SequoiaInputFormat.class);
	
	@Override
	public RecordReader<BSONWritable, BSONWritable> createRecordReader(
			InputSplit split, TaskAttemptContext context) throws IOException,
			InterruptedException {
		if( !(split instanceof SequoiaInputSplit))
			throw new IllegalStateException( "Creation of a new RecordReader requires a SequoiaInputsplit instance." );
		final SequoiaInputSplit sis = (SequoiaInputSplit)split;
		return new SequoiaRecordReader(sis);
	}

	@Override
	public List<InputSplit> getSplits(JobContext context) throws IOException,
			InterruptedException {
//		System.out.println("get splits");
		final SequoiaConfiguration sconf = new SequoiaConfiguration(context.getConfiguration());
		return SequoiaConfigUtil.getSplits(sconf);
	}

	public boolean verifyConfiguration(Configuration conf) {
//		System.out.println("verify configuration....");
        return true;
    }
}