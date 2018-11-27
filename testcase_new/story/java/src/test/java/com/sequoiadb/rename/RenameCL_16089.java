package com.sequoiadb.rename;

import java.util.ArrayList;
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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCL_16089.java  并发执行split切分和修改cl名 
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCL_16089 extends SdbTestBase{
	
	private String clName = "rename_CL_16089";
	private String newCLName= "rename_CL_16089_new";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection cl = null;
	private int recordNum = 10000;
	private String sourceGroup = null;
	private String targetGroup = null;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		if(CommLib.isStandAlone(sdb)){
			throw new SkipException("skip StandAlone");
		}
		List<String> rgNames = CommLib.getDataGroupNames(sdb);
		if(rgNames.size() <= 1){
			throw new SkipException("current environment less than tow groups");
		}
		sourceGroup = rgNames.get(0);
		targetGroup = rgNames.get(1);
		cs = sdb.getCollectionSpace(csName);
		cl = createShardingCL();
		RenameUtil.insertData(cl, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCLThread renameCLThread = new RenameCLThread();
		SplitThread splitThread = new SplitThread();
		
		renameCLThread.start();
		splitThread.start();
		
		boolean rename = renameCLThread.isSuccess();
		boolean split = splitThread.isSuccess();
		
		if(!rename){
			Integer[] errnosA = { -147 };
			BaseException errorA = (BaseException)renameCLThread.getExceptions().get(0);
			if( !Arrays.asList(errnosA).contains(errorA.getErrorCode()) ){
				Assert.fail(renameCLThread.getErrorMsg());
			}
		}
		
		if(!split){
			Integer[] errnosB = { -23, -147 };
			BaseException errorB = (BaseException)splitThread.getExceptions().get(0);
			if( !Arrays.asList(errnosB).contains(errorB.getErrorCode()) ){
				Assert.fail(splitThread.getErrorMsg());
			}
		}
		
		try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ){
			if(rename && !split){
				RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
				List<String> groups = new ArrayList<>();
				groups.add(sourceGroup);
				RenameUtil.checkSplitResult(db, csName, newCLName, groups);
			}else if(!rename && split){
				cs = db.getCollectionSpace(csName);
				if(cs.isCollectionExist(clName)){
					Assert.fail("cl is been rename faild, should not exist");
				}
				List<String> groups = new ArrayList<>();
				groups.add(sourceGroup);
				groups.add(targetGroup);
				RenameUtil.checkSplitResult(db, csName, clName, groups);
			}else if(rename && split){
				RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
				List<String> groups = new ArrayList<>();
				groups.add(sourceGroup);
				groups.add(targetGroup);
				RenameUtil.checkSplitResult(db, csName, newCLName, groups);
			}else{
				Assert.fail("rename cl and split cl all failed");
			}
		}
	}
	
	@AfterClass
	public void tearDown(){
		try {
			CommLib.clearCL(sdb, csName, clName);
			CommLib.clearCL(sdb, csName, newCLName);
		} finally {
			if(sdb!=null){
				sdb.close();
			}
		}
	}
	
	private class RenameCLThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
				CollectionSpace cs = db.getCollectionSpace(csName);
				cs.renameCollection(clName, newCLName);
			}
		}
	}
	
	private class SplitThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
				DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
				cl.split(sourceGroup, targetGroup, 50);
			}
		}
	}
	
	private DBCollection createShardingCL() {
		BSONObject options = new BasicBSONObject();
		options.put("ShardingKey", new BasicBSONObject("a", 1));
		options.put("ShardingType", "hash");
		options.put("Group", sourceGroup);
		return cs.createCollection(clName, options);
	}
	
}
