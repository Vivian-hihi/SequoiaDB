package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.*;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.BSONTimestamp;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.*;
import java.util.logging.Logger;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.assertByteArrayEqual;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.createEmptyLob;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.getRandomBytes;
import static java.lang.Math.abs;

/**
 * Created by laojingtang on 17-11-6.
 */
public class LobTest13237 extends SdbTestBase {
    Logger log = Logger.getLogger(LobTest13237.class.getName());
    Sequoiadb db = null;
    DBCollection dbcl = null;
    CollectionSpace cs = null;
    String csName;
    String clName;

    @BeforeClass
    public void setupClass() {
        csName = SdbTestBase.csName;
        clName = "cl_" + this.getClass().getSimpleName();
        db = new Sequoiadb(coordUrl, "", "");
        cs = db.getCollectionSpace(csName);
        List<String> groupNames = RandomWriteLobUtil.getDataGroups(db);

        dbcl = cs.createCollection(clName,
                (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\",Group:'" + groupNames.get(0) + "'}"));
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }

    /**
     * 1、多个连接多线程并发如下操作:
     * (1)打开已存在lob对象，seek指定偏移范围，执行read读取指定范围内数据
     * 多个并发线程中指定数据范围有交集，如线程1读数据范围为1-5、线程2读数据范围为2-15、线程3锁定数据范围为3-7
     * 2、检查操作结果
     * 1、所有线程读lob成功，读取指定范围内数据，查询读取对应范围内lob信息和实际写入数据一致（比较MD5值）
     *
     * @param lobsize
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13273(int lobsize) throws InterruptedException {
        final byte[] expectBytes = getRandomBytes(lobsize);
        DBLob lob = dbcl.createLob();
        final ObjectId oid = lob.getID();
        lob.write(expectBytes);
        lob.close();

        List<DbLobReadTask> lobTasks = new ArrayList<>(10);

        final int step = lobsize / 10;
        for (int i = 0; i < 10; i++) {
            final int begin = i * step;
            int length = begin + 50 > lobsize ? lobsize - begin : 50;
            lobTasks.add(new DbLobReadTask(csName, clName, begin, length, oid));
        }

        for (DbLobReadTask lobTask : lobTasks) lobTask.start();
        for (DbLobReadTask lobTask : lobTasks) lobTask.join();

        for (DbLobReadTask lobTask : lobTasks) {
            Assert.assertTrue(lobTask.isTaskSuccess(), lobTask.getErrorMsg());
            int b = lobTask.getBegin();
            int length = lobTask.getLength();
            byte[] expect = Arrays.copyOfRange(expectBytes, b, b + length);
            byte[] actual = lobTask.getResult();
            assertByteArrayEqual(actual, expect);
        }
    }



    private byte[] readLob(ObjectId id) {
        return RandomWriteLobUtil.readLob(dbcl, id, 5);
    }

    /**
     * 1、写入lob大小为空
     * 2、共享模式下打开lob
     * 3、写入lob数据
     * 3、检查lob写入结果
     * 1、写入lob成功，查询lob数据信息和实际写入数据一致（比较MD5值）
     *
     * @param lobsize
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13237(int lobsize) {
        ObjectId id = createEmptyLob(dbcl);

        byte[] randomBytes = getRandomBytes(lobsize);

        try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
            lob.write(randomBytes);
        }

        assertByteArrayEqual(readLob(id), randomBytes);
    }






    /**
     * 1、打开已存在lob
     * 2、seek、lock指定偏移范围锁定数据段
     * 3、写入lob
     * 4、读取lob
     * 5、检查操作结果
     * 6、相同连接中重复步骤3-5
     * 1、写入、读取lob成功，第一次读取lob和插入数据一致；第一次读取lob数据为更新后lob数据（MD5码比对结果）
     *
     * @param lobsize
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13255(int lobsize) {
        ObjectId id = createEmptyLob(dbcl);

        for (int i = 0; i < 5; i++) {

            byte[] data = getRandomBytes(lobsize);
            try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
                lob.seek(100, DBLob.SDB_LOB_SEEK_SET);
                lob.write(data);
            }

            try (DBLob lob = dbcl.openLob(id)) {
                byte[] actual = new byte[lobsize];
                lob.seek(100, DBLob.SDB_LOB_SEEK_SET);
                lob.read(actual);
                assertByteArrayEqual(actual, data);
            }
        }
    }

    /**
     * 1、打开已存在lob对象
     * 2、锁定数据段（lockAndSeek）
     * 3、写入lob
     * 4、检查操作结果
     * 1、写入lob成功，查询lob数据包含新写入和未覆盖数据，且lob信息和实际插入信息一致（比较MD5值）；
     * 2、list查询lob
     * size信息正确
     *
     * @param lobsize
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13258(int lobsize) {
        ObjectId id = createEmptyLob(dbcl);
        byte[] data = getRandomBytes(lobsize);

        try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
            lob.lockAndSeek(100, lobsize);
            lob.write(data);
        }

        try (DBLob lob = dbcl.openLob(id)) {
            lob.seek(100, DBLob.SDB_LOB_SEEK_SET);
            byte[] actual = new byte[lobsize];
            lob.read(actual);
            assertByteArrayEqual(actual, data);
        }
    }


    /**
     * 1、重新打开lob
     * 2、指定范围锁定lob，写入lob
     * 3、list查看创建和修改lob时间信息（createTime、modificationTime）
     * 4、重复步骤2-3
     * 1、查看lob创建时间正确，且多次写入lob后时间不变化
     * 2、每次写入lob操作后，查看修改lob时间更新为最后一次写lob时间
     *
     * @param lobsize
     */
    @Test(enabled = false)
    public void testLob13256(int lobsize) throws InterruptedException {
        ObjectId id = createEmptyLob(dbcl);
        long time1 = System.currentTimeMillis() / 1000;
        log.info("create...");

        Thread.sleep(5 * 1000);

        long time2;
        try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
            lob.write(getRandomBytes(lobsize));
            time2 = System.currentTimeMillis() / 1000;
        }

        DBCursor cursor = dbcl.listLobs();
        BSONObject lobInfo = null;
        while (cursor.hasNext()) {
            BSONObject bsonObject = cursor.getNext();
            if (bsonObject.get("Oid").equals(id))
                lobInfo = bsonObject;
        }
        Assert.assertNotNull(lobInfo, "can find lob oid: " + id);
        BSONTimestamp creatTime = (BSONTimestamp) lobInfo.get("CreateTime");
        BSONTimestamp modifTime = (BSONTimestamp) lobInfo.get("ModificationTime");

        long createTimeLong = creatTime.toTimestamp().getTime() / 1000;
        long modifTimeLong = modifTime.toTimestamp().getTime() / 1000;

        Assert.assertTrue(abs(createTimeLong - time1) < 2, "\nexpect create time: " + time1 + "\nbut actual time:" + createTimeLong);
        Assert.assertTrue(abs(modifTimeLong - time2) < 2, "\nexpect create time: " + time2 + "\nbut actual time:" + modifTime);
    }

    /**
     * 1、共享模式下，多个连接多线程并发如下操作:
     * (1)打开已存在lob对象，seek指定偏移范围，执行lock锁定数据段，向锁定数据段写入lob
     * （2）打开已存在lob对象，指定偏移范围执行lockAndSeek锁定数据段，向锁定数据段写入lob
     * 并发操作间锁定数据段范围不冲突
     * 2、读取lob，检查操作结果
     * 1、写入lob成功，查询lob信息按指定位置写入数据，且写入数据信息正确（比较MD5值）
     */
    @Test
    public void testLob13262() throws InterruptedException {
        final ObjectId id = createEmptyLob(dbcl);

        List<byte[]> datas = new ArrayList<>(10);
        List<DbLobWriteTask> tasks = new ArrayList<>(10);
        int step = 1024;
        for (int i = 0; i < 10; i++) {
            byte[] data = getRandomBytes(step);
            datas.add(data);
            tasks.add(new DbLobWriteTask(csName, clName, id, i * step, data) {
                @Override
                protected void exec() throws Exception {
                    DBLob lob = this.dbcl.openLob(id, DBLob.SDB_LOB_WRITE);
                    lob.lockAndSeek(this.begin, this.data.length);
                    lob.write(this.data);
                    lob.close();
                }
            });
        }

        for (DbLobWriteTask task : tasks) task.start();
        for (DbLobWriteTask task : tasks) task.join();
        for (DbLobWriteTask task : tasks) Assert.assertTrue(task.isTaskSuccess(), task.getErrorMsg());


        byte[] actualData = readLob(id);
        for (int i = 0; i < 10; i++) {
            assertByteArrayEqual(Arrays.copyOfRange(actualData, i * step, i * step + step), datas.get(i));
        }
    }


}
