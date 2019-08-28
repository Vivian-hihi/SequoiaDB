package com.sequoiadb.lob.randomwrite;

import java.util.Arrays;
import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* @Description seqDB-13244:multiple execution of locked data segments to write lob operations,
*                          and locking data segment range discontinuity
* @author wuyan
* @Date    2017.11.7
* @version 1.00
*/
public class RewriteLob13244 extends SdbTestBase {
	private String clName = "writelob13244";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
	private static DBCollection cl = null; 
	private byte[] testLobBuff= null;
	
    	
	@BeforeClass
	public void setUp(){		
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");		
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024}";
		cl = RandomWriteLobUtil.createCL(cs, clName, clOptions );		
	}	
	
	@Test
	public void testLob(){			
		int writeSize = 1024 * 2;
		testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, testLobBuff);			
		rewriteLob(oid);
		checkAllLobResult(oid);		
	}
	
	@AfterClass
	public void tearDown(){		
		try{					
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}			
		}finally{
			if(sdb != null){
				sdb.close();
			}
		}
	}	
	
	private void checkAllLobResult(ObjectId oid){		
		//check the all write lob 
		try (DBLob lob = cl.openLob(oid)) {
	    	byte[] actAllLob = new byte[(int)lob.getSize()];
	           lob.read(actAllLob);
	           if(!Arrays.equals(actAllLob, testLobBuff)){
	        	   RandomWriteLobUtil.writeLobAndExpectData2File(lob, testLobBuff);
	        	   Assert.fail("check actlob and expbuff different");	   	    	
	   	    }
	    }
	}	
	
	private void rewriteLob(ObjectId oid){	
		int offset1 = 1024;
		int lobSize1 = 1024 * 254;
		int offset2 = 1024 * 5;
		int lobSize2 = 1024 * 1024;	
		byte[] rewriteBuff1 = RandomWriteLobUtil.getRandomBytes(lobSize1);
		byte[] rewriteBuff2 = RandomWriteLobUtil.getRandomBytes(lobSize2);			
		
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)){				
			lob.lockAndSeek(offset1, lobSize1);
			lob.write(rewriteBuff1);
			
			lob.lockAndSeek(offset2, lobSize2);			
			lob.write(rewriteBuff2);			
		}
		
		//write to compare buff
		testLobBuff = RandomWriteLobUtil.appendBuff(testLobBuff, rewriteBuff1, offset1);
		testLobBuff = RandomWriteLobUtil.appendBuff(testLobBuff, rewriteBuff2, offset2);
	 
		//the intersection part is covered and written
		byte[] coverWriteLobBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, rewriteBuff2.length, offset2);
		RandomWriteLobUtil.assertByteArrayEqual(coverWriteLobBuff, rewriteBuff2);		
	}	
}


