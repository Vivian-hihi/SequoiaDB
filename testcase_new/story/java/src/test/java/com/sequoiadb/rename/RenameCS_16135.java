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
 * @Description RenameCS_16135.java 并发修改cs1、cs2名，cs2指定修改为cs1
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCS_16135 extends SdbTestBase{
	
	private String csNameA = "renameCS_16135A";
	private String csNameB = "renameCS_16135B";
	private String newCSNameB = "renameCS_16135B_new";
	private String clNameA = "renameCS_CL_16135A";
	private String clNameB = "renameCS_CL_16135B";
	private Sequoiadb sdb = null;
	private CollectionSpace csA = null;
	private CollectionSpace csB = null;
	private DBCollection clA= null;
	private DBCollection clB= null;
	private int recordNum = 1000;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		csA = sdb.createCollectionSpace(csNameA);
		csB = sdb.createCollectionSpace(csNameB);
		clA = csA.createCollection(clNameA);
		clB = csB.createCollection(clNameB);
		RenameUtil.insertData(clA, recordNum);
		RenameUtil.insertData(clB, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCSAThread reCSANameThread = new RenameCSAThread();
		RenameCSBThread reCSBNameThread = new RenameCSBThread();
		
		reCSANameThread.start();
		reCSBNameThread.start();
		
		boolean csARe = reCSANameThread.isSuccess();
		boolean csBRe = reCSBNameThread.isSuccess();
		
		Assert.assertTrue(csBRe, reCSBNameThread.getErrorMsg());
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(csARe){
				RenameUtil.checkRenameCSResult(db, csNameA, csNameB, 1);
			}else{
				RenameUtil.checkRenameCSResult(db, "NotExistCS16135A", csNameA, 1);
			}
			RenameUtil.checkRenameCSResult(db, "NotExistCS16135B", csNameB, 1);
		} finally{
			db.close();
		}
	}
	
	@AfterClass
	public void tearDown(){
		CommLib.clearCS(sdb, csNameA);
		CommLib.clearCS(sdb, csNameB);
		if(sdb!=null){
			sdb.close();
		}
	}
	
	private class RenameCSAThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Thread.sleep(5);
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				db.renameCollectionSpace(csNameA, csNameB);
			}finally {
				db.close();
			}
		}
	}
	
	private class RenameCSBThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				db.renameCollectionSpace(csNameB, newCSNameB);
			}finally {
				db.close();
			}
		}
	}
	
}
