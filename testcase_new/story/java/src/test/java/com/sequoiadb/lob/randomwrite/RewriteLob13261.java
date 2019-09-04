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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description: testlink case:seqDB-13261:write lobdata to the locked data
 *               segment and write lobdata to the non locked data segment,
 *               testlink case:seqDB-13261
 * 
 * @author wuyan
 * @Date 2017.11.8
 * @version 1.00
 */
public class RewriteLob13261 extends SdbTestBase {
    private String clName = "writelob13261";
    private static Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private static DBCollection cl = null;
    private static ObjectId oid = null;
    private byte[] testLobBuff = null;

    @BeforeClass
    public void setUp() {
        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        } catch (BaseException e) {
            Assert.assertTrue(false, "connect %s failed," + SdbTestBase.coordUrl + e.getMessage());
        }

        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024," + "ReplSize:0,Compressed:true}";
        cl = RandomWriteLobUtil.createCL(cs, clName, clOptions);

        // put lob
        int writeSize = 1024 * 1024 * 2;
        testLobBuff = RandomWriteLobUtil.getRandomBytes(writeSize);
        oid = RandomWriteLobUtil.createAndWriteLob(cl, testLobBuff);
    }

    @Test
    public void testLob() {
        int offset1 = 1024 * 1024 * 1;
        int rewriteLobSize1 = 1024 * 1024 * 4;
        int offset2 = 1024 * 1024 * 2;
        int rewriteLobSize2 = 1024 * 1024 * 4;
        byte[] seekAndRewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize1);
        byte[] lockAndRewriteBuff = RandomWriteLobUtil.getRandomBytes(rewriteLobSize2);
        SeekAndRewriteLobTask seekAndRewriteLob = new SeekAndRewriteLobTask(offset1, seekAndRewriteBuff);
        LockAndRewriteLobTask lockAndRewriteLob = new LockAndRewriteLobTask(offset2, lockAndRewriteBuff);

        lockAndRewriteLob.start();
        seekAndRewriteLob.start();

        if (lockAndRewriteLob.isSuccess()) {
            if (!seekAndRewriteLob.isSuccess()) {
                Assert.assertTrue(!seekAndRewriteLob.isSuccess(), seekAndRewriteLob.getErrorMsg());
                BaseException e = (BaseException) (seekAndRewriteLob.getExceptions().get(0));
                Assert.assertEquals(-320, e.getErrorCode(),
                        "seekAndRewriteLob must fail:" + seekAndRewriteLob.getErrorMsg());
                readLobAndcheckWriteResult(cl, oid, testLobBuff, lockAndRewriteBuff, offset2);
            } else {
                // can't determine the status of the server, and maybe all
                // operations are sucessful,
                // only check the lob size
                Assert.assertTrue(seekAndRewriteLob.isSuccess());
                try (DBLob lob = cl.openLob(oid)) {
                    long actWriteLobSize = lob.getSize();
                    long expLobSize = 1024 * 1024 * 6;
                    Assert.assertEquals(actWriteLobSize, expLobSize, "the lobsize is different!");
                }
            }
        } else if (!lockAndRewriteLob.isSuccess()) {
            Assert.assertTrue(seekAndRewriteLob.isSuccess(), seekAndRewriteLob.getErrorMsg());
            BaseException e = (BaseException) (lockAndRewriteLob.getExceptions().get(0));
            Assert.assertEquals(-320, e.getErrorCode(),
                    "LockAndRewriteLob must fail:" + lockAndRewriteLob.getErrorMsg());
            readLobAndcheckWriteResult(cl, oid, testLobBuff, seekAndRewriteBuff, offset1);
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }
            sdb.close();
        } catch (BaseException e) {
            Assert.assertTrue(false, "clean up failed:" + e.getMessage());
        }
    }

    private class SeekAndRewriteLobTask extends SdbThreadBase {
        private int offset;
        private byte[] rewriteLobBuff;

        public SeekAndRewriteLobTask(int offset, byte[] rewriteLobBuff) {
            this.offset = offset;
            this.rewriteLobBuff = rewriteLobBuff;
        }

        @Override
        public void exec() throws Exception {
            DBLob lob = null;
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = sdb.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
                lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);
                lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
                lob.write(rewriteLobBuff);
                lob.close();
            }
        }
    }

    private class LockAndRewriteLobTask extends SdbThreadBase {
        private int offset;
        private byte[] rewriteLobBuff;

        public LockAndRewriteLobTask(int offset, byte[] rewriteLobBuff) {
            this.offset = offset;
            this.rewriteLobBuff = rewriteLobBuff;
        }

        @Override
        public void exec() throws Exception {
            DBLob lob = null;
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = sdb.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
                lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);
                lob.lockAndSeek(offset, rewriteLobBuff.length);
                lob.write(rewriteLobBuff);
                lob.close();
            }
        }
    }

    private void readLobAndcheckWriteResult(DBCollection cl, ObjectId oid, byte[] lobBuff, byte[] rewriteBuff,
            int offset) {
        // check the rewrite lob
        byte[] actBuff = RandomWriteLobUtil.seekAndReadLob(cl, oid, rewriteBuff.length, offset);
        RandomWriteLobUtil.assertByteArrayEqual(actBuff, rewriteBuff);

        // check the all write lob
        byte[] expBuff = RandomWriteLobUtil.appendBuff(lobBuff, rewriteBuff, offset);
        try (DBLob lob = cl.openLob(oid)) {
            byte[] actAllLob = new byte[(int) lob.getSize()];
            lob.read(actAllLob);
            if (!Arrays.equals(actAllLob, expBuff)) {
                RandomWriteLobUtil.writeLobAndExpectData2File(lob, expBuff);
                Assert.fail("check actlob and expbuff different: oid=" + oid.toString());
            }
        }
    }
}
