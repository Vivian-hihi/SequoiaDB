package com.sequoiadb.ant.sdbtask;

/**
 * @author LongYang
 */
public class SdbTime{

	private String startTime;
	private String endTime;

	public static String getStartTime(Date value){
		SimpleDateFormat formatter=new SimpleDateFormat("HH:mm:ss");
		this.startTime=value;
		String dateStart=formatter.format(startTime);
		return dateStart;
	}

	public static String getEndTime(Date value){
		SimpleDateFormat formatter=new SimpleDateFormat("HH:mm:ss");
		this.endtTime=value;
		String dateEnd=formatter.format(EndTime);
		return dateEnd;
	}

	public void sub(int startTime,int endTime){
		SimpleDateFormat for
		int sub;
		sub=endTime-startTime; 
		System.out.println("Testcase runtime:"+sub);
	}
}