package com.sequoiadb.lob.randomwrite;

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
 * @Description seqDB-13248:lock the data segment to write lob,and locking data segment range
 *              discontinuity
 * @author wuyan
 * @Date 2017.11.7
 * @version 1.00
 */
public class RewriteLob13248 extends SdbTestBase {
    private String clName = "writelob13248";
    private static Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private static DBCollection cl = null;
    private byte[] testLobBuff = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        String clOptions = "{ShardingKey:{no:1},ShardingType:'hash'}";
        cl = RandomWriteLobUtil.createCL(cs, clName, clOptions);
    }

    @Test
    public void testLob() {
        int writeSize = 1024 * 1024 * 1;
        testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
        ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, testLobBuff);

        int offset = 1024;
        int reWriteLobSize = 1024 * 256;
        byte[] lastWriteBuff = rewriteLob(oid, offset, reWriteLobSize);

        RandomWriteLobUtil.checkRewriteLobResult(cl, oid, offset, lastWriteBuff, testLobBuff);
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

    private byte[] rewriteLob(ObjectId oid, int offset, int writeLobSize) {
        int lockCount = 10;
        byte[] rewriteBuff = new byte[writeLobSize];
        try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
            for (int i = 0; i < lockCount; i++) {
                rewriteBuff = RandomWriteLobUtil.getRandomBytes(writeLobSize);
                lob.lockAndSeek(offset, writeLobSize);
                lob.write(rewriteBuff);
            }
        }
        // get the last written lobBuff
        return rewriteBuff;
    }
}
