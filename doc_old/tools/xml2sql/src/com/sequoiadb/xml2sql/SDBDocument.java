package com.sequoiadb.xml2sql;

public class SDBDocument {
	private int id;
	private String cnname;
	private String enname;
	private String cnpath;
	private String enpath;
	private int pid;
	private int order;
	private boolean valid;
	private boolean isDirectory;
	
	public int getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}
	public String getCnname() {
		return cnname;
	}
	public void setCnname(String cnname) {
		this.cnname = cnname;
	}
	public String getEnname() {
		return enname;
	}
	public void setEnname(String enname) {
		this.enname = enname;
	}
	public String getCnpath() {
		return cnpath;
	}
	public void setCnpath(String cnpath) {
		this.cnpath = cnpath;
	}
	public String getEnpath() {
		return enpath;
	}
	public void setEnpath(String enpath) {
		this.enpath = enpath;
	}
	public int getPid() {
		return pid;
	}
	public void setPid(int pid) {
		this.pid = pid;
	}
	public int getOrder() {
		return order;
	}
	public void setOrder(int order) {
		this.order = order;
	}
	public boolean isValid() {
		return valid;
	}
	public void setValid(boolean valid) {
		this.valid = valid;
	}
	public boolean isDirectory() {
		return isDirectory;
	}
	public void setDirectory(boolean isDirectory) {
		this.isDirectory = isDirectory;
	}
}
