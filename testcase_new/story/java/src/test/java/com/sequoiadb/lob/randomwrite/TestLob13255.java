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
* @Description seqDB-13255 : 重复多次读写lob
* @author laojingtang
* @UpdateAuthor wuyan
* @Date    2017.12.1
* @UpdateDate 2019.07.17
* @version 1.10
*/
public class TestLob13255 extends SdbTestBase {   
    private Sequoiadb db = null;
    private DBCollection dbcl = null;
    private CollectionSpace cs = null;    
    private String clName = "lobcl_13255";
    private Random random = new Random();
    private byte[] lobBuff= null;

    @BeforeClass
    public void setUp() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cs = db.getCollectionSpace(SdbTestBase.csName);
        dbcl = cs.createCollection(clName);        
    }    

    @Test
    public void testLob13255(){
    	int writeSize = random.nextInt( 1024 * 1024 * 10 );
        lobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);		
        ObjectId oid = RandomWriteLobUtil.createAndWriteLob(dbcl, lobBuff);	        

        int offset = 1024;
        int reWriteSize = 1024 * 256;
        int writeCount = 10;
        byte[] reWriteBuff = new byte[ reWriteSize ];
        for( int i = 0; i < writeCount; i++ ){        	
        	reWriteBuff = RandomWriteLobUtil.getRandomBytes(reWriteSize);
            try (DBLob lob = dbcl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
                lob.lockAndSeek(offset, reWriteSize);
                lob.write(reWriteBuff);           
            }
            
            //check the rewrite lob 
    		byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(dbcl, oid, reWriteBuff.length, offset);		
    		RandomWriteLobUtil.assertByteArrayEqual(actBuff, reWriteBuff,"write count:" + i + " is content error!");
              
        }
        
        //check the all write lob 
        byte[] expBuff = RandomWriteLobUtil.appendBuff(lobBuff, reWriteBuff, offset);
      	byte[] actAllLobBuff = RandomWriteLobUtil.seekAndReadLob(dbcl, oid, expBuff.length, 0);
      	RandomWriteLobUtil.assertByteArrayEqual(actAllLobBuff, expBuff);     
    }
    
    @AfterClass
    public void tearDown(){		
		try{					
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}			
		}finally{
			if(db != null){
				db.close();
			}
		}
	}	

}
