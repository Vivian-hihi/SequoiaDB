package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.logging.Logger;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.assertByteArrayEqual;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.getRandomBytes;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.readLob;

/**
 * Created by laojingtang on 17-11-22.
 */
public class LobTest13227 extends SdbTestBase {
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
        if (groupNames.size() < 2)
            throw new SkipException("");

        dbcl = cs.createCollection(clName,
                (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\",Group:'" + groupNames.get(0) + "'}"));
        dbcl.split(groupNames.get(0), groupNames.get(1), 50);
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }

    /**
     * 测试用例 seqDB-13227 :: 版本: 1 :: 写lob同时并发读lob
     * 1、打开lob（创建模式是指定lob oid）
     * 2、写入lob数据，写入过程中读取该lob数据（读取lob时获取指定lob oid）
     * 3、检查操作结果
     * 1、写入lob成功，读取lob操作失败，返回对应错误信息
     * 2、再次读取lob数据和插入lob数据一致（比较MD5值相同）
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void test13227(final int lobsize) throws InterruptedException {
        final ObjectId oid = ObjectId.get();
        final byte[] expectBytes = getRandomBytes(lobsize);

        final AtomicBoolean canRead = new AtomicBoolean(false);

        DbClOperateTask writeThread = new DbClOperateTask(csName, clName) {
            @Override
            protected void exec() throws Exception {
                DBLob lob = null;
                lob = this.dbcl.createLob(oid);
                canRead.set(true);
                lob.write(expectBytes);
                lob.close();
            }
        };

        DbClOperateTask readThread = new DbClOperateTask(csName, clName) {
            @Override
            protected void exec() throws Exception {
                while (true) {
                    if (canRead.get()) {
                        byte[] actualBytes = new byte[lobsize];
                        DBLob lob = null;
                        lob = this.dbcl.openLob(oid);
                        lob.read(actualBytes);
                        lob.close();
                        break;
                    } else {
                        Thread.sleep(500);
                    }
                }
            }
        }.ignoreExceptionCode(SDBError.SDB_FNE.getErrorCode())
                .ignoreExceptionCode(SDBError.SDB_LOB_IS_IN_USE.getErrorCode());

        writeThread.start();
        readThread.start();
        writeThread.join();
        readThread.join();

        Assert.assertTrue(writeThread.isTaskSuccess(), writeThread.getErrorMsg());
        Assert.assertTrue(readThread.isTaskSuccess(), readThread.getErrorMsg());

        byte[] actualBytes = readLob(dbcl, oid);
        assertByteArrayEqual(actualBytes, expectBytes);
    }
}
