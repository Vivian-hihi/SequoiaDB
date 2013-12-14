package com.sequoiadb.hadoop;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapreduce.InputSplit;

public class SequoiaInputSplit extends InputSplit implements Writable {

	private static final Log _log = LogFactory.getLog(SequoiaInputSplit.class);
	private String hostname;
	private int port;
	

	public SequoiaInputSplit() {}
	
	
	public SequoiaInputSplit(String hostname, int port) {
		this.hostname = hostname;
		this.port = port;
	}
	
	public String getHostname() {
		return hostname;
	}
	
	public int getPort() {
		return port;
	}
	
	@Override
	public long getLength() throws IOException {
//		System.out.println("get length....");
		return Long.MAX_VALUE;
	}

	@Override
	public String[] getLocations() throws IOException {
//		System.out.println("get locations....");
		String[] a = new String[1];
		if(hostname == null)
			return a;
		a[0] = hostname;
		return a;
	}

	@Override
	public void readFields(DataInput input) throws IOException {
//		System.out.println("read fields...");
		this.hostname = input.readUTF();
		this.port = input.readInt();
	}

	@Override
	public void write(DataOutput out) throws IOException {
//		System.out.println("write fields....");
		out.writeUTF(hostname);
		out.writeInt(port);
	}
	
	@Override
	public String toString() {
		return hostname + ":" + port;
	}


}