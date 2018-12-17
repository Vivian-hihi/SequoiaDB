package com.sequoiadb.rename;

import java.util.Arrays;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCL_16814.java  seqDB-16814:并发修改cl名和analyze统计集合空间信息
 * @author luweikang
 * @date 2018年10月17日
 * @review  wuyan 2018.10.31
 */
public class RenameCL_16814 extends SdbTestBase{
	
	private String clName = "rename_CL_16814";
	private String newCLName = "rename_CL_16814_new";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection cl = null;
	private int recordNum = 1000;
	private String groupName = null;
	
	//覆盖mode 5种模式
	@DataProvider(name = "operData")
	public Object[][] generateDatas(){
		return new Object[][]{
			new Object[]{1, clName + "_" + 0, newCLName + "_" + 0},
			new Object[]{2, clName + "_" + 1, newCLName + "_" + 1},
			new Object[]{3, clName + "_" + 2, newCLName + "_" + 2},
			new Object[]{4, clName + "_" + 3, newCLName + "_" + 3},
			new Object[]{5, clName + "_" + 4, newCLName + "_" + 4},
		};
	}
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		// 跳过 standAlone
		if (CommLib.isStandAlone(sdb)) {
			throw new SkipException("skip StandAlone");
		}
		cs = sdb.getCollectionSpace( SdbTestBase.csName );
		List<String> rgNames = CommLib.getDataGroupNames(sdb);	
		groupName = rgNames.get(0);
		
		for (int i = 0; i < 5; i++) {
			cl = cs.createCollection( clName + "_" + i, new BasicBSONObject("Group", groupName) );
			RenameUtil.insertData( cl, recordNum );
		}
	}
	
	@Test(dataProvider = "operData")
	public void test(int analyzeMode, String testCLName, String testNewCLName){ 
		RenameCLThread reCLNameThread = new RenameCLThread(testCLName, testNewCLName);
		AnalyzeCL analyzeCL = new AnalyzeCL(analyzeMode, testCLName);
		
		reCLNameThread.start();
		analyzeCL.start();
		
		boolean analyze = analyzeCL.isSuccess();
		Assert.assertTrue(reCLNameThread.isSuccess(), "concurrent exec rename cl and analyze, renameCL failed");
		
		try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
			RenameUtil.checkRenameCLResult(db, csName, testCLName, testNewCLName);
		}
		
		if(!analyze){
			Integer[] errnos = { -23, -34, -264 };
			BaseException error = (BaseException)analyzeCL.getExceptions().get(0);
			if( !Arrays.asList(errnos).contains(error.getErrorCode()) ){
				Assert.fail(analyzeCL.getErrorMsg());
			}
		}
		
		//只校验rename之后旧统计信息有清理,新统计信息有创建,不关注内容
		try(Sequoiadb dataDB = sdb.getReplicaGroup(groupName).getMaster().connect()){
			DBCollection stat = dataDB.getCollectionSpace("SYSSTAT").getCollection("SYSCOLLECTIONSTAT");
			BSONObject filter1 = new BasicBSONObject();
			filter1.put("Collection", testCLName);
			filter1.put("CollectionSpace", csName);
			long num1 = stat.getCount(filter1);
			Assert.assertEquals(num1, 0, "check cl " + testCLName +" be rename, analyze shuold cleanUp");
			BSONObject filter2 = new BasicBSONObject();
			filter2.put("Collection", testNewCLName);
			filter2.put("CollectionSpace", csName);
			long num2 = stat.getCount(filter2);
			long expNum;
			if(analyzeMode > 3 || !analyze){
				expNum = 0;
			}else{
				expNum = 1;
			}
			Assert.assertEquals(num2, expNum, "check new name cl " + testNewCLName +" , analyze shuold create");
		}
	}
	
	@AfterClass
	public void tearDown(){
		try {
			for (int i = 0; i < 5; i++) {
				CommLib.clearCL(sdb, SdbTestBase.csName, newCLName + "_" +i);
			}
		} finally {
			if(sdb!=null){
				sdb.close();
			}
		}
	}
	
	private class RenameCLThread extends SdbThreadBase{

		private String oldCLName;
		private String newCLName;
		
		public RenameCLThread( String oldCLName, String newCLName){
			this.oldCLName = oldCLName;
			this.newCLName = newCLName;
		}
		
		@Override
		public void exec() throws Exception {
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
				CollectionSpace cs = db.getCollectionSpace(SdbTestBase.csName);
				cs.renameCollection(oldCLName, newCLName);
			}
		}
	}
	
	private class AnalyzeCL extends SdbThreadBase{
		
		private int analyzeMode;
		private String clName1;
		
		public AnalyzeCL( int analyzeMode, String clName){
			this.analyzeMode = analyzeMode;
			this.clName1 = clName;
		}
		
		@Override
		public void exec() throws Exception {
			
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ){
				BSONObject options = new BasicBSONObject();
				options.put("Mode", analyzeMode);
				options.put("Collection", csName +"."+clName1);
				options.put("GroupName", groupName);
				options.put("NodeSelect", "master");
				db.analyze(options);
			}
		}
		
	}
	
}
