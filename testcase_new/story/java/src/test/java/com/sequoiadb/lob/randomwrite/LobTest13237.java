package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.*;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.*;
import com.sequoiadb.lob.randomwrite.RandomWriteLobUtil;


/**
* @Description seqDB-13237 : 写空lob,再次打开lob写数据
* @author laojingtang
* @UpdateAuthor wuyan
* @Date    2017.11.6
* @UpdateDate 2019.07.16
* @version 1.10
*/
public class LobTest13237 extends SdbTestBase {    
    private Sequoiadb db = null;
    private DBCollection dbcl = null;
    private CollectionSpace cs = null;    
    private String clName = "lobcl_13237";
    private Random random = new Random();

    @BeforeClass
    public void setUp() {       
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cs = db.getCollectionSpace(SdbTestBase.csName);
        dbcl = cs.createCollection(clName,
                (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\"}"));
    }
    
    @Test
    public void testLob13237() {
        ObjectId id = RandomWriteLobUtil.createEmptyLob(dbcl);
        
        //write lob again
        int lobSize = random.nextInt( 1024 * 1024 * 5 );
        byte[] randomBytes = RandomWriteLobUtil.getRandomBytes(lobSize);
        try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
            lob.write(randomBytes);
        }
        
        //check write lob result
        byte[] readLobBuff = RandomWriteLobUtil.readLob(dbcl, id, lobSize);
        RandomWriteLobUtil.assertByteArrayEqual(readLobBuff, randomBytes);
    }
    
    @AfterClass
	public void tearDown(){		
		try {			
			if (cs.isCollectionExist(clName)) {
				cs.dropCollection(clName);;
			}			
		} finally{
			if(db != null){
				db.close();
			}
		}
	}	
}
