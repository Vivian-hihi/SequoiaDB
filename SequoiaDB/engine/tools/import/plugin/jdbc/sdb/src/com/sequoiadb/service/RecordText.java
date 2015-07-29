package com.sequoiadb.service;

import java.io.File;
import java.io.FileOutputStream;

public class RecordText {

	public static void main(String[] args) {
		record();
	}
	public static void record(){
		FileOutputStream out = null;
		int count=1000;//写文件行数 
		try {   
            out = new FileOutputStream(new File("D:/record.txt"));     
            for (int i = 0; i < count; i++) {   
                out.write("测试java 文件操作\r\n".getBytes());   
            }
		}catch (Exception e) {   
                e.printStackTrace();   
            }   
            finally {   
                try {   
                    out.close();   
                } catch (Exception e) {   
                    e.printStackTrace();   
                } 
	     }
  }
}