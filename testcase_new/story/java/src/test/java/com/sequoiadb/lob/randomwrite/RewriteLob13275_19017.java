package com.sequoiadb.lob.randomwrite;

import java.util.Arrays;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.lob.utils.LobSubUtils;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @FileName seqDB-13275: 并发读写lob seqDB-19017 主子表并发读写lob
 * @Author linsuqiang
 * @Date 2017-11-08
 * @Version 1.00
 */

/*
 * 1、多个连接多线程并发如下操作: (1)打开已存在lob对象，锁定（lockAndSeek）指定范围数据段，写入lob （2）打开已存在lob对象，seek指定偏移范围读 2、检查操作结果
 */

public class RewriteLob13275_19017 extends SdbTestBase {

    private final String csName = "writelob13275";
    private final String clName = "writelob13275";
    private final String mainCLName = "mainCL19017";
    private final String subCLName = "subCL19017";
    private final int lobPageSize = 512 * 1024; // 512k

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;

    @DataProvider(name = "clNameProvider", parallel = false)
    public Object[][] generateCLName() {
        return new Object[][] {
                // the parameter is csName, clName
                // testcase:13275
                new Object[] { clName },
                // testcase:19017
                new Object[] { mainCLName } };
    }

    @BeforeClass
    public void setUp() {

        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            // TODO:1、非主子表不需要屏蔽独立模式
            if (CommLib.isStandAlone(sdb)) {
                throw new SkipException("is standalone skip testcase");
            }
            // create cs cl
            BSONObject csOpt = (BSONObject) JSON.parse("{LobPageSize: " + lobPageSize + "}");
            cs = sdb.createCollectionSpace(csName, csOpt);
            BSONObject clOpt = (BSONObject) JSON.parse("{ShardingKey:{a:1},ShardingType:'hash'}");
            cs.createCollection(clName, clOpt);

            LobSubUtils.createMainCLAndAttachCL(sdb, csName, mainCLName, subCLName);

        } catch (BaseException e) {
            e.printStackTrace();// TODO:2、这里的try-catch和打印信息可以去掉
            Assert.fail(e.getMessage());
        }
    }

    @Test(dataProvider = "clNameProvider")
    public void testLob(String clName) {
        try {
            int lobSize = 1 * 1024 * 1024;
            byte[] data = RandomWriteLobUtil.getRandomBytes(lobSize);
            DBCollection cl = sdb.getCollectionSpace(csName).getCollection(clName);
            ObjectId oid = RandomWriteLobUtil.createAndWriteLob(cl, data);

            WriteLobThread wThrd = new WriteLobThread(clName, oid, data);
            ReadLobThread rThrd = new ReadLobThread(csName, clName, oid, data);
            wThrd.start();
            rThrd.start();
            boolean writeOk = wThrd.isSuccess();
            boolean readOk = rThrd.isSuccess();

            if (writeOk && !readOk) {
                int readErrCode = ((BaseException) rThrd.getExceptions().get(0)).getErrorCode();
                Assert.assertEquals(readErrCode, -317, rThrd.getErrorMsg());

            } else if (!writeOk && readOk) {
                int writeErrCode = ((BaseException) wThrd.getExceptions().get(0)).getErrorCode();
                Assert.assertEquals(writeErrCode, -317, wThrd.getErrorMsg());

            } else if (!writeOk && !readOk) {
                Assert.fail("both write and read lob fail");

            } else if (writeOk && readOk) {
                System.out.println("writing and reading lob are not concurrently");
            }

        } catch (BaseException e) {// TODO:3、这里的try-catch和打印信息可以去掉
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
            e.printStackTrace();// TODO:2、这里打印信息可以去掉
            Assert.fail(e.getMessage());
        } finally {
            if (null != sdb) {
                sdb.close();
            }
        }
    }

    private class WriteLobThread extends SdbThreadBase {

        private String clNamet;
        private ObjectId oid = null;
        private byte[] expData = null;

        public WriteLobThread(String clName, ObjectId oid, byte[] expData) {
            this.clNamet = clName;
            this.oid = oid;
            this.expData = expData;
        }

        @Override
        public void exec() throws Exception {
            Random random = new Random();
            Thread.sleep(random.nextInt(100));

            Sequoiadb db = null;
            DBLob lob = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(this.clNamet);
                lob = cl.openLob(oid, DBLob.SDB_LOB_WRITE);

                // write lob
                long offset = 0;
                long length = lob.getSize() / 2;
                lob.lockAndSeek(offset, length);
                byte[] data = new byte[(int) length];
                random.nextBytes(data);
                lob.write(data);
                expData = RandomWriteLobUtil.appendBuff(expData, data, (int) offset);

                // check result
                lob.close();
                lob = cl.openLob(oid, DBLob.SDB_LOB_READ);
                byte[] actData = new byte[(int) lob.getSize()];
                lob.read(actData);
                RandomWriteLobUtil.assertByteArrayEqual(actData, expData, "lob data is wrong");

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

    private class ReadLobThread extends SdbThreadBase {

        private String csNamet;
        private String clNamet;
        private ObjectId oid = null;
        private byte[] expData = null;

        public ReadLobThread(String csName, String clName, ObjectId oid, byte[] expData) {
            this.csNamet = csName;
            this.clNamet = clName;
            this.oid = oid;
            this.expData = expData;
        }

        @Override
        public void exec() throws Exception {
            Thread.sleep(new Random().nextInt(100));

            Sequoiadb db = null;
            DBLob lob = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(this.csNamet).getCollection(this.clNamet);
                lob = cl.openLob(oid, DBLob.SDB_LOB_READ);

                // read content
                long offset = lob.getSize() / 2;
                long length = lob.getSize() / 2;
                byte[] readData = new byte[(int) length];
                lob.lockAndSeek(offset, length);
                lob.read(readData);

                // check result
                expData = Arrays.copyOfRange(expData, (int) offset, (int) (offset + length));
                RandomWriteLobUtil.assertByteArrayEqual(readData, expData, "lob data is wrong");

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

}