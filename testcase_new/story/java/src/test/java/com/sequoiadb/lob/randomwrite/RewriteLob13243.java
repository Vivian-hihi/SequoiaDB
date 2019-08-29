package com.sequoiadb.lob.randomwrite;

import java.util.Arrays;

import org.bson.types.ObjectId;
import org.testng.Assert;
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
 * @Description seqDB-13243:lock the data segment to write lob,and locking data segment range
 *              intersection
 * @author wuyan
 * @Date 2017.11.7
 * @version 1.00
 */
public class RewriteLob13243 extends SdbTestBase {
    private String clName = "writelob13243";
    private static Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private static DBCollection cl = null;
    private byte[] testLobBuff = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024}";
        cl = RandomWriteLobUtil.createCL(cs, clName, clOptions);
    }

    @Test
    public void testLob() {
        int writeSize = 1024 * 2;
        testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
        ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, testLobBuff);
        rewriteLob(oid);
        checkResult(oid);
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

    private void checkResult(ObjectId oid) {
        // check the all write lob
        try (DBLob lob = cl.openLob(oid)) {
            byte[] actAllLob = new byte[(int) lob.getSize()];
            lob.read(actAllLob);
            if (!Arrays.equals(actAllLob, testLobBuff)) {
                RandomWriteLobUtil.writeLobAndExpectData2File(lob, testLobBuff);
                Assert.fail("check actlob and expbuff different");
            }
        }
    }

    private void rewriteLob(ObjectId oid) {
        int offset1 = 1024;
        int lobSize1 = 260096;
        int offset2 = 260096;
        int lobSize2 = 262144;
        int offset3 = 360096;
        int lobSize3 = 262144;
        int rewriteSize = offset3 + lobSize3 - offset1;
        byte[] rewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteSize);
        testLobBuff = RandomWriteLobUtil.appendBuff(testLobBuff, rewriteBuff, offset1);
        try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);) {
            lob.lock(offset1, lobSize1);
            lob.lock(offset2, lobSize2);
            lob.lock(offset3, lobSize3);
            lob.seek(offset1, DBLob.SDB_LOB_SEEK_SET);
            lob.write(rewriteBuff);
            long actWriteLobSize = lob.getSize();
            long expLobSize = offset1 + rewriteSize;
            Assert.assertEquals(actWriteLobSize, expLobSize, "the lobsize is different!");
        }
    }
}
