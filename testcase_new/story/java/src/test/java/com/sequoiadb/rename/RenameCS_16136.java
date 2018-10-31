package com.sequoiadb.rename;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
 * @Description RenameCS_16136.java 主子表在不同cs上，并发修改主表和子表的cs名
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCS_16136 extends SdbTestBase{
	
	private String mainCSName = "maincs_16136";
	private String newMainCSName = "maincs_16136_new";
	private String subCSName = "subcs_16136";
	private String mainCLName = "maincl_16136";
	private String subCLName = "subcl_16136";
	private String newSubCLName = "subcl_16136_new";
	private Sequoiadb sdb = null;
	private CollectionSpace mainCS = null;
	private CollectionSpace subCS = null;
	private DBCollection mainCL = null;
	private int recordNum = 1000;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		mainCS = sdb.createCollectionSpace(mainCSName);
		subCS = sdb.createCollectionSpace(subCSName);
		createMainAndSubCL();
		RenameUtil.insertData(mainCL, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCSThread reCSNameThread = new RenameCSThread();
		RenameCLThread reCLNameThread = new RenameCLThread();
		
		reCSNameThread.start();
		reCLNameThread.start();
		
		Assert.assertTrue(reCSNameThread.isSuccess(), reCSNameThread.getErrorMsg());
		Assert.assertTrue(reCLNameThread.isSuccess(), reCLNameThread.getErrorMsg());
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			RenameUtil.checkRenameCSResult(db, mainCSName, newMainCSName, 0);
			RenameUtil.checkRenameCLResult(db, subCSName, subCLName, newSubCLName);
		} finally{
			db.close();
		}
	}
	
	@AfterClass
	public void tearDown(){
		CommLib.clearCS(sdb, newMainCSName);
		CommLib.clearCS(sdb, subCSName);
		if(sdb!=null){
			sdb.close();
		}
	}
	
	private class RenameCSThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Thread.sleep(5);
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				db.renameCollectionSpace(mainCSName, newMainCSName);
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
				CollectionSpace cs = db.getCollectionSpace(subCSName);
				cs.renameCollection(subCLName, newSubCLName);
			}finally {
				db.close();
			}
		}
	}
	
	private void createMainAndSubCL() {
		BSONObject options = new BasicBSONObject();
		options.put("ShardingKey", new BasicBSONObject("a", 1));
		options.put("ShardingType", "range");
		options.put("IsMainCL", true);
		mainCL = mainCS.createCollection(mainCLName, options);
		subCS.createCollection(subCLName);
		BSONObject bound = new BasicBSONObject();
		bound.put("LowBound", new BasicBSONObject("a", 0));
		bound.put("UpBound", new BasicBSONObject("a", 2000));
		mainCL.attachCollection(subCSName+"."+subCLName, bound);
	}
}
