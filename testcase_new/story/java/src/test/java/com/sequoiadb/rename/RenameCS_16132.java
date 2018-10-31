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
 * @Description RenameCS_16132.java 修改cs名和修改cl名并发 
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCS_16132 extends SdbTestBase{
	
	private String csName = "renameCS_16132";
	private String newCSName = "renameCS_16132_new";
	private String clName = "renameCS_CL_16132";
	private String newCLName = "renameCS_CL_16132_new";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection cl = null;
	private int recordNum = 1000;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cs = sdb.createCollectionSpace(csName);
		cl = cs.createCollection(clName);
		RenameUtil.insertData(cl, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCSThread reCSNameThread = new RenameCSThread();
		RenameCLThread reCLNameThread = new RenameCLThread();
		
		reCSNameThread.start();
		reCLNameThread.start();
		
		boolean csRe = reCSNameThread.isSuccess();
		boolean clRe = reCLNameThread.isSuccess();
		
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(csRe&&clRe){
				System.out.println("-------   1  ---------");
				RenameUtil.checkRenameCSResult(db, csName, newCSName, 1);
				RenameUtil.checkRenameCLResult(db, newCSName, clName, newCLName);
			}else if(!csRe&&clRe){
				System.out.println("-------   2  ---------");
				RenameUtil.checkRenameCSResult(db, newCSName, csName, 1);
				RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
			}else if(csRe&&!clRe){
				System.out.println("-------   3  ---------");
				RenameUtil.checkRenameCSResult(db, csName, newCSName, 1);
				RenameUtil.checkRenameCLResult(db, newCSName, newCLName, clName);
			}else if(!csRe&&!clRe){
				Assert.fail("Concurrent to renameCS and renameCL failed");
			}
		} finally{
			db.close();
		}
	}
	
	@AfterClass
	public void tearDown(){
		CommLib.clearCS(sdb, csName);
		CommLib.clearCS(sdb, newCSName);
		if(sdb!=null){
			sdb.close();
		}
	}
	
	private class RenameCSThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				db.renameCollectionSpace(csName, newCSName);
			}finally {
				db.close();
			}
		}
	}
	
	private class RenameCLThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace sdbcs = db.getCollectionSpace(csName);
				sdbcs.renameCollection(clName, newCLName);
			}finally {
				db.close();
			}
		}
	}
	
}
