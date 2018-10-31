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
 * @Description RenameCL_16085.java  并发修改cl1名和创建cl2，其中创建cl名为更新名
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCL_16085 extends SdbTestBase{
	
	private String csName = "renameCS_16085";
	private String clName = "rename_CL_16085";
	private String newCLName= "rename_CL_16085_new";
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
		
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(renameCL && !createCL){
				RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
			}else if(!renameCL && createCL){
				cs = db.getCollectionSpace(csName);
				if(!cs.isCollectionExist(clName)){
					Assert.fail("cl is been create, should exist");
				}
				if(!cs.isCollectionExist(newCLName)){
					Assert.fail("cl is been rename, should exist");
				}
			}else if(!renameCL && !createCL){
				Assert.fail("rename cl and create cl to the same name, all failed");
			}else{
				Assert.fail("rename cl and create cl to the same name, all success");
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
				cs.createCollection(newCLName);
			}finally {
				db.close();
			}
		}
	}
	
}
