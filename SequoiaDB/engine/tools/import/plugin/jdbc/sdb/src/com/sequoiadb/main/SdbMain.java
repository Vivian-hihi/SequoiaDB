package com.sequoiadb.main;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.commons.cli.PosixParser;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import com.sequoiadb.service.ConnectDataBase;
import com.sequoiadb.tasks.ReadSdb;

public class SdbMain {
	
public	static Logger logger = Logger.getLogger(SdbMain.class);

public static void main(String[] args) {
	
	/**
	 * @param corePoolSize
	 * @param maximumPoolSize
	 * @param keepAliveTime
	 * @param unit
	 * @param workQueue
	 * */
	ThreadPoolExecutor executor = new ThreadPoolExecutor(12, 20, 200, TimeUnit.MILLISECONDS,
            new ArrayBlockingQueue<Runnable>(12));
	PropertyConfigurator.configure("lib/log4j.properties");
	
	Map<String,Object> map=paraseCommand(args);
	if(map != null){
	List<String> obj = (List)map.get("sqlList");
	for(String sql: obj){
	    ReadSdb myTask = new ReadSdb(map,sql);
        executor.execute(myTask);
    }
    executor.shutdown();
	}
}

   private static Map<String,Object> paraseCommand(String[] args){
	   
//	    command = "--connect jdbc:db2://192.168.30.223:50110/sequoia --username db2inst1 --password liuck_2015 --table sedb10 --sumrow 1000000 --threads 4";
//		String command = command;
		Options opt = new Options();
		opt.addOption("connect", true, "JDBC URL.");
		opt.addOption("username", true, "DataBase UserName.");
		opt.addOption("password", true, "DataBase password.");
		opt.addOption("table", true, "table name which you want to select.");
		opt.addOption("sumrow", true, "Rows which you want to select.");
		opt.addOption("threads", true, "Concurrent number");
		opt.addOption("version", false, "information about sdbJdbc");
		opt.addOption("sql", true, "sql");
		opt.addOption("h", "help", false, "print help for the command.");
		String formatstr = "gmkdir [--connect][--username][password][--table][--sumrow][--threads][-h/--help] DirectoryName";
		String versionstr = "version  v1.1";
		HelpFormatter formatter = new HelpFormatter();
		CommandLineParser parser = new PosixParser();
		CommandLine cl = null;
		List<String> list = new ArrayList<String>();
		Map<String, Object> map = new HashMap<String, Object>();
		try {
			cl = parser.parse(opt, args);
		} catch (ParseException e) {
			formatter.printHelp(formatstr, opt);
			logger.error(e.getStackTrace(), e);
		}
		// -h or --help
		if (cl == null) {
			logger.error("CommandLine error");
			return null;
		}
		if (null != cl && cl.hasOption("h")) {
			HelpFormatter hf = new HelpFormatter();
			hf.printHelp(formatstr, "", opt, "");
			return null;
		}
		if (null != cl && cl.hasOption("version")) {
			System.out.println(versionstr);
			return null;
		}
		// --connect
		if (null != cl && cl.hasOption("connect")) {
			String conStr = cl.getOptionValue("connect");
			int mysqlIndex = conStr.indexOf("oracle");
			int DB2Index = conStr.indexOf("db2");
			if (mysqlIndex > 0)
				map.put("dbType", "oracle");
			if (DB2Index > 0)
				map.put("dbType", "db2");
			map.put("url", cl.getOptionValue("connect"));
		}
		// --username
		if (null != cl && cl.hasOption("username")) {
			map.put("user", cl.getOptionValue("username"));
		}
		// --password
		if (null != cl && cl.hasOption("password")) {
			map.put("password", cl.getOptionValue("password"));
		}
		// --table
		if (null != cl && cl.hasOption("table")) {
			map.put("table", cl.getOptionValue("table"));
		}
		// --sumrow
		if (null != cl && cl.hasOption("sumrow")) {
			map.put("sumRow", cl.getOptionValue("sumrow"));
		}
		// --threads
		if (null != cl && cl.hasOption("threads")) {
			map.put("threads", cl.getOptionValue("threads"));
		}
		if (map.get("url") == null) {
			System.out.println("could not found --connect option");
			logger.error("could not found --connect option");
			return null;
		}
		if (map.get("dbType") == null) {
			logger.error("could not found --dbType option");
			return null;
		}
		if (map.get("user") == null) {
			System.out.println("could not found --username option");
			logger.error("could not found --username option");
			return null;
		}
		if (map.get("password") == null) {
			System.out.println("could not found --password option");
			logger.error("could not found --password option");
			return null;
		}
		if(cl != null && !cl.hasOption("sql")){
		if (map.get("table") == null) {
				System.out.println("could not found --table option");
				logger.info("could not found --table option");
				return null;
		}	
		if (map.get("sumRow") == null) {
			int sumrow = queryRowCount(map);
			map.put("sumRow",sumrow);
			logger.info("could not found --sumrow option");
			logger.info("default query all rows from"+map.get("table"));
		}
		if (map.get("threads") == null) {
			map.put("threads", 1);
			logger.info("could not found --threads option");
		}
		
		int sumRow = Integer.parseInt(map.get("sumRow").toString());
		int threads = Integer.parseInt(map.get("threads").toString());
		int avg = sumRow / threads;
		int start = 0;
		int end = avg;
		String sql = null;
		String table = map.get("table").toString();
		
		for (int i = 1; i <= threads; i++) {
			if (i == threads) {
				if (map.get("dbType").toString() == "db2")
					sql = "select * from (select s.*,rownumber() over() as rn from (select * from " + table
							+ ") as s ) as s1 where s1.rn between " + start + " and " + sumRow;
				if (map.get("dbType").toString() == "oracle")
					sql = "select * from (select rownum,* from " + table + " where rownum <=" + sumRow
							+ ") where rownum >" + start;
			} else {
				if (map.get("dbType").toString() == "db2")
					sql = "select * from (select s.*,rownumber() over() as rn from (select * from " + table
							+ ") as s ) as s1 where s1.rn between " + start + " and " + end;
				if (map.get("dbType").toString() == "oracle")
					sql = "select * from (select rownum,* from " + table + " where rownum <=" + sumRow
							+ ") where rownum >" + start;
			}
			start = i * avg + 1;
			end = (i + 1) * avg;
			list.add(sql);
		}
		}
		if(cl != null && cl.hasOption("sql")){
			String sqlList = cl.getOptionValue("sql");
			String sqlStr[] = sqlList.split(";");
			for(String sql : sqlStr){
			list.add(sql);
			}
		}
		map.put("sqlList", list);
		logger.info("args:" + map);
		return map;
   }
   private static int queryRowCount(Map<String,Object> map){
	   ConnectDataBase cdb = new ConnectDataBase(map.get("dbType").toString(), map.get("url").toString(),map.get("user").toString(),map.get("password").toString());
	   Connection conn = null;
	   PreparedStatement pstmt = null;
	   ResultSet rs = null;
	   conn = cdb.getConnection();
	   int rowCount = 0;
	   String queryRowSql = "select count(1) from "+map.get("table");
	   try {
		pstmt = conn.prepareStatement(queryRowSql);
		rs = pstmt.executeQuery();
		while(rs.next()){
			rowCount = Integer.parseInt(rs.getString(1));
		}
	} catch (SQLException e) {
		logger.error("query sumRows from table error");
		logger.error(e.getMessage());
	}
		
	   return rowCount;
   }
}
