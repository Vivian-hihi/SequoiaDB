package com.sequoiadb.lob.randomwrite;

import java.util.Arrays;

import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.lob.randomwrite.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* FileName: RewriteLob13241.java
* test content:lock the data segment to write lob,and locking data segment range discontinuity
* testlink case:seqDB-13241
* @author wuyan
    * @Date    2017.11.7
* @version 1.00
*/
public class RewriteLob13241 extends SdbTestBase {
	private String clName = "writelob13241";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
	private static DBCollection cl = null; 
	private byte[] testLobBuff= null;
	
    	
	@BeforeClass
	public void setUp(){				
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){			
			Assert.assertTrue(false,"connect %s failed,"+SdbTestBase.coordUrl+e.getMessage());
		}	
		
		sdb.setSessionAttr(new BasicBSONObject("PreferedInstance", "M"));
		cs = sdb.getCollectionSpace(SdbTestBase.csName);		
		String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024,"
				+ "ReplSize:0,Compressed:true}";
		cl = RandomWriteLobUtil.createCL(cs, clName, clOptions );		
	}	
	
	@Test
	public void testLob(){
		int writeSize = 1024*1024*35;
		testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, testLobBuff);
			
		int[] offset = {1024, 524288, 26214400, 46214400};
		int[] writeLobSize = {260096, 524289, 262144, 2621440};
		long actLobSize = rewriteLob(oid, offset, writeLobSize);
		checkLobSize(actLobSize);
		checkLobDataResult(oid, offset, writeLobSize);		
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
	
	private void checkLobDataResult(ObjectId oid, int[] offset, int[] writeLength){	
		
		//check the offset write lob 	
		try (DBLob lob = cl.openLob(oid)) {
			for( int i = 0; i< 4; i++){
				byte[] actLobBuff = new byte[writeLength[i]];
				lob.seek(offset[i], DBLob.SDB_LOB_SEEK_SET);
		        lob.read(actLobBuff);
		        System.out.println("actLobBuff=="+actLobBuff.length);
		        byte[] expBuff = Arrays.copyOfRange(testLobBuff, offset[i], offset[i]+writeLength[i]);
		        System.out.println("expBuff=="+expBuff.length);
		        Assert.assertEquals(actLobBuff, expBuff,"the lob offset="+offset[i]+" different");		           	    	
			}
		}	
	    
	}
	
	private void checkLobSize(long actLobSize){		
		//check the all write lob 		
		long expLobSize = 48835840;
		Assert.assertEquals(actLobSize, expLobSize,"the lobsize is different!");
	}
	
	private long rewriteLob(ObjectId oid, int offset[], int writeLobSize[]){			
		long actWriteLobSize = 0;
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)){				
			for( int i = 0; i< 4; i++){
				byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(writeLobSize[i]);
				lob.lockAndSeek(offset[i], writeLobSize[i]);
				lob.write(rewriteBuff);
				testLobBuff = RandomWriteLobUtil.appendBuff(testLobBuff, rewriteBuff, offset[i]);
			}	
			
			actWriteLobSize = lob.getSize();			
		}catch(BaseException e){			
			Assert.assertTrue(false,"rewrite lob fail"+e.getMessage());
		}
		return actWriteLobSize;
	}	
}


