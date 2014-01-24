package com.sequoiadb.ant.tools;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;

import com.ibm.staf.STAFException;
import com.ibm.staf.STAFHandle;
import com.ibm.staf.STAFResult;

public class stafTools extends Task{
	String workHost=null;
	String workType=null;
	String toHost=null;
	String workDir=null;
	String common=null;
	String fileName=null;
	String saveDir=null;
	public void setWorkHost( String value ){
		workHost = value;
	}
	public void setWorkType( String value ){
		workType = value;
	}
	public void setToHost( String value ){
		toHost = value;
	}
	public void setWorkDir( String value){
		workDir = value;
	}
	public void setCommon( String value ){
		common = value;
	}
	public void setFileName( String value ){
		fileName = value;
	}
	public void setSaveDir( String value ){
		saveDir=value;
	}
	private String STAFResultToString(STAFResult result) {

		String msg = "RC=" + result.rc + "\nmsg=" + result.result;
		return msg;
	}
	
	public void execute(){
		STAFHandle handle = null;
		STAFResult result = null ;
		String request  = null;
		System.out.println("stafTools");
		System.out.println("workType is : "+workType);
		try {
			
			handle = new STAFHandle("ant-sdbtasks");
			if(workType.equals("copy"))
			{
				System.out.println("copy work");
				request = "COPY FILE " 
						+ fileName
						+ " TODIRECTORY "
						+ saveDir
						+ " TOMACHINE "
						+ toHost ; 
				
				System.out.println("exec: staf " + workHost+ " FS " + request);
				result = handle.submit2( workHost , "FS", request);
				if (result.rc != STAFResult.Ok) {
					throw new BuildException(STAFResultToString(result));
				}
				
				
			}
			if( workType.equals("shell"))
			{
				System.out.println("shell work");
				request = "START SHELL COMMAND " + common + " WAIT 30m WORKDIR "+workDir ; 
				System.out.println("exec: staf " + workHost+ " PROCESS " + request);
				result = handle.submit2( workHost , "PROCESS", request);
				if (result.rc != STAFResult.Ok) {
					throw new BuildException(STAFResultToString(result));
				}
			}
		} catch (STAFException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
