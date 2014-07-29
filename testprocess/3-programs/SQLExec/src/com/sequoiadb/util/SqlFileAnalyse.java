package com.sequoiadb.util;

import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.File;

public class SqlFileAnalyse {
	private File testcase;
	private FileReader fr;
	private BufferedReader br;
	
	private EXPECTEDFLAG expectedFlag;
	
	public SqlFileAnalyse(){
		testcase = null;
		fr = null;
		br = null;
		
		expectedFlag = EXPECTEDFLAG.IGNORE;
	}
	
	public EXPECTEDFLAG getExpectedFlag(){
		return expectedFlag;
	}
	
	public void setFile(File file){
		completeRead();
		testcase = file;
	}
	
	public void startRead(){
		try{
			fr = new FileReader(testcase);
			br = new BufferedReader(fr);
		}
		catch(IOException e){
			e.printStackTrace();
		}
	}
	
	public void completeRead(){
		try{
			if(br != null)
				br.close();
			if(fr != null)
				fr.close();
		}
		catch(IOException e){
			e.printStackTrace();
		}
	}
	
	public String getNextSql(){
		try{
			String s1;
			
			do{
				s1 = br.readLine();
			}while(s1 != null && !validateSql(s1));
			
			return (s1!=null)? s1.trim() : null;
		}
		catch(IOException e){
			e.printStackTrace();
			return null;
		}
	}
	
	private boolean validateSql(String line){
		if(line.trim().length()==0
			|| line.trim().startsWith("--")
			|| line.trim().startsWith("//")
			//|| line.indexOf("DISTINCT  CAST")>-1  // cancel SELECT  DISTINCT  CAST, because distinct null will cause ssql-server core dump
		  )
		{
			setExpectedFalg(line);
			return false;
		}
		
		return true;
	}
	
	private void setExpectedFalg(String line){
		String tmp = line.trim();
		
		if(tmp.startsWith("--succ"))
			expectedFlag = EXPECTEDFLAG.SUCCESS;
		else if(tmp.startsWith("--fail"))
			expectedFlag = EXPECTEDFLAG.FAIL;
		else if(tmp.startsWith("--ignore"))
			expectedFlag = EXPECTEDFLAG.IGNORE;
	}
}
