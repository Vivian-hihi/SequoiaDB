package com.sequoiadb.lob.randomwrite;

import org.bson.types.ObjectId;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.lob.randomwrite.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* FileName: RewriteLob13238.java
* test content:lock the data segment to write lob,set different offset positions to covering rewrite lob
* testlink case:seqDB-13238
* @author wuyan
    * @Date    2017.11.2
* @version 1.00
*/
public class RewriteLob13238 extends SdbTestBase {	
	@DataProvider(name = "pagesizeProvider", parallel = true)
	public Object[][] generatePageSize(){
		return new Object[][]{
			//the parameter is  writeLobSize, offset, rewriteLobSize
			//start from the start position
			new Object[]{1024*1024, 0, 1024*512},
			//start from the middle position
			new Object[]{1024*1024, 1024*512, 1024*1024},
			//start from the end postition
			new Object[]{1024*512,  1024*512, 1024*4},			
		};
	}	
	
	private String clName = "writelob13238";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
    	
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
		RandomWriteLobUtil.createCL(cs, clName, clOptions );
		
	}	
	
	@Test(dataProvider = "pagesizeProvider")
	public void testLob( int writeLobSize, int offset, int rewriteLobSize){	
		
		try( Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
			DBCollection dbcl = sdb.getCollectionSpace(SdbTestBase.csName).getCollection(clName);			
			byte[] lobBuff = RandomWriteLobUtil.getRandomBytes(writeLobSize);
			ObjectId oid = RandomWriteLobUtil.createAndWriteLob(dbcl, lobBuff);
			
			byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);			
			rewriteLob(dbcl, oid, offset, rewriteBuff);			
			checkResult( dbcl, oid, offset, lobBuff, rewriteBuff);				
		}catch(BaseException e){
			Assert.assertTrue(false,"no seek to write lob fail:"+e.getMessage()+e.getStackTrace());	
		}
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
	
	
	private void checkResult( DBCollection cl, ObjectId oid, int offset, byte[] lobBuff, byte[] rewriteBuff) {		
		//check the rewrite lob 
		byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, rewriteBuff.length, offset);		
		RandomWriteLobUtil.assertByteArrayEqual(actBuff, rewriteBuff);
		
		//check the all write lob 
		byte[] expBuff = RandomWriteLobUtil.appendBuff(lobBuff, rewriteBuff, offset);
		byte[] actAllLobBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, expBuff.length, 0);
		RandomWriteLobUtil.assertByteArrayEqual(actAllLobBuff, expBuff);		
	}	
	
	private void rewriteLob(DBCollection cl,ObjectId oid,int offset,byte[] rewriteBuff){		
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)){				
			lob.lockAndSeek(offset, rewriteBuff.length);
			lob.write(rewriteBuff);			    
		}catch(BaseException e){			
			Assert.assertTrue(false,"write lob fail"+e.getMessage());
		}			
	}
	
}


