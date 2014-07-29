package com.sequoiadb.testsql;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;


public class OperateConnection {
	
	public static Connection initConnection(String url, String uid, String pwd){
		try{
			return DriverManager.getConnection(url, uid, pwd);
		}
		catch(Exception e){
			e.printStackTrace();
			return null;
		}
	}
	
	public static void destroyConnection(Connection conn){
		try{
			if(conn != null && !conn.isClosed())
				conn.close();
			return;
		}
		catch(SQLException e){
			e.printStackTrace();
		}
	}
	
	public static void loadThirdDriver(String driverName){
		try{
			Class.forName(driverName);
		}catch(ClassNotFoundException e){
			e.printStackTrace();
		}
	}
}
