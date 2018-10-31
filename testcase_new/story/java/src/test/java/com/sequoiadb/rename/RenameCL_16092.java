package com.sequoiadb.rename;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCL_16092.java  并发事务操作和修改cl名 
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCL_16092 extends SdbTestBase{
	
	private String csName = "renameCS_16092";
	private String clName = "rename_CL_16092";
	private String newCLName= "rename_CL_16092_new";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private int recordNum = 2000;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cs = sdb.createCollectionSpace(csName);
		cs.createCollection(clName);
	}
	
	@Test
	public void test(){ 
		RenameCLThread renameCLThread = new RenameCLThread();
		TransactionThread transThread = new TransactionThread();
		
		renameCLThread.start();
		transThread.start();
		
		boolean rename = renameCLThread.isSuccess();
		boolean trans = transThread.isSuccess();
		
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(rename && !trans){
				System.out.println(1);
				RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
			}else if(!rename && trans){
				RenameUtil.checkCLExit(db, csName, newCLName, true);
			}else if(rename && trans){
				System.out.println(3);
				RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
			}else{
				Assert.fail("rename cl and split cl all failed");
			}
		} finally{
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
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace cs = db.getCollectionSpace(csName);
				for (int i = 0; i < 20; i++) {
					cs.renameCollection(clName, newCLName);
					Thread.sleep(20);
					cs.renameCollection(newCLName, clName);
				}
			}finally {
				db.close();
			}
		}
	}
	
	private class TransactionThread extends SdbThreadBase{
		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
				db.beginTransaction();
				RenameUtil.insertData(cl, recordNum);
				db.commit();
			}finally {
				db.close();
			}
		}
	}
	
}
