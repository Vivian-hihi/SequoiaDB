package com.sequoiadb.lob.randomwrite;

import java.util.Random;

import org.bson.types.ObjectId;
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
* @Description seqDB-13239:lock the data segment to write lob,the lock offset exceeds the maximum length of lob
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
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");		
		cs = sdb.getCollectionSpace(SdbTestBase.csName);
		String clOptions = "{ShardingKey:{no:1},ShardingType:'hash'}";
		cl = RandomWriteLobUtil.createCL(cs, clName, clOptions );		
	}	
	
	@Test
	public void testLob(){		
		int writeSize = random.nextInt(1024 * 1024 * 2);
		byte[] lobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
		ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, lobBuff);
		
		int offset = writeSize + random.nextInt(1024 * 254 );
		int rewriteLobSize = random.nextInt(1024 * 1024 * 2);
		byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize);	
		rewriteLob(oid, offset, rewriteBuff);	
		//check the rewrite lob 
		byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, rewriteBuff.length, offset);		
		RandomWriteLobUtil.assertByteArrayEqual(actBuff, rewriteBuff);
		//check the old lob 
		byte[] actOldLobBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, lobBuff.length, 0);		
		RandomWriteLobUtil.assertByteArrayEqual(actOldLobBuff, lobBuff);			
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
	
	private void rewriteLob(ObjectId oid,int offset,byte[] rewriteBuff){		
		try(DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)){				
			lob.lockAndSeek(offset, rewriteBuff.length);
			lob.write(rewriteBuff);			    
		}			
	}
	
	
}


