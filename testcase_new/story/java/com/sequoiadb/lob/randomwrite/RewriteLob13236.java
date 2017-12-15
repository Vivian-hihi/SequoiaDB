package com.sequoiadb.lob.randomwrite;

import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
* FileName: TestWriteLob13236.java
* test content:lock the data segment to write lob,test the lob pieces size boundary value
* testlink case:seqDB-13236
* @author wuyan
    * @Date    2017.11.2
* @version 1.00
*/
public class RewriteLob13236 extends SdbTestBase {
	@DataProvider(name = "pagesizeProvider")
	public Object[][] generatePageSize(){
		int lobPageSizes[] = {4096,8192,16384,32768,65536,131072,262144,524288};
		int len = lobPageSizes.length;
		Random random = new Random();		
		int num = lobPageSizes[random.nextInt(len-1)];
		return new Object[][]{		
			//the paramter is lobPageSize, write offset,write lob length	
			//sharding after just over 1 pages
			new Object[]{num, num-1024, num * 2},
		    //sharding after less than 1byte, lobsize is num-1
			new Object[]{num, num-1024, num - 1},
	        //sharding after more than 1kb
			new Object[]{num, num-1024, num + 1},
		};
	}
	
	private String csName = "lobcs13236";
	private String clName = "writelob13236";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;		
    	
	@BeforeClass
	public void setUp(){				
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){			
			Assert.assertTrue(false,"connect %s failed,"+SdbTestBase.coordUrl+e.getMessage());
		}		
	}	
	
	@Test(dataProvider = "pagesizeProvider")
	public void testLob(int lobPageSize, int offset,int length){	
		DBCollection cl = createCL(lobPageSize);	
		//long writeSize = 1048576;	
		int writeSize = 1048576;	
		byte[] lobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, lobBuff);
		
		byte[] rewritelobBuff = RandomWriteLobUtil.getRandomBytes(length);
		rewriteLob(cl, oid, offset, rewritelobBuff);
		checkResult(cl, oid, offset,rewritelobBuff, lobBuff);			
	}
	
	@AfterClass
	public void tearDown(){		
		try{					
			if(sdb.isCollectionSpaceExist(csName)){
				dropCS();
			}
			sdb.close();
		}catch(BaseException e){			
			Assert.assertTrue(false,"clean up failed:"+e.getMessage());
		}
	}	
		
	private void rewriteLob(DBCollection cl,ObjectId oid, int offset,byte[] rewriteBuff){		
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)){			
			lob.lockAndSeek(offset, rewriteBuff.length);
			lob.write(rewriteBuff);			
		}catch(BaseException e){			
			Assert.assertTrue(false,"rewrite lob fail"+e.getMessage());
		}				
	}	
		
	private void checkResult(DBCollection cl,ObjectId oid,int offset, byte[] rewriteBuff, byte[] lobBuff){		
		//check the rewrite lob 
		byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, rewriteBuff.length, offset);		
		RandomWriteLobUtil.assertByteArrayEqual(actBuff, rewriteBuff);
		
		//check the all write lob 
		byte[] expBuff = RandomWriteLobUtil.appendBuff(lobBuff, rewriteBuff, offset);		
		byte[] actAllLobBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, expBuff.length, 0);
				
	    	    
		RandomWriteLobUtil.assertByteArrayEqual(actAllLobBuff, expBuff);
	}
		
	private DBCollection createCL(int lobPagesize){
		if (sdb.isCollectionSpaceExist(csName)){
			sdb.dropCollectionSpace(csName);
		}
		
		BSONObject options = new BasicBSONObject();
		options.put("LobPageSize", lobPagesize);	
		DBCollection cl = null;
		try
		{
			cs = sdb.createCollectionSpace(csName, options);	
			cl = cs.createCollection(clName);
		}catch(BaseException e){
			Assert.assertTrue(false,"create CS/CL fail "+e.getErrorType()+":"+e.getMessage());			
		}
		return cl;
	}
	
	private void dropCS(){
		try{
			sdb.dropCollectionSpace(csName);		
		}catch(BaseException e){
			Assert.assertTrue(false,"create CS/CL fail "+e.getErrorType()+":"+e.getMessage());
		}
	}	
	
	
	
	
}


