package com.sequoiadb.tasks;

import java.sql.Connection;
import java.sql.Date;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Types;
import java.text.SimpleDateFormat;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.log4j.Logger;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BSONTimestamp;

import com.sequoiadb.main.SdbMain;
import com.sequoiadb.service.ConnectDataBase;

public class ReadSdb implements Runnable {


	private Map<String, Object> map;

	private String sql = null;

	Logger logger = Logger.getLogger(ReadSdb.class);
//	private static ConcurrentLinkedQueue<BSONObject> queue = new ConcurrentLinkedQueue<BSONObject>();
//	private static int count = 2;
//	private static CountDownLatch lath = new CountDownLatch(count); // finished when lath==0
    

	public ReadSdb() {

	}
	public ReadSdb(Map<String, Object> map, String sql) {

		this.map = map;
		this.sql = sql;
	}

	Connection conn = null;

	PreparedStatement pstmt = null;

	ResultSet rs = null;

	@Override
	public void run() {
		try {

			read(map.get("dbType").toString(), map.get("url").toString(), map.get("user").toString(),
					map.get("password").toString(), sql);
		} catch (InterruptedException e) {
			logger.error(e.getMessage());
			e.printStackTrace();
		}

	}
	// readDB
	@SuppressWarnings("unused")
	public void read(String dbType, String url, String user, String password, String sql) throws InterruptedException {

		ConnectDataBase cdb = new ConnectDataBase(dbType, url, user, password);
		conn = cdb.getConnection();
		long startTime = System.currentTimeMillis();
		logger.info("dbType=" + dbType + " url=" + url + " user=" + user + " sql" + sql);
		
		try {
			//select * from (select t.*,rownum as rown from DEPT t where rownum <=4) tabalias where tabalias.rown >1
			pstmt = conn.prepareStatement(sql);
			rs = pstmt.executeQuery();
			ExecutorService es = Executors.newFixedThreadPool(4);
			int sum=0;
			while (rs.next()) {
				ResultSetMetaData rsmd = rs.getMetaData();
				BSONObject bson = new BasicBSONObject();
				SdbMain.paraseSuccess++;
				int i = -1;
				int type = -1;
				try {
					for (i = 1; i <= rsmd.getColumnCount(); i++) {
						String name = rsmd.getColumnName(i);
						type = rsmd.getColumnType(i);
						switch (type) {
						case Types.BIGINT:
							bson.put(name, rs.getLong(name));
							break;
						case Types.BOOLEAN:
							bson.put(name, rs.getBoolean(name));
							break;
						case Types.DATE:
							bson.put(name, rs.getDate(name));
							break;
						case Types.DOUBLE:
							bson.put(name, rs.getDouble(name));
							break;
						case Types.FLOAT:
							bson.put(name, rs.getFloat(name));
							break;
						case Types.INTEGER:
							bson.put(name, rs.getInt(name));
							break;
						case Types.SMALLINT:
							bson.put(name, rs.getInt(name));
							break;
						case Types.TIME:
							bson.put(name, rs.getTime(name));
							break;
						case Types.TIMESTAMP:
							String tsamp = rs.getObject(name).toString();
							String dateStr = tsamp.substring(0, tsamp.lastIndexOf("."));
							String incStr = tsamp.substring(tsamp.lastIndexOf(".")+1);
							if(!incStr.equals("0")){
							SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
							java.util.Date date = format.parse(dateStr);
							int seconds = (int)(date.getTime()/1000);
							int inc = Integer.parseInt(incStr);
							BSONTimestamp ts = new BSONTimestamp(seconds,inc);
							bson.put(name, ts.toString());
							}else{
								bson.put(name, rs.getTimestamp(name));
							}
							break;
						case Types.TINYINT:
							bson.put(name, rs.getShort(name));
							break;
						case Types.VARCHAR:
							bson.put(name, rs.getString(name));
							break;
						case Types.CHAR:
							bson.put(name, rs.getString(name));
							break;
						case Types.NCHAR:
							bson.put(name, rs.getNString(name));
							break;
						case Types.NVARCHAR:
							bson.put(name, rs.getNString(name));
							break;
						case Types.BIT:
							bson.put(name, rs.getByte(name));
							break;
						case Types.BINARY:
							bson.put(name, rs.getBinaryStream(name));
							break;	
						case Types.BLOB:
							bson.put(name, null);
							break;
						case Types.CLOB:
							bson.put(name, null);
							break;
						case Types.NCLOB:
							bson.put(name, null);
							break;	
						case Types.NUMERIC:
							if(dbType.equals("oracle") && !name.equals("ROWN"))
							bson.put(name, rs.getBigDecimal(name));
							break;
						case Types.ROWID:
							bson.put(name, rs.getRowId(name));
							break;	
						case Types.NULL:
							bson.put(name, null);
							break;
						default:
							logger.error("could not found this type on metadata");
							break;
						}
					}
				} catch (Exception e) {
					logger.error(e.getMessage());
				}
				
				System.out.println(bson);
				/*queue.offer(bson);
				for (int k = 0; k < count; k++) {
					es.submit(new Poll());
					
				}*/
			}
			/*
			lath.await();
			es.shutdown();*/
			
		} catch (SQLException e) {
			logger.error(e.getMessage());
			e.printStackTrace();
		} finally {
			try {

				if (pstmt != null)
					pstmt.close();
				if (conn != null)
					conn.close();
				logger.info("costTime:" + (System.currentTimeMillis() - startTime)+"s");
			} catch (SQLException e) {
				logger.error(e.getMessage());
				e.printStackTrace();
			}
		}
		
	}

	/*public static class Poll implements Runnable {
        
		@Override
		public void run() {
			
			while (!queue.isEmpty()) {
			BSONObject s;
			s = queue.poll();
			if(s != null)
			System.out.println(s);
			}
			
			lath.countDown();
		}

	}*/

}
