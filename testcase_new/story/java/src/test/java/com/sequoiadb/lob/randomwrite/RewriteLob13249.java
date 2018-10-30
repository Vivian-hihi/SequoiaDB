package com.sequoiadb.lob.randomwrite;

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
* FileName: RewriteLob13249.java
* test content:write empty pieces over limits, 
* 					record empty pieces metadata information no more than 320 bytes ,
*                   No more than 40 empty pieces   
* testlink case:seqDB-13249
* @author wuyan
    * @Date    2017.11.7
* @version 1.00
*/
public class RewriteLob13249 extends SdbTestBase {
	private String clName = "writelob13249";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
	private static DBCollection cl = null; 
    	
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
		byte[] testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, testLobBuff);			
		rewriteLob(oid);	
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
		
	
	private void rewriteLob(ObjectId oid){		
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);){	
			int rewriteLobSize = 262144;
			for( int i=0; i<40; i++){					
				int offset = (i*2+2) * 524288;				
				byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);
				lob.lockAndSeek(offset, rewriteLobSize);
				lob.write(rewriteBuff);							
			}	
			Assert.fail("the number of empty pieces exceeds the limit to be reported wrong");
		}catch(BaseException e){
			if( -319 != e.getErrorCode()){
				Assert.assertTrue(false,"empty pieces num limit check faild"+e.getMessage());
			}			
		}				
	}		
}


