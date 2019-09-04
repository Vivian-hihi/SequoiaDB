package com.sequoiadb.lob.randomwrite;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Description seqDB-13245 : 写lob超过锁定范围
 * @author laojingtang
 * @UpdateAuthor wuyan
 * @Date 2017.12.1
 * @UpdateDate 2019.07.17
 * @version 1.10
 */
public class RewriteLob13245 extends SdbTestBase {
    private Sequoiadb db = null;
    private DBCollection dbcl = null;
    private CollectionSpace cs = null;
    private String clName = "lobcl_13245";

    @BeforeClass
    public void setUp() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cs = db.getCollectionSpace(SdbTestBase.csName);
        dbcl = cs.createCollection(clName, (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\"}"));
    }

    @Test
    public void testLob13245() {
        ObjectId id = RandomWriteLobUtil.createEmptyLob(dbcl);

        int lobSize = 1024 * 50;
        byte[] lobBytes = RandomWriteLobUtil.getRandomBytes(lobSize);
        long offset = 0;
        long lockLength = lobSize - 1;
        try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
            lob.lock(offset, lockLength);
            try {
                lob.write(lobBytes);
                Assert.fail("write lob size exceed the lock length should fail!");
            } catch (BaseException e) {
                if (e.getErrorCode() != SDBError.SDB_INVALIDARG.getErrorCode())
                    throw e;
            }
        }

        // check lob content,the actual lob size is 0
        try (DBLob lob = dbcl.openLob(id)) {
            byte[] actual = RandomWriteLobUtil.readLob(lob);
            Assert.assertEquals(actual.length, 0);
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }
        } finally {
            if (db != null) {
                db.close();
            }
        }
    }
}
