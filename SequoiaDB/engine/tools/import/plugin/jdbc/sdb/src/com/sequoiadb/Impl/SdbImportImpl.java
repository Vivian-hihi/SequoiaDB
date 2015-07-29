package com.sequoiadb.Impl;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import com.sequoiadb.tasks.ReadSdb;

public class SdbImportImpl implements SdbImport {

	@Override
	public void readDb(String command) {
		ThreadPoolExecutor executor = new ThreadPoolExecutor(5, 10, 200, TimeUnit.MILLISECONDS,
	            new ArrayBlockingQueue<Runnable>(5));
		Map<String,Object> map = new HashMap<String,Object>();
		map = paser(command);
		for(int i=0;i<15;i++){
//	        ReadSdb myTask = new ReadSdb(map);
//	        executor.execute(myTask);
//	        System.out.println("PoolSize£º"+executor.getPoolSize()+"£¬WatingThread£º"+
//	        executor.getQueue().size()+"£¬Finished£º"+executor.getCompletedTaskCount());
	    }
	    executor.shutdown();
		
	}

	private Map<String,Object> paser(String command){
		
		return null;
	}
}
