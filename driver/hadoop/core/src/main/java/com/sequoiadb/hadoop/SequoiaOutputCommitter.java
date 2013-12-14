package com.sequoiadb.hadoop;

import java.io.IOException;

import org.apache.hadoop.mapreduce.OutputCommitter;

public class SequoiaOutputCommitter extends OutputCommitter {

	@Override
	public void setupJob(org.apache.hadoop.mapreduce.JobContext jobContext)
			throws IOException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void setupTask(
			org.apache.hadoop.mapreduce.TaskAttemptContext taskContext)
			throws IOException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public boolean needsTaskCommit(
			org.apache.hadoop.mapreduce.TaskAttemptContext taskContext)
			throws IOException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void commitTask(
			org.apache.hadoop.mapreduce.TaskAttemptContext taskContext)
			throws IOException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void abortTask(
			org.apache.hadoop.mapreduce.TaskAttemptContext taskContext)
			throws IOException {
		// TODO Auto-generated method stub
		
	}

}
