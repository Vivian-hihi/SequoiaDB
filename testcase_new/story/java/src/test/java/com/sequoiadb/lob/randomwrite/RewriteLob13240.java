package com.sequoiadb.lob.randomwrite;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
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
 * @Description seqDB-13240 : 锁定连续范围数据段写lob
 * @author laojingtang
 * @UpdateAuthor wuyan
 * @Date 2017.12.1
 * @UpdateDate 2019.07.16
 * @version 1.10
 */
public class RewriteLob13240 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private DBCollection dbcl = null;
    private CollectionSpace cs = null;
    private String clName = "lobcl_13240";
    private ObjectId id = null;
    private int writeSize = 1024 * 1024 * 10;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(coordUrl, "", "");
        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        dbcl = cs.createCollection(clName, (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\"}"));

        byte[] lobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
        id = RandomWriteLobUtil.createAndWriteLob(dbcl, lobBuff);
    }

    @Test
    public void testLob13240() {
        byte[] writeLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
        try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
            int offset = 0;
            int length = 1024 * 257;
            lob.lock(offset, length);
            lob.lock(length, writeSize);
            lob.write(writeLobBuff);
        }

        // check lock and write lob result.
        byte[] readLobBuff = RandomWriteLobUtil.readLob(dbcl, id);
        RandomWriteLobUtil.assertByteArrayEqual(readLobBuff, writeLobBuff);
    }

    @AfterClass
    public void tearDown() {
        try {
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }
        } finally {
            if (sdb != null) {
                sdb.close();
            }
        }
    }
}
