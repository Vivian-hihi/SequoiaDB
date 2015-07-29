package com.sequoiadb.tasks;

import java.io.IOException;
import java.io.PipedOutputStream;
import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;

	/**
	 * @author liuchuankai
	 * 将BSON写入管道
	 * */
public class PipeSender extends Thread{
	
	private PipedOutputStream out =null;
	List<BSONObject> list = new ArrayList<BSONObject>();
//	BSONObject s = null;
	public PipeSender(List<BSONObject> list){
		this.list = list;
	}
	public  void setBson(BSONObject out){
//		this.s = out;
	}
	public PipedOutputStream getOutputStream()  
	   {  
	        this.out=new PipedOutputStream();  
	        return out;           
	   }
	
	@Override
	public void run() {
        try {  
//        	   System.out.println("sender:"+Thread.currentThread().getName());
        	    for(BSONObject s :list){
                out.write(s.toString().getBytes());
        	    }
                out.close();  
         } catch (IOException e)  
          {  
               System.out.println( e.getMessage() );  
          } 
	}
	
//	public static void main(String[] args) {
//		PipeSender sender = new PipeSender(null);
//		sender.start();
//	}
}
