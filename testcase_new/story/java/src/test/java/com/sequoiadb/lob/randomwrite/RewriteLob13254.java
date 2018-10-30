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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.lob.randomwrite.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* FileName: RewriteLob13254.java
* test content:read empty lob piece
* testlink case:seqDB-13254
* @author wuyan
    * @Date    2017.11.7
* @version 1.00
*/
public class RewriteLob13254 extends SdbTestBase {
	private String clName = "writelob13254";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
	private static DBCollection cl = null; 	
	private byte[] testLobBuff= null;
	private byte[] testWriteBuff= null;
	
    	
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
		int writeSize = 261120;
		testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, testLobBuff);		
		rewriteLob(oid);
		checkWriteLobResult(oid);
		
		readLobFromNullPieces(oid);
		readContainsNullPieces(oid);	
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
	
	private void checkWriteLobResult(ObjectId oid){
		try (DBLob lob = cl.openLob(oid)) {
	    	byte[] actAllLob = new byte[(int)lob.getSize()];
	           lob.read(actAllLob);
	           if(!Arrays.equals(actAllLob, testWriteBuff)){
	        	   RandomWriteLobUtil.writeLobAndExpectData2File(lob, testWriteBuff);
	        	   Assert.fail("check actlob and expbuff different");	   	    	
	   	    }
	    }
	}	
	
	private void rewriteLob(ObjectId oid){	
		//contain 1 and 2 pieces
		int offset1 = 261120;
		int lobSize = 524288;
		//contian 5 and 6 pieces
		int offset2 = 1309696;		
		byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(lobSize);	
		
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);){				
			lob.lockAndSeek(offset1, lobSize);
			lob.write(rewriteBuff);			
			lob.lockAndSeek(offset2, lobSize);
			lob.write(rewriteBuff);				
			byte[] testBuff = RandomWriteLobUtil.appendBuff(testLobBuff, rewriteBuff, offset1);			
			testWriteBuff = RandomWriteLobUtil.appendBuff(testBuff, rewriteBuff, offset2);				   
		}catch(BaseException e){
			Assert.assertTrue(false,"rewrite lob fail"+e.getMessage());
		}		
	}	
	
	
	private void readLobFromNullPieces(ObjectId oid){		
		byte[] rbuff = null;
		try(DBLob rLob = cl.openLob(oid);)
		{		
			int offset = 785408;
			rbuff = new byte[ 262144];
			rLob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
			rLob.read(rbuff);	
			byte[] testnullBuff = new byte[ 262144];		
			RandomWriteLobUtil.assertByteArrayEqual(rbuff, testnullBuff);
		}catch(BaseException e){
			Assert.assertTrue(false,"read null pieces fail"+e.getMessage());
		}	
	}
	
	private void readContainsNullPieces(ObjectId oid){		
		byte[] rbuff = null;
		try(DBLob rLob = cl.openLob(oid);)
		{		
			//read the 4 and 5 pieces, and the 4 pieces is null
			int offset = 1047552;
			int length = 524288;
			rbuff = new byte[ length];
			rLob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
			rLob.read(rbuff);	
			byte[] containsNullBuff = Arrays.copyOfRange( testWriteBuff, offset, offset + length );
			RandomWriteLobUtil.assertByteArrayEqual(rbuff, containsNullBuff);
		}catch(BaseException e){
			Assert.assertTrue(false,"read null pieces fail"+e.getMessage());
		}	
	}
	
	
}


