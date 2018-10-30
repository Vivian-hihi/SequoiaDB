package com.sequoiadb.lob.randomwrite;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.lob.randomwrite.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
* FileName: ConcurrentRewriteLob13270.java
* test content:lock the data segment to write lob,test the lob pieces size boundary value
* testlink case:seqDB-13270
* @author wuyan
    * @Date    2017.11.8
* @version 1.00
*/
public class ConcurrentRewriteLob13270 extends SdbTestBase {	
	private String clName = "writelob13270";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
	private static DBCollection cl = null; 
	private static ObjectId oid = null;	
	private byte[] testLobBuff= null;
	
	CommLib CommLib = new CommLib();
	private String sourceRGName;
	private String targetRGName;
    	
	@BeforeClass
	public void setUp(){				
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){			
			Assert.assertTrue(false,"connect %s failed,"+SdbTestBase.coordUrl+e.getMessage());
		}		
		
		if (CommLib.isStandAlone(sdb)){
			throw new SkipException("is standalone skip testcase");
		}
		
		if (CommLib.OneGroupMode(sdb)){
			throw new SkipException("less two groups skip testcase");
		}
		
		sdb.setSessionAttr(new BasicBSONObject("PreferedInstance", "M"));
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024,"
				+ "ReplSize:0,Compressed:true}";
		cl = RandomWriteLobUtil.createCL(cs, clName, clOptions );
		
		//put lob		
		int writeSize = 1024 * 1024 * 2;
		testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		oid = RandomWriteLobUtil.createAndWriteLob(cl, testLobBuff);
	}	
	
	
	@Test
	public void testLob(){	
		int offset = 1024 * 1024;
		int rewriteLobSize = 1024 * 1024;		
		List<LockAndRewriteLobTask> rewriteLobTasks = new ArrayList<>(10);
		byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);
		for( int i = 0; i < 20; i++){			
			rewriteLobTasks.add( new LockAndRewriteLobTask(offset, rewriteBuff));
			offset = offset + rewriteLobSize + 10240;			
		}
		
		for( LockAndRewriteLobTask rewriteLobTask: rewriteLobTasks ){			
			rewriteLobTask.start();
		}
		SplitTask splitTask = new SplitTask(); 
		splitTask.start();
		
		for( LockAndRewriteLobTask rewriteLobTask: rewriteLobTasks ){
			rewriteLobTask.join();
		}
		splitTask.join();
		
		Assert.assertTrue(splitTask.isSuccess(), splitTask.getErrorMsg());
		for( LockAndRewriteLobTask rewriteLobTask: rewriteLobTasks ){
			Assert.assertTrue( rewriteLobTask.isSuccess(), rewriteLobTask.getErrorMsg());
		}
		
		//check write result
		readLobAndcheckWriteResult( rewriteBuff);
		//check split result
		checkSplitResult();
	
	}
	
	
	@AfterClass
	public void tearDown(){		
		try{					
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}
			sdb.close();
		}catch(BaseException e){			
			Assert.assertTrue(false,"clean up failed:"+e.getMessage());
		}
	}		
	
	
	public class SplitTask extends SdbThreadBase{
		@Override
        public void exec() throws BaseException{             
            sourceRGName = RandomWriteLobUtil.getSrcGroupName(sdb, SdbTestBase.csName, clName);
			targetRGName = RandomWriteLobUtil.getSplitGroupName(sdb, sourceRGName);
            try(Sequoiadb db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "")){            	
            	DBCollection cl1 = db1.getCollectionSpace(SdbTestBase.csName).getCollection(clName);            	
				int persent = 80;
				cl1.split(sourceRGName, targetRGName,persent);
            }catch(BaseException e){
            	Assert.assertTrue(false,"split fail\n"+"srcGroup:"+sourceRGName
									+"\ntarGroup"+targetRGName+e.getMessage());
            }	
		}
	}	
	
	private class LockAndRewriteLobTask extends SdbThreadBase {
		private int offset;
		private byte[] rewriteLobBuff;
		
		public LockAndRewriteLobTask(int offset, byte[] rewriteLobBuff) {
			this.offset = offset;
			this.rewriteLobBuff = rewriteLobBuff;
		}
		@Override
		public void exec() throws Exception {
		    DBLob lob = null;		    
		    try(Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")){	
		    	DBCollection cl = sdb.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
		    	lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);
		    	lob.lockAndSeek(offset, rewriteLobBuff.length);
		    	lob.write(rewriteLobBuff);		    	
		    	lob.close();
		    	updateExpBuff(rewriteLobBuff, offset);
		    } 		    
		}		
	}
	
	synchronized private void updateExpBuff( byte[] rewriteBuff, int offset){
		testLobBuff = RandomWriteLobUtil.appendBuff(testLobBuff, rewriteBuff, offset);
	}
	
	private void readLobAndcheckWriteResult( byte[] rewriteBuff) {	
		
		//check the rewrite lob 		
		int expOffset = 1024 * 1024;
		for( int i = 0; i < 20; i++){	
			byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, rewriteBuff.length, expOffset);
			expOffset = expOffset + rewriteBuff.length + 10240;
			RandomWriteLobUtil.assertByteArrayEqual(actBuff, rewriteBuff);
		}		
		
		//check write lob before split
		int lobsize = 1024 * 1024;
		byte[] expBuff = Arrays.copyOfRange(testLobBuff, 0, lobsize);
		byte[] rbuff =  new byte[lobsize];
		try( DBLob rlob = cl.openLob(oid)){	
			rlob.seek(0,DBLob.SDB_LOB_SEEK_SET);
			rlob.read(rbuff);
		}		
		RandomWriteLobUtil.assertByteArrayEqual(rbuff, expBuff);		
	}	
	
	/**
	* construct expected result values
	* @return expected result values,rg:["group1","{"":0}","{"":500}"]
	*/
	private List<CataInfoItem> buildExpectResult(){
		List<CataInfoItem> cataInfo = new ArrayList<CataInfoItem>();
		CataInfoItem item  = new CataInfoItem();
		item.groupName = sourceRGName;
		item.lowBound = 0;
		item.upBound = 205;
		
		cataInfo.add(item);
		item  = new CataInfoItem();
		item.groupName = targetRGName;
		item.lowBound = 205;
		item.upBound = 1024;
		cataInfo.add(item);		
		return cataInfo;
	}
	
	private void checkSplitResult(){
		String cond = String.format("{Name:\"%s.%s\"}", SdbTestBase.csName, clName);	
		DBCursor collections = sdb.getSnapshot(8, cond, null, null);
		List<CataInfoItem> cataInfo = buildExpectResult();
		while(collections.hasNext()){
			BasicBSONObject doc = (BasicBSONObject)collections.getNext();				 
			doc.getString("Name");
			BasicBSONList subdoc = (BasicBSONList)doc.get("CataInfo");
			for (int i = 0; i < cataInfo.size(); ++i){
				BasicBSONObject elem = (BasicBSONObject)subdoc.get(i);
				String groupName = elem.getString("GroupName");
				BasicBSONObject obj = (BasicBSONObject)elem.get("LowBound");
				int LowBound;
				if (obj.containsField("")){
					LowBound = obj.getInt("");
				}else{
					LowBound = obj.getInt("partition");
				}
				
				int UpBound;				
				obj = (BasicBSONObject)elem.get("UpBound");
				if (obj.containsField("")){
					UpBound = obj.getInt("");
				}else{
					UpBound = obj.getInt("partition");
				}			
				
				boolean compareResult = cataInfo.get(i).Compare(groupName, LowBound, UpBound);				
				Assert.assertTrue(compareResult, cataInfo.get(i).toString()+"actResult:"
						+"groupName:"+groupName+" LowBound:"+LowBound+" UpBound:"+UpBound);				
			}		
		}
	}
	
	private class CataInfoItem {
		public String groupName;
		public int lowBound;
		public int upBound;
		
		public boolean Compare(String name, int low, int up){
			return name.equals(groupName) && low == lowBound && 
				   up == upBound;
		}
		
		public String toString(){
			return "groupName : " + groupName + " lowBound: {'':" + lowBound + "}"
					+ " upBound:{'':" + upBound + "}";
		}
	}
}
	
	



