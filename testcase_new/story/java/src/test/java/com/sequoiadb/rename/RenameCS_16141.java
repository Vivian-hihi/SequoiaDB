package com.sequoiadb.rename;

import java.util.Arrays;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
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
	private String subCSName = "subcs_16141";
	private String newSubCSName = "subcs_16141_new";
	private String mainCLName = "maincl_16141";
	private String subCLName = "subcl_16141";
	private Sequoiadb sdb = null;
	private CollectionSpace mainCS = null;
	private CollectionSpace subCS = null;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		// 跳过 standAlone 和数据组不足的环境
		if (CommLib.isStandAlone(sdb)) {
			throw new SkipException("skip StandAlone");
		}
		List<String> groupsName = CommLib.getDataGroupNames(sdb);
		if (groupsName.size() < 2) {
			throw new SkipException("current environment less than tow groups ");
		}
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
		
		if(!atttachThread.isSuccess()){
			Integer[] errnos = { -23 };
			BaseException error = (BaseException)atttachThread.getExceptions().get(0);
			if( !Arrays.asList(errnos).contains(error.getErrorCode()) ){
				Assert.fail(atttachThread.getErrorMsg());
			}
		}
		
		try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ){
			RenameUtil.checkRenameCSResult(db, subCSName, newSubCSName, 0);
			if(atttachThread.isSuccess()){
				checkSnapshot(db, mainCSName+"."+mainCLName, newSubCSName+"."+subCLName, true);
			}else{
				checkSnapshot(db, "", newSubCSName+"."+subCLName, false);
				
			}
		}
	}
	
	@AfterClass
	public void tearDown(){
		try {
			CommLib.clearCS(sdb, mainCSName);
			CommLib.clearCS(sdb, newSubCSName);
		} finally {
			if(sdb!=null){
				sdb.close();
			}
		}
	}
	
	private class RenameCSThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
				db.renameCollectionSpace(subCSName, newSubCSName);
			}
		}
	}
	
	private class AttachCLThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
				CollectionSpace cs = db.getCollectionSpace(mainCSName);
				DBCollection cl = cs.getCollection(mainCLName);
				BSONObject bound = new BasicBSONObject();
				bound.put("LowBound", new BasicBSONObject("a", 0));
				bound.put("UpBound", new BasicBSONObject("a", 2000));
				cl.attachCollection(subCSName+"."+subCLName, bound);
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
	
	private void checkSnapshot(Sequoiadb db, String fullMainCLName, String fullSubCLName, boolean mainCLExist){
		DBCursor cur = db.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{'Name':'" + fullSubCLName + "'}", "", "");
		BSONObject obj = cur.getNext();
		if(obj==null){
			Assert.fail("snapshot do not contain " + fullMainCLName +" message");
		}
		if(mainCLExist){
			String mainCLName = (String) obj.get("MainCLName");
			Assert.assertEquals(mainCLName, fullMainCLName, "cl attach success");
		}else{
			if(obj.containsField("MainCLName")){
				Assert.fail("cl not detach, snapshot:"+obj.toString());
			}
		}
	}
}
