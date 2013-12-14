package com.sequoia.hadoop.test;

import java.io.IOException;

import org.apache.hadoop.mapreduce.Mapper;

import com.mongodb.hadoop.io.BSONWritable;


public class WordCountMapper extends Mapper<BSONWritable, BSONWritable, BSONWritable, BSONWritable> {
	@Override
	protected void map(BSONWritable key, BSONWritable value,
			Context context)
			throws IOException, InterruptedException {
//		System.out.println("Begin mapper...");
//		System.out.println("Key:" + key.getBson() +"\nValue:" + value.getBson());
		context.write(key, value);
	}
}
