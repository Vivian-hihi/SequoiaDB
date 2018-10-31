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
 * @Description RenameCL_16081.java 修改cs名和修改cl名并发 
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCL_16081 extends SdbTestBase{
	
	private String csName = "renameCS_16081";
	private String clNameA = "rename_CL_16081A";
	private String clNameB = "rename_CL_16081B";
	private String newCLName= "rename_CL_16081_new";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection clA = null;
	private DBCollection clB = null;
	private int recordNum = 1000;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cs = sdb.createCollectionSpace(csName);
		clA = cs.createCollection(clNameA);
		clB = cs.createCollection(clNameB);
		RenameUtil.insertData(clA, recordNum);
		RenameUtil.insertData(clB, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCLAThread reCLANameThread = new RenameCLAThread();
		RenameCLBThread reCLBNameThread = new RenameCLBThread();
		
		reCLANameThread.start();
		reCLBNameThread.start();
		
		boolean clARe = reCLANameThread.isSuccess();
		boolean clBRe = reCLBNameThread.isSuccess();
		
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(clARe && !clBRe){
				RenameUtil.checkRenameCLResult(db, csName, clNameA, newCLName);
				RenameUtil.checkRenameCLResult(db, csName, "16081NotExistCLB", clNameB);
			}else if(!clARe && clBRe){
				RenameUtil.checkRenameCLResult(db, csName, clNameB, newCLName);
				RenameUtil.checkRenameCLResult(db, csName, "16081NotExistCLA", clNameA);
			}else if(!clARe && !clBRe){
				Assert.fail("rename cl name to the same name, all failed");
			}else{
				Assert.fail("rename cl name to the same name, all success");
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
	
	private class RenameCLAThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace cs = db.getCollectionSpace(csName);
				cs.renameCollection(clNameA, newCLName);
			}finally {
				db.close();
			}
		}
	}
	
	private class RenameCLBThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace cs = db.getCollectionSpace(csName);
				cs.renameCollection(clNameB, newCLName);
			}finally {
				db.close();
			}
		}
	}
	
}
