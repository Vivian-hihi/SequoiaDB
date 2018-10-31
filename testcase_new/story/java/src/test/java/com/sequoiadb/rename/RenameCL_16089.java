package com.sequoiadb.rename;

import java.util.ArrayList;
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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCL_16089.java  并发执行split切分和修改cl名 
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCL_16089 extends SdbTestBase{
	
	private String csName = "renameCS_16089";
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
		cs = sdb.createCollectionSpace(csName);
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
		
		Sequoiadb db = null; 
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(rename && !split){
				RenameUtil.checkRenameCLResult(db, csName, clName, newCLName);
				List<String> groups = new ArrayList<>();
				groups.add(sourceGroup);
				RenameUtil.checkSplitResult(db, csName, newCLName, groups);
			}else if(!rename && split){
				RenameUtil.checkCLExit(db, csName, clName, true);
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
	
	private class SplitThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
				cl.split(sourceGroup, targetGroup, 50);
			}finally {
				db.close();
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
