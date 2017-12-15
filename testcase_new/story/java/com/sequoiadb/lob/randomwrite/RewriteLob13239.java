package com.sequoiadb.lob.randomwrite;

import java.util.Arrays;
import java.util.Random;

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
* FileName: RewriteLob13239.java
* test content:lock the data segment to write lob,the lock offset exceeds the maximum length of lob
* testlink case:seqDB-13239
* @author wuyan
    * @Date    2017.11.7
* @version 1.00
*/
public class RewriteLob13239 extends SdbTestBase {	
	private String clName = "writelob13239";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
	private static DBCollection cl = null; 
	private Random random = new Random();	
    	
	@BeforeClass
	public void setUp(){				
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){			
			Assert.assertTrue(false,"connect %s failed,"+SdbTestBase.coordUrl+e.getMessage());
		}	
		
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024,"
				+ "ReplSize:0,Compressed:true}";
		cl = RandomWriteLobUtil.createCL(cs, clName, clOptions );		
	}	
	
	@Test
	public void testLob(){		
		int writeSize = random.nextInt(1024 * 1024 * 2);
		byte[] lobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, lobBuff);
		
		int offset = writeSize + 1024;
		int rewriteLobSize = random.nextInt(1024 * 1024 * 2);
		byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);	
		rewriteLob(oid, offset, rewriteBuff);
		checkResult(oid, offset, lobBuff, rewriteBuff);		
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
	
	
	private void checkResult( ObjectId oid, int offset, byte[] lobBuff, byte[] rewriteBuff) {		
		//check the rewrite lob 
		byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, rewriteBuff.length, offset);		
		RandomWriteLobUtil.assertByteArrayEqual(actBuff, rewriteBuff);
		
		//check the the old write lob,the old lob must be ok						
	    try (DBLob lob = cl.openLob(oid)) {
	    	byte[] actOldLob = new byte[lobBuff.length];
	    	   lob.seek(0, DBLob.SDB_LOB_SEEK_SET);
	           lob.read(actOldLob);
	           if(!Arrays.equals(actOldLob, lobBuff)){
	        	   RandomWriteLobUtil.writeLobAndExpectData2File(lob, lobBuff);
	        	   Assert.fail("check actlob and expbuff different");	   	    	
	   	    }
	    }
	}	
	
	private void rewriteLob(ObjectId oid,int offset,byte[] rewriteBuff){		
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)){				
			lob.lockAndSeek(offset, rewriteBuff.length);
			lob.write(rewriteBuff);			    
		}catch(BaseException e){			
			Assert.assertTrue(false,"write lob fail"+e.getMessage());
		}			
	}
	
	
}


