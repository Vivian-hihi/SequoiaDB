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
 * @Description RenameCL_16084.java 并发修改cl名和创建相同cl 
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCL_16084 extends SdbTestBase{
	
	private String csName = "renameCS_16084";
	private String clName = "rename_CL_16084";
	private String newCLName= "rename_CL_16084_new";
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
		RenameUtil.insertData(cl, 1000);
	}
	
	@Test
	public void test(){ 
		RenameCLThread renameCLThread = new RenameCLThread();
		CreateCLBThread createCLThread = new CreateCLBThread();
		
		renameCLThread.start();
		createCLThread.start();
		
		boolean renameCL = renameCLThread.isSuccess();
		boolean createCL = createCLThread.isSuccess();
		
		Assert.assertTrue(renameCL, renameCLThread.getErrorMsg());
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(renameCL && !createCL){
				RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
			}else if(renameCL && createCL){
				cs = db.getCollectionSpace(csName);
				if(cs.isCollectionExist(clName)){
					Assert.fail("cl is been create, should exist");
				}
				if(cs.isCollectionExist(newCLName)){
					Assert.fail("cl is been rename, should exist");
				}
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
				cs.renameCollection(clName, newCLName);
			}finally {
				db.close();
			}
		}
	}
	
	private class CreateCLBThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace cs = db.getCollectionSpace(csName);
				cs.createCollection(clName);
			}finally {
				db.close();
			}
		}
	}
	
}
