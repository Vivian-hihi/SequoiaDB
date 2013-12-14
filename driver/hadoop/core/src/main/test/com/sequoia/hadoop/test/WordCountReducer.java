package com.sequoia.hadoop.test;

import java.io.IOException;

import org.apache.hadoop.mapreduce.Reducer;

import com.mongodb.hadoop.io.BSONWritable;


public class WordCountReducer extends Reducer<BSONWritable, BSONWritable, BSONWritable, BSONWritable> {
	int i = 0;
	BSONWritable cc = new BSONWritable();
	public void reduce(BSONWritable key, Iterable<BSONWritable> values, Context context) throws IOException, InterruptedException  {
		for(BSONWritable value : values) {
//			System.out.println("Key:" + key.get("_id"));
//			System.out.println("value:" + value.get("age3"));
			++i;
		}
		
		cc.put("count", i);
		context.write(key, cc);
	}
	
}