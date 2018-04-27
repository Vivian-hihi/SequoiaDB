package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * @FileName seqDB-13263: 并发锁全部lob写入 
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、共享模式下，多个连接多线程并发如下操作: 
 *    (1)打开已存在lob对象，锁定全部lob，向锁定的数据段写入数据 
 * 2、读取lob，检查操作结果
 */

public class LockAllAndWrite13263 extends SdbTestBase {

    private final String csName = "writelob13263";
    private final String clName = "writelob13263";
    private final int lobPageSize = 16 * 1024; // 16k
    private final int threadNum = 16;
    private AtomicInteger successTimes = new AtomicInteger(0);
    private byte[] expData = null;

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
    public void testLob() {
        try {
            int lobSize = 8 * 1024 * 1024;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);
            
            expData = data;
            List<LobPart> parts = new ArrayList<>(threadNum);
            final int partSize = lobSize / threadNum;
            for (int i = 0; i < threadNum; ++i) {
                parts.add(new LobPart(partSize * i, partSize * (i + 1)));
            }

            List<WriteLobThread> thrdList = new ArrayList<>(threadNum);
            for (int i = 0; i < threadNum; ++i) {
                WriteLobThread wLobThrd = new WriteLobThread(oid, parts.get(i));
                wLobThrd.start();
                thrdList.add(wLobThrd);
            }
            for (int i = 0; i < threadNum; ++i) {
                thrdList.get(i).join();
            }

            Assert.assertNotEquals( successTimes.get(), 0);
            byte[] actData = RandomWriteLobUtil.readLob(cl, oid);
            RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");
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
            try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                try (DBLob lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE)) {
                    lob.lock(0, lob.getSize());
                    lob.seek(part.getOffset(), DBLob.SDB_LOB_SEEK_SET);
                    lob.write(part.getData());
                    updateExpData(part);
                    successTimes.getAndIncrement();
                }
            }
        }
    }
    
    private synchronized void updateExpData(LobPart part) {
        expData = RandomWriteLobUtil.appendBuff(expData, part.getData(), part.getOffset());
    }
    
}
