package com.sequoiadb.ant.tools;

public class sdbProperty {
	private String proName ; 
	private String proPort ; 
	
	public void setProName( String value )
	{
		this.proName = value ;
	}
	public void setProPort( String value )
	{
		this.proPort = value ; 
	}
	
	public String getProName()
	{
		return this.proName ; 
	}
	public String getProPort()
	{
		return this.proPort ; 
	}

}
