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
 * @Description RenameCS_16141.java attach子表和修改子表cs名并发
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCS_16141 extends SdbTestBase{
	
	private String mainCSName = "maincs_16141";
	private String newMainCSName = "maincs_16141_new";
	private String subCSName = "subcs_16141";
	private String mainCLName = "maincl_16141";
	private String subCLName = "subcl_16141";
	private Sequoiadb sdb = null;
	private CollectionSpace mainCS = null;
	private CollectionSpace subCS = null;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		mainCS = sdb.createCollectionSpace(mainCSName);
		subCS = sdb.createCollectionSpace(subCSName);
		createMainAndSubCL();
	}
	
	@Test
	public void test(){ 
		RenameCSThread reCSNameThread = new RenameCSThread();
		AttachCLThread atttachThread = new AttachCLThread();
		
		reCSNameThread.start();
		atttachThread.start();
		
		Assert.assertTrue(reCSNameThread.isSuccess(), reCSNameThread.getErrorMsg());
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			RenameUtil.checkRenameCSResult(db, mainCSName, newMainCSName, 0);
			if(atttachThread.isSuccess()){
				checkSnapshot(db, newMainCSName+"."+mainCLName, true);
			}else{
				checkSnapshot(db, "", false);
			}
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
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				db.renameCollectionSpace(mainCSName, newMainCSName);
			}finally {
				db.close();
			}
		}
	}
	
	private class AttachCLThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace cs = db.getCollectionSpace(mainCSName);
				DBCollection cl = cs.getCollection(mainCLName);
				BSONObject bound = new BasicBSONObject();
				bound.put("LowBound", new BasicBSONObject("a", 0));
				bound.put("UpBound", new BasicBSONObject("a", 2000));
				cl.attachCollection(subCSName+"."+subCLName, bound);
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
		mainCS.createCollection(mainCLName, options);
		subCS.createCollection(subCLName);
	}
	
	private void checkSnapshot(Sequoiadb db, String fullMainCLName, boolean mainCLExist){
		DBCursor cur = db.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{'Name':'" + subCSName+"."+subCLName + "'}", "", "");
		BSONObject obj = cur.getNext();
		if(mainCLExist){
			String mainCLName = (String) obj.get("MainCLName");
			if(!mainCLName.equals(fullMainCLName)){
				Assert.fail("cl already detach, should not exist, snapshot:"+obj.toString());
			}
		}else{
			if(obj.get("MainCLName") != null){
				Assert.fail("cl not detach, snapshot:"+obj.toString());
			}
		}
	}
}
