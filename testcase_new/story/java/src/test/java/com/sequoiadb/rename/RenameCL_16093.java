package com.sequoiadb.rename;


import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCS_16094.java 并发增删改数据和修改cl名 
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCL_16093 extends SdbTestBase{
	
	private String csName = "renameCS_16094";
	private String clName = "rename_CL_16094";
	private String newCLName = "rename_CL_16094_new";
	private int recordNum = 2000;
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection cl = null;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cs = sdb.createCollectionSpace(csName);
		BSONObject options = new BasicBSONObject();
		cl = cs.createCollection(clName, options);
		RenameUtil.insertData(cl, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCLThread renameThread = new RenameCLThread();
		InsertThread putThread = new InsertThread();
		DeleteThread deleteThread = new DeleteThread();
		UpdateThread updateThread = new UpdateThread();
		
		renameThread.start();
		putThread.start();
		deleteThread.start();
		updateThread.start();
		
		Assert.assertTrue(renameThread.isSuccess(), renameThread.getErrorMsg());
		System.out.println(putThread.isSuccess());
		System.out.println(updateThread.isSuccess());
		System.out.println(deleteThread.isSuccess());
		
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
			cl = db.getCollectionSpace(csName).getCollection(newCLName);
			checkRecord(cl);
		}finally{
			db.close();
		}
	}
	
	@AfterClass
	public void tearDown(){
		CommLib.clearCS(sdb, csName);
		if(sdb!=null){
			sdb.close();
		}
	}
	
	private class RenameCLThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Thread.sleep(3000);
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace cs = db.getCollectionSpace(csName);
				cs.renameCollection(clName, newCLName);
			}finally {
				db.close();
			}
		}
	}
	
	private class InsertThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection sdbcl = db.getCollectionSpace(csName).getCollection(clName);
				for (int i = 0; i < 1000; i++) {
					BSONObject record = new BasicBSONObject();
					record.put("a", 1000+i);
					record.put("no", "No."+1000+i);
					record.put("phone", 13700000000L + 1000+i);
					record.put("text", "Test ReName, This is the test statement used to populate the data");
					sdbcl.insert(record);
				}
			}finally {
				db.close();
			}
		}
	}
	
	private class DeleteThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection sdbcl = db.getCollectionSpace(csName).getCollection(clName);
				sdbcl.delete(new BasicBSONObject("a", new BasicBSONObject("$lt", "1000")));
			}finally {
				db.close();
			}
		}
	}
	
	private class UpdateThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection sdbcl = db.getCollectionSpace(csName).getCollection(clName);
				sdbcl.update(new BasicBSONObject("a", new BasicBSONObject("$lt", "2000")), new BasicBSONObject("no", ""), null);
			}finally {
				db.close();
			}
		}
	}
	
	private void checkRecord(DBCollection dbcl){
		DBCursor cur = dbcl.query("{a :{$isnull: 0}, no: {$isnull: 0}, phone: {$isnull: 0}, text: {$isnull: 0}}", null, null, null);
		while(cur.hasNext()){
			BSONObject obj = cur.getNext();
			obj.toString();
		}
		cur.close();
	} 
	
}
