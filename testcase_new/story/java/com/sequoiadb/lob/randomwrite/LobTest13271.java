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

/**
 * Created by laojingtang on 17-11-20.
 */
public class LobTest13271 extends SdbTestBase {
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


    private byte[] getRandomBytes(int size) {
        if (randomBytesMap.containsKey(size))
            return randomBytesMap.get(size);
        else {
            byte[] b = RandomWriteLobUtil.getRandomBytes(size);
            randomBytesMap.put(size, b);
            return b;
        }
    }

    /**
     * 1、多个连接多线程并发如下操作:
     * (1)打开已存在lob对象，seek指定偏移范围，执行read读取指定范围内数据
     * 多个并发线程中指定数据范围连续，如线程1读数据范围为1-3、线程2读数据范围为4-5、线程3锁定数据范围为5-9
     * 2、检查操作结果
     * 1、所有线程读lob成功，查询对应范围内lob信息和实际写入数据一致（比较MD5值）
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13271(int lobsize) throws InterruptedException {
        byte[] expectBytes = getRandomBytes(lobsize);
        DBLob lob = dbcl.createLob();
        final ObjectId oid = lob.getID();
        lob.write(expectBytes);
        lob.close();

        List<DbLobReadTask> lobTasks = new ArrayList<>(10);

        final int step = lobsize / 10;
        for (int i = 0; i < 10; i++) {
            final int begin = i * step;
            lobTasks.add(new DbLobReadTask(csName, clName, begin, step, oid));
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
}
