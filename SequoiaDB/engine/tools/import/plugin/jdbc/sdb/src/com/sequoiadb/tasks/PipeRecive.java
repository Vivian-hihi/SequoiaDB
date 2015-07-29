package com.sequoiadb.tasks;

import java.io.PipedInputStream;

public class PipeRecive extends Thread{
	
	PipedInputStream in = null;
	
	public PipedInputStream getPipedInputStream(){
         
		in = new PipedInputStream();
		
		return in;
	}
    @Override
    public void run() {
    	byte[] bys = new byte[1024];
    	try {
    		in.read(bys);
    		while(true){
    		System.out.println("读到的信息："+new String(bys).trim());
    		}
//    		in.close();
		} catch (Exception e) {
			// TODO: handle exception
			System.out.println(e.getMessage());
		}
    }
}
