package com.sequoiadb.lob.randomwrite;

import org.bson.types.ObjectId;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* @Description  seqDB-13246:lock the all lob to write,set different offset positions to covering rewrite lob
* @author wuyan
* @Date    2019.07.16
* @version 1.00
*/
public class RewriteLob13246 extends SdbTestBase {	
	@DataProvider(name = "pagesizeProvider", parallel = true)
	public Object[][] generatePageSize(){
		return new Object[][]{
			//the parameter is  writeLobSize, offset, rewriteLobSize
			//start from the start position
			new Object[]{1024*1024, 0, 1024 * 1024 * 2},
			//start from the middle position
			new Object[]{1024*1024, 1024*512, 1024 * 1024},
			//start from the end postition
			new Object[]{1024*512,  1024*512, 1024*4},			
		};
	}	
	
	private String clName = "writelob13246";	
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
	public void testLob( int writeLobSize, int offset, int rewriteLobSize){		
		try( Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
			DBCollection dbcl = sdb.getCollectionSpace(SdbTestBase.csName).getCollection(clName);			
			byte[] lobBuff = RandomWriteLobUtil.getRandomBytes(writeLobSize);
			ObjectId oid = RandomWriteLobUtil.createAndWriteLob(dbcl, lobBuff);
			
			byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);			
			lockAndRewriteLob(dbcl, oid, offset, rewriteBuff);			
			RandomWriteLobUtil.checkRewriteLobResult( dbcl, oid, offset, rewriteBuff, lobBuff);				
		}
	}
	
	@AfterClass
	public void tearDown(){		
		try{					
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}
			sdb.close();
		}finally{
			if( sdb != null ){
				sdb.close();
			}
		}
	}
	
	private void lockAndRewriteLob(DBCollection cl,ObjectId oid,int offset,byte[] rewriteBuff){	
		long lockLength = -1;
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)){				
			lob.lockAndSeek(offset, lockLength);
			lob.write(rewriteBuff);			    
		}			
	}
	
}


