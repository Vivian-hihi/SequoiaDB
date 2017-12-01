package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.*;
import java.util.logging.Logger;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.assertByteArrayEqual;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.getRandomBytes;

/**
 * Created by laojingtang on 17-11-22.
 */
public class LobTest13274 extends SdbTestBase{
    Logger log = Logger.getLogger(LobTest13237.class.getName());
    Sequoiadb db = null;
    DBCollection dbcl = null;
    CollectionSpace cs = null;
    String csName;
    String clName;
    private static Map<Integer, byte[]> randomBytesMap = new HashMap<>();

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
     * (1)打开已存在lob对象，seek指定偏移范围，读取
     * 相同范围内数据
     * 2、检查操作结果
     * 1、所有操作读lob成功，查看多个线程读取lob信息一致
     *
     * @param lobsize
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13274(int lobsize) throws InterruptedException {
        final byte[] expectBytes = getRandomBytes(lobsize);
        DBLob lob = dbcl.createLob();
        final ObjectId oid = lob.getID();
        lob.write(expectBytes);
        lob.close();

        List<DbLobReadTask> lobTasks = new ArrayList<>(10);

        for (int i = 0; i < 10; i++) lobTasks.add(new DbLobReadTask(csName, clName, 100, 1025, oid));

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
}
