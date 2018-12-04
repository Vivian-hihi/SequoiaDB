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
 * @Description RenameCS_16140.java detach子表和修改主表cs名并发
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCS_16140 extends SdbTestBase{
	
	private String mainCSName = "maincs_16140";
	private String newMainCSName = "maincs_16140_new";
	private String subCSName = "subcs_16140";
	private String mainCLName = "maincl_16140";
	private String subCLName = "subcl_16140";
	private Sequoiadb sdb = null;
	private CollectionSpace mainCS = null;
	private CollectionSpace subCS = null;
	private DBCollection mainCL = null;
	private int recordNum = 1000;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		// 跳过 standAlone 和数据组不足的环境
		if (CommLib.isStandAlone(sdb)) {
			throw new SkipException("skip StandAlone");
		}
		//TODO:1、不需要屏蔽一组三节点模式
		List<String> groupsName = CommLib.getDataGroupNames(sdb);
		if (groupsName.size() < 2) {
			throw new SkipException("current environment less than tow groups ");
		}
		mainCS = sdb.createCollectionSpace(mainCSName);
		subCS = sdb.createCollectionSpace(subCSName);
		createMainAndSubCL();
		RenameUtil.insertData(mainCL, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCSThread reCSNameThread = new RenameCSThread();
		DetachCLThread detachThread = new DetachCLThread();
		
		reCSNameThread.start();
		detachThread.start();
		
		Assert.assertTrue(reCSNameThread.isSuccess(), reCSNameThread.getErrorMsg());
		
		if(!detachThread.isSuccess()){
			Integer[] errnos = { -23 };
			BaseException error = (BaseException)detachThread.getExceptions().get(0);
			if( !Arrays.asList(errnos).contains(error.getErrorCode()) ){
				Assert.fail(detachThread.getErrorMsg());
			}
		}		

		try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ){
			RenameUtil.checkRenameCSResult(db, mainCSName, newMainCSName, 0);
			if(detachThread.isSuccess()){				
				checkSnapshot(db, "", false);
			}else{
				checkSnapshot(db, newMainCSName+"."+mainCLName, true);
			}
		}//TODO:3、和文本测试用例结果不符，请确认检测点
	}
	
	@AfterClass
	public void tearDown(){
		try {
			CommLib.clearCS(sdb, newMainCSName);
			CommLib.clearCS(sdb, subCSName);
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
				db.renameCollectionSpace(mainCSName, newMainCSName);
			}
		}
	}
	
	private class DetachCLThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
				CollectionSpace cs = db.getCollectionSpace(mainCSName);
				DBCollection cl = cs.getCollection(mainCLName);
				Thread.sleep(10);
				cl.detachCollection(subCSName+"."+subCLName);
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
	
	private void checkSnapshot(Sequoiadb db, String fullMainCLName, boolean mainCLExist){
		DBCursor cur = db.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{'Name':'" + subCSName+"."+subCLName + "'}", "", "");
		BSONObject obj = cur.getNext();
		if(mainCLExist){
			String mainCLName = (String) obj.get("MainCLName");
			if(!mainCLName.equals(fullMainCLName)){
				Assert.fail("cl already detach, should not exist, snapshot:"+obj.toString());
			}
		}else{//TODO:2、测试点不严谨，如果此时子表被关联删除，用例不会报错
			if(obj.get("MainCLName") != null){
				Assert.fail("cl not detach, snapshot:"+obj.toString());
			}
		}
	}
}
