package com.sequoiadb.lob.randomwrite;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @FileName seqDB-13264:并发加锁写lob，其中写数据范围连续
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、共享模式下，多个连接多线程并发如下操作: (1)打开已存在lob对象，seek指定偏移范围，执行lock锁定数据段， 向锁定数据段写入lob 多个并发线程中锁定数据段为连续范围，
 * 如线程1锁定数据范围为1-3、线程2锁定数据范围为4-5、线程3锁定数据范围为5-9 2、读取lob，检查操作结果
 */

public class LockAndWriteContinuous13264 extends SdbTestBase {

    private final String csName = "writelob13264";
    private final String clName = "writelob13264";
    private final int lobPageSize = 32 * 1024; // 32k
    private final int threadNum = 16;
    private final int writeSizePerThread = 2 * lobPageSize;

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection cl = null;

    @BeforeClass
    public void setUp() {

        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

            // create cs cl
            BSONObject csOpt = (BSONObject) JSON.parse("{LobPageSize: " + lobPageSize + "}");
            cs = sdb.createCollectionSpace(csName, csOpt);
            BSONObject clOpt = (BSONObject) JSON.parse("{ShardingKey:{a:1},ShardingType:'hash'}");
            cl = cs.createCollection(clName, clOpt);

        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }

    @Test
    // start <threadNum> threads, and every thread put
    // a part of lob, which size is <writeSizePerThread>
    public void testLob() {
        try {
            int lobSize = 512 * 1024;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            List<LobPart> parts = getContinuousParts(threadNum, writeSizePerThread);
            List<WriteLobThread> wLobThrds = new ArrayList<WriteLobThread>();

            // initialize threads and expData
            byte[] expData = data;
            for (int i = 0; i < threadNum; ++i) {
                WriteLobThread wLobThrd = new WriteLobThread(oid, parts.get(i));
                wLobThrds.add(wLobThrd);
                expData = updateExpData(expData, parts.get(i));
            }

            // write concurrently
            for (WriteLobThread wLobThrd : wLobThrds) {
                wLobThrd.start();
            }
            for (WriteLobThread wLobThrd : wLobThrds) {
                Assert.assertTrue(wLobThrd.isSuccess(), wLobThrd.getErrorMsg());
            }

            // check lob data and size
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
            int expSize = threadNum * writeSizePerThread;
            int actSize = getLobSizeByList(cl, oid);
            Assert.assertEquals(actSize, expSize, "lob size is wrong");

        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            if (sdb.isCollectionSpaceExist(csName)) {
                sdb.dropCollectionSpace(csName);
            }
        } catch (BaseException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            if (null != sdb) {
                sdb.close();
            }
        }
    }

    private class WriteLobThread extends SdbThreadBase {
        private ObjectId oid = null;
        private LobPart part = null;

        public WriteLobThread(ObjectId oid, LobPart part) {
            this.oid = oid;
            this.part = part;
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            DBLob lob = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);

                lob.lockAndSeek(part.getOffset(), part.getLength());
                lob.write(part.getData());
            } finally {
                if (null != lob) {
                    lob.close();
                }
                if (null != db) {
                    db.close();
                }
            }
        }
    }

    private List<LobPart> getContinuousParts(int partNum, int partSize) {
        List<LobPart> parts = new ArrayList<LobPart>();
        for (int i = 0; i < partNum; ++i) {
            LobPart part = new LobPart(i * partSize, partSize);
            parts.add(part);
        }
        return parts;
    }

    private byte[] updateExpData(byte[] expData, LobPart part) {
        return RandomWriteLobUtil.appendBuff(expData, part.getData(), part.getOffset());
    }

    private int getLobSizeByList(DBCollection cl, ObjectId oid) {
        DBCursor cursor = cl.listLobs();
        boolean oidFound = false;
        int lobSize = 0;
        while (cursor.hasNext()) {
            BSONObject currRec = cursor.getNext();
            ObjectId currOid = (ObjectId) currRec.get("Oid");
            if (currOid.equals(oid)) {
                lobSize = (int) ((long) currRec.get("Size"));
                oidFound = true;
                break;
            }
        }
        cursor.close();
        if (!oidFound) {
            throw new RuntimeException("oid not found");
        }
        return lobSize;
    }

}
