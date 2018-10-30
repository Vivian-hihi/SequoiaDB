package com.sequoiadb.lob.randomwrite;

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
* FileName: TestWriteLob13229.java
* test content:lock the data segment to write lob of createonly mode 
* testlink case:seqDB-13229
* @author wuyan
    * @Date    2017.11.2
* @version 1.00
*/
public class WriteLob13229 extends SdbTestBase {
	private String clName = "writelob13229";	
	private static Sequoiadb sdb = null;
	private CollectionSpace cs = null;	
	private static DBCollection cl = null; 
	private static ObjectId oid = null;		
	private Random random = new Random();
	private byte[] lobBuff = null;
    	
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
		int writeSize = random.nextInt(1048576);
		int offset = random.nextInt(1048576);

		ObjectId oid = putLob(writeSize, offset);
		checkResult( oid, writeSize, offset );		
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
	
	private ObjectId putLob(int writeSize,int offset){		
		lobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);			
		try(DBLob lob = cl.createLob() ){				    
			long length = writeSize - 2;
			lob.lockAndSeek(offset, length);
			lob.write(lobBuff);		
			
		    oid = lob.getID();		    
		}catch(BaseException e){			
			Assert.assertTrue(false,"write lob fail"+e.getMessage());
		}		
		return oid;				
	}
	
	private void checkResult( ObjectId oid, int writeSize,int offset ) {		
		byte[] expBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, writeSize, offset);
		RandomWriteLobUtil.assertByteArrayEqual(lobBuff, expBuff);
	}	
}


