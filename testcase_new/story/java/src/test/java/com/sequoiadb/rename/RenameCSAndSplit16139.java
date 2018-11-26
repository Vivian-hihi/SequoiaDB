package com.sequoiadb.rename;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BSONDecimal;
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
* @FileName  RenameCSAndSplit16139.java
* @content   concurrent rename cs and split cl
* @testlink  seqDB-16139
* @author    wuyan
* @Date      2018.10.30
* @version   1.00
*/
public class RenameCSAndSplit16139 extends SdbTestBase{
	
	private String csName = "renameCS16139";
	private String newCSName = "renameCSNew16139";
	private String clName = "rename16139";
	private String srcGroupName = null;
	private String targetGroupName = null;	
	private DBCollection cl = null;
	private Sequoiadb sdb = null;	
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		if(CommLib.isStandAlone(sdb)){
			throw new SkipException("standAlone skip testcase16128");
		}
		
		if (CommLib.OneGroupMode(sdb)){
			throw new SkipException("less than two groups skip testcase16139");
		}
		RenameUtil.removeCS(sdb, newCSName);
		
		List<String> rgNames = CommLib.getDataGroupNames(sdb);		
		srcGroupName = rgNames.get(0);
		targetGroupName = rgNames.get(1);
		CollectionSpace cs = RenameUtil.createCS(sdb, csName);
		String clOptions = "{ShardingKey:{no:1},ShardingType:'range',"
				+ "ReplSize:0,Compressed:true}, Group:" + srcGroupName + "}";
		cl = RenameUtil.createCL(cs, clName, clOptions);
		
		insertDatas( cl );
	}
	
	@Test
	public void test(){ 
		SplitCLThread splitCLThread = new SplitCLThread();		
		RenameCSThread renameCSThread = new RenameCSThread();
		splitCLThread.start();		
		renameCSThread.start();				
		
		if(renameCSThread.isSuccess()){	
			Assert.assertTrue(!splitCLThread.isSuccess(),splitCLThread.getErrorMsg() );
			BaseException e=(BaseException)(splitCLThread.getExceptions().get(0));			
			if ( e.getErrorCode() != -147 && e.getErrorCode() != -172 
					&& e.getErrorCode() != -23  ){
				Assert.fail("split fail:"+splitCLThread.getErrorMsg()+ "  e:"+e.getErrorCode());
			}
			
			int clNums = 1;
			RenameUtil.checkRenameCSResult(sdb, csName, newCSName, clNums);
			checkSplitFailResult( sdb );
		}else {
			Assert.assertTrue(splitCLThread.isSuccess(),splitCLThread.getErrorMsg() );			
			BaseException e=(BaseException)(renameCSThread.getExceptions().get(0));	
			if ( e.getErrorCode() != -33 && e.getErrorCode() != -334 ){
				Assert.fail("renameCS fail:"+renameCSThread.getErrorMsg() + "  e:"+e.getErrorCode());
			}
			
			checkSplitResult(sdb);
			checkRenameFailResult( csName, newCSName );
		}	
	}
	
	@AfterClass
	public void tearDown(){
		try{					
			RenameUtil.removeCS(sdb, newCSName);	
			RenameUtil.removeCS(sdb, csName);
		}catch(BaseException e){			
			Assert.assertTrue(false,"clean up failed:"+e.getMessage());
		}finally {
			if( sdb != null )
			sdb.close();
		}
	}
	
	public class RenameCSThread extends SdbThreadBase{
		@Override
		public void exec() throws BaseException {	
			try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
				//random wait time to create cs
				Random random = new Random();
				int result = random.nextInt(2000);
				try {
					Thread.sleep(result);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
						
				db.renameCollectionSpace(csName, newCSName);				
			}
		}
	}
	
	public class SplitCLThread extends SdbThreadBase{
		@Override
		public void exec() throws Exception {
			
			try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
				DBCollection dbcl = db.getCollectionSpace(csName).getCollection(clName);				
				int present = 80;
				dbcl.split(srcGroupName, targetGroupName, present);				
			}
		}
	}
	
	private void insertDatas(DBCollection dbcl){
		ArrayList<BSONObject> insertRecods = new ArrayList<BSONObject>();
		for ( int i = 0; i < 10000; i++){
			 BSONObject obj = new BasicBSONObject();				
			 obj.put("testa", "test"+i);					 
			 String str = "32345.067891234567890123456789" + i;
			 BSONDecimal decimal = new BSONDecimal(str);			
			 obj.put("decimala",decimal);
			 obj.put("no", i);	
			 obj.put("flag", i);	
			 obj.put("str", "test_" + String.valueOf(i));
			 insertRecods.add(obj);			 
		 }
		dbcl.insert(insertRecods);
	}
	
	private void checkSplitFailResult( Sequoiadb sdb ){	
		try (Sequoiadb dataNode = sdb.getReplicaGroup(targetGroupName).getMaster().connect()){			
			dataNode.getCollectionSpace(csName).getCollection(clName);			
			Assert.fail(" the cl should not be in the targetGroup!");			
		}catch(BaseException e){
			if(e.getErrorCode() != -34){
				Assert.fail(" get cl fail by targetGroupName:" + e.getErrorCode());
			}
		}
	}
	
	private void checkRenameFailResult( String oldCSName, String newCSName ){
		if( sdb.isCollectionSpaceExist(newCSName)){
			Assert.fail(" the new cs should not exist!");
		}
		if( !sdb.isCollectionSpaceExist(csName)){
			Assert.fail(" the old cs should be exist!");
		}
	}
	
	private void checkSplitResult( Sequoiadb sdb ){		
		String srcGroupMather = "{no:{$gte:0,$lt:2000}}";
		long srcGroupCount = 2000;
		checkGroupData(sdb, srcGroupCount, srcGroupMather, srcGroupName);
		
		String targetGroupMather = "{no:{$gte:2000,$lt:10000}}";
		long targetGroupCount = 8000;
		checkGroupData(sdb, targetGroupCount, targetGroupMather, targetGroupName);
		
		long expectTotalCount = 10000;
		long actCount = cl.getCount();
		Assert.assertEquals(actCount, expectTotalCount, " count after split is incorrect! ");
		
	}
	
	private void checkGroupData(Sequoiadb sdb, long expectedCount, String macher, String groupName)	{	
		try (Sequoiadb dataNode = sdb.getReplicaGroup(groupName).getMaster().connect()){			
			DBCollection dbcl = dataNode.getCollectionSpace(csName).getCollection(clName);			
			long count = dbcl.getCount(macher);
			if (count != expectedCount) {
				Assert.fail("check data are incorrect!"+ groupName + " count is  " + count);
			}			
		} 
	}	
}
