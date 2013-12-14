package com.sequoia.hadoop.test;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.Job;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import com.mongodb.hadoop.io.BSONWritable;
import com.sequoiadb.hadoop.SequoiaConfiguration;
import com.sequoiadb.hadoop.SequoiaInputFormat;
import com.sequoiadb.hadoop.SequoiaOutputFormat;

public class WordCount {
	public static void main(String[] args) throws Exception {
		long beginTime = System.currentTimeMillis();
		if(args == null || args.length != 3) {
			System.out.println("Invalid argument");
			System.exit(1);
		}
		Configuration conf = new Configuration();
		SequoiaConfiguration sconf = new SequoiaConfiguration(conf);
		sconf.setInputConn(InitialData.conn);
//		sconf.setInputCollectionspace(InitialData.csInputName);
//		sconf.setInputCollection(InitialData.cInputName);
		sconf.setInputCollectionspace(args[1]);
		sconf.setInputCollection(args[2]);
		sconf.setOutputConn(InitialData.conn);
		sconf.setOutputCollectionspace(InitialData.csOutputName);
		sconf.setOutputCollection(InitialData.cOutputName);
//		sconf.setInputUsername(InitialData.USERNAME);
//		sconf.setInputPassword(InitialData.PASSWORD);
//		sconf.setOutputUsername(InitialData.USERNAME);
//		sconf.setOutputPassword(InitialData.PASSWORD);
		
		BSONObject condition = new BasicBSONObject();
		BSONObject query = new BasicBSONObject();
		condition.put("$gte", 0);
		condition.put("$lte", 10);
		query.put("age", condition);

		BSONObject fields = new BasicBSONObject();
		fields.put("name", "");
		fields.put("age", "");
		
		sconf.setInputFields(fields);

//		sconf.setInputQuery(query);
		sconf.setInputKey("name");
		
		final Job job = new Job(conf, args[0]);
//		System.out.println("input query:" + job.getConfiguration().get(SequoiaConfiguration.INPUT_QUERY));
		
		job.setJarByClass(WordCount.class);
		
		job.setMapperClass(WordCountMapper.class);
		job.setCombinerClass(WordCountReducer.class);	// combiner
		job.setReducerClass(WordCountReducer.class);
		
		job.setOutputKeyClass(BSONWritable.class);
		job.setOutputValueClass(BSONWritable.class);
		
		job.setInputFormatClass(SequoiaInputFormat.class);
		job.setOutputFormatClass(SequoiaOutputFormat.class);

//		System.exit(job.waitForCompletion(true) ? 0:1);
		job.waitForCompletion(true);
		long endTime = System.currentTimeMillis();
		System.out.println("During time: " + (endTime-beginTime));
	}

}
