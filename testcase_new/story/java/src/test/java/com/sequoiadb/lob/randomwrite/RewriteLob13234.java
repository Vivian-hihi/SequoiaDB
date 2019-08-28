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
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* @Description seqDB-13234:no seek to write lob of write mode* 
* @author wuyan
* @Date    2017.11.2
* @version 1.00
*/
public class RewriteLob13234 extends SdbTestBase {
	@DataProvider(name = "pagesizeProvider", parallel = true)
	public Object[][] generatePageSize(){
		return new Object[][]{
			//the parameter is  writeLobSize, rewriteLobSize
			//the writeLobSize > rewriteLobSize
			new Object[]{1024*1024*2, 1024*512},
			//the writeLobSize = rewriteLobSize
			new Object[]{1024*1024, 1024*1024},
			//the writeLobSize < rewriteLobSize
			new Object[]{1024*256,  1024*512},			
		};
	}
	
	private String clName = "writelob13234";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
    	
	@BeforeClass
	public void setUp(){		
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");		
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024,"
				+ "ReplSize:0}";	
		RandomWriteLobUtil.createCL(cs, clName, clOptions );
	}	
		
	@Test(dataProvider = "pagesizeProvider")
	public void testLob( int writeLobSize, int rewriteLobSize){	
		try( Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
			DBCollection dbcl = sdb.getCollectionSpace(SdbTestBase.csName).getCollection(clName);			
			byte[] lobBuff = RandomWriteLobUtil.getRandomBytes(writeLobSize);
			ObjectId oid = RandomWriteLobUtil.createAndWriteLob(dbcl, lobBuff);
			
			byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);			
			writeLob(dbcl, oid, rewriteBuff);			
			RandomWriteLobUtil.checkRewriteLobResult( dbcl, oid, 0, rewriteBuff,lobBuff );
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
		
	private void writeLob(DBCollection cl, ObjectId oid, byte[] rewriteBuff){				
		try(DBLob lob = cl.openLob(oid,DBLob.SDB_LOB_WRITE )){			
			lob.write(rewriteBuff);					    	    
		}			
	}	
}


