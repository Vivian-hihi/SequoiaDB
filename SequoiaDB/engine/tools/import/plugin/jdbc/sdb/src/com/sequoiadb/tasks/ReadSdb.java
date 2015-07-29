package com.sequoiadb.tasks;

import java.io.IOException;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Types;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.Logger;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import com.sequoiadb.service.ConnectDataBase;


public class ReadSdb implements Runnable{
	
	private int taskNum;
	
	private Map<String,Object> map;
	
	private String sql =null;
	
	Logger logger = Logger.getLogger(ReadSdb.class);
	private static ConcurrentLinkedQueue<BSONObject> queue = new ConcurrentLinkedQueue<BSONObject>();
	private static int count = 2;
	private static CountDownLatch lath = new CountDownLatch(count); //finished when lath==0
	
	public  ReadSdb(int num) {
		this.taskNum = num;
	}
	public ReadSdb(){
		
	}
	public ReadSdb(Map<String,Object> map,String sql){
		
		this.map = map;
		this.sql = sql;
	}
   Connection conn = null;
	
	PreparedStatement pstmt = null;
	
	ResultSet rs = null;
	

	@Override
	public void run() {
        try {
        	
        	read(map.get("dbType").toString(),map.get("url").toString(),map.get("user").toString(),map.get("password").toString(),sql);
        } catch (InterruptedException e) {
        	logger.error(e.getMessage());
            e.printStackTrace();
        }
		
	}
	//readDB
	
	public  void read(String dbType,String url,String user,String password,String sql) throws InterruptedException{
		
		ConnectDataBase cdb = new ConnectDataBase(dbType,url,user,password);
		  conn = cdb.getConnection();
		  
          long startTime = 	System.currentTimeMillis();
          logger.info("dbType="+dbType+" url="+url+" user="+user+" sql"+sql);
//          System.out.println("dbType="+dbType+" url="+url+" user="+user+" sql"+sql);
		  try {
		   pstmt = conn.prepareStatement(sql);
		   rs = pstmt.executeQuery();
		   ExecutorService es = Executors.newFixedThreadPool(4);
		     
		   while (rs.next()) {
			   ResultSetMetaData  rsmd=rs.getMetaData();
			   BSONObject bson =  new BasicBSONObject();
			   
			   int i = -1;
			   int type = -1;
			   try{
				   for(i = 1;i<=rsmd.getColumnCount();i++){
					   String name = rsmd.getColumnName(i);
					   type = rsmd.getColumnType(i);
					   switch(type){
					   case Types.BIGINT:
						   bson.put(name, rs.getLong(name));break;
					   case Types.BOOLEAN:
						   bson.put(name, rs.getBoolean(name));break;
					   case Types.DATE:
						   bson.put(name, rs.getDate(name));break;
					   case Types.DOUBLE:
						   bson.put(name, rs.getDouble(name));break;
					   case Types.FLOAT:
						   bson.put(name, rs.getFloat(name));break;
					   case Types.INTEGER:
						   bson.put(name, rs.getInt(name));break;  
					   case Types.SMALLINT:
						   bson.put(name, rs.getInt(name));break;
					   case Types.TIME:
						   bson.put(name, rs.getTime(name));break;	
					   case Types.TIMESTAMP:
						   bson.put(name, rs.getTimestamp(name));break;	
					   case Types.TINYINT:
						   bson.put(name, rs.getShort(name));break;
					   case Types.VARCHAR:
						   bson.put(name, rs.getString(name));break;	  
					   case Types.NCHAR:
						   bson.put(name, rs.getNString(name));break;  
					   case Types.NVARCHAR:
						   bson.put(name, rs.getNString(name));break; 
					   case Types.BIT:
						   bson.put(name, rs.getByte(name));break;   
					   }
				   }
			   }catch(Exception e){
				   logger.error(e.getMessage());
			   }
			   ReadSdb.offer(bson);
			   for(int k=0;k<count;k++){
					es.submit(new Poll());
				}
		   }
		   lath.await();
			es.shutdown();
		  } catch (SQLException e) {
			  logger.error(e.getMessage());  
		   e.printStackTrace();
		  } finally {
		   try {
			   
		    if (pstmt != null)
		     pstmt.close();
		    if (conn != null)
		     conn.close();
		    logger.info("costTime"+(System.currentTimeMillis() - startTime));
//		    System.err.println("duringTime :"+(System.currentTimeMillis() - startTime));
		   } catch (SQLException e) {
			   logger.error(e.getMessage());  
		    e.printStackTrace();
		   }
		  }
	}
	
	public static void offer(BSONObject bon){
			
			queue.offer(bon);
	}
	public static class Poll implements Runnable{

		@Override
		public void run() {
			while(!queue.isEmpty()){
//				queue.poll();
				System.out.println(queue.poll());
			}
			lath.countDown();
			
		}
		
	}
	
}
