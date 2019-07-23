package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.lob.randomwrite.RandomWriteLobUtil;

/**
* @Description seqDB-13233 : 未加锁写lob
* @author laojingtang
* @UpdateAuthor wuyan
* @Date    2017.11.2
* @UpdateDate 2019.07.16
* @version 1.10
*/
public class LobTest13233 extends SdbTestBase {
	@DataProvider(name = "test13233DataProvider")
    public static Object[][] test13233DataProvider() {
        return new Object[][]{
        	//oldLobSize, newLobSize, offset
        	//test a: newLobSize < oldLobSize
            {1024, 500, 500},
            //test b: newLobSize = oldLobSize
            {1024, 1023, 1},
            //test c: newLobSize > oldLobSize
            {1024, 1024, 500}
        };
    }
	
    private Sequoiadb db = null;
    private DBCollection dbcl = null;
    private CollectionSpace cs = null;    
    private String clName = "lobcl_13233";    

    @BeforeClass
    public void setUp() {       
        db = new Sequoiadb(coordUrl,"","");
        cs = db.getCollectionSpace(SdbTestBase.csName);
        dbcl = cs.createCollection(clName,
                (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\"}"));       
    }
    
    @Test(dataProvider = "test13233DataProvider")
    public void testLob13233(int lobSize, int newDataSize, int offset) {        
        byte[] randomBytes = RandomWriteLobUtil.getRandomBytes(lobSize);        
        ObjectId oid = RandomWriteLobUtil.createAndWriteLob(dbcl, randomBytes);

        //seek and write lob
        byte[] newData = RandomWriteLobUtil.getRandomBytes(newDataSize);
        try (DBLob lob = dbcl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
        	 lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);             
             lob.write(newData);
		}
        
        //read lob and check the lob content
        try (DBLob lob = dbcl.openLob(oid)) {
        	 byte[] actual = new byte[(int) lob.getSize()];
             lob.read(actual);
             lob.close();
             RandomWriteLobUtil.assertByteArrayEqual(actual, RandomWriteLobUtil.appendBuff(randomBytes, newData, offset));
	    }       
    }
    
    @AfterClass
    public void tearDown() {
    	try{			
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}			
		}finally{
			if( db != null ){
				db.close();
			}
		}       
    }   
}
