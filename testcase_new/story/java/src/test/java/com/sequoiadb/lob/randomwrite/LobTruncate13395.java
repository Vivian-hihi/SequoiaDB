package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.*;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.logging.Logger;

/**
 * Created by laojingtang on 17-11-20.
 */
public class LobTruncate13395 extends SdbTestBase {
    Logger log = Logger.getLogger(LobTruncate13395.class.getSimpleName());
    private String csName, clName;
    Sequoiadb db;
    CollectionSpace cs;
    DBCollection dbcl;
    final int lobSize = 1024;
    final byte[] _randomDatas = RandomWriteLobUtil.getRandomBytes(lobSize);

    @BeforeClass
    public void setupClass() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        csName = SdbTestBase.csName;
        clName = "cl_" + this.getClass().getSimpleName();
        cs = db.getCollectionSpace(csName);
        dbcl = cs.createCollection(
                clName,
                (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\"}")
        );
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }

    /**
     * 1、执行truncate删除指定大小lob数据（如，删除500M大小）
     * 2、过程中读取lob，读取lob范围覆盖测试加锁范围数据、未加锁范围数据
     * 3、检查写入和读取lob结果
     * 1、truncatelob成功，读取lob失败，返回对应错误信息
     * 2、查看指定lob数据已不存在，执行listLobs查看lob大小和truncate指定大小一致
     */
    @Test
    public void testLob13395() throws InterruptedException {
        final ObjectId id;
        try (DBLob lob = dbcl.createLob()) {
            lob.write(_randomDatas);
            id = lob.getID();
        }

        final AtomicBoolean canRead = new AtomicBoolean(false);

        DbClOperateTask readDbClTask = new DbClOperateTask(csName, clName) {
            @Override
            protected void exec() throws Exception {
                while (!canRead.get())
                    Thread.sleep(100);
                try (DBLob lob = this.dbcl.openLob(id)) {
                    RandomWriteLobUtil.readLob(lob, 5);
                }
            }
        };


        DbClOperateTask truncateLob = new DbClOperateTask(csName, clName) {
            @Override
            protected void exec() throws Exception {
                this.dbcl.truncateLob(id, 100);
                canRead.set(true);
            }
        };

        truncateLob.start();
        readDbClTask.start();
        truncateLob.join();
        readDbClTask.join();

        String lobErrMsg = "lob id : " + id.toString();
        Assert.assertTrue(truncateLob.isTaskSuccess(), lobErrMsg);
        try (DBLob lob = dbcl.openLob(id)) {
            Assert.assertEquals(lob.getSize(), 100, lobErrMsg);
            RandomWriteLobUtil.assertByteArrayEqual(RandomWriteLobUtil.readLob(lob, 5), Arrays.copyOf(_randomDatas, 100), lobErrMsg);
        }
        if (!readDbClTask.isTaskSuccess()) {
            Assert.assertEquals(
                    readDbClTask.getSdbErrCode(),
                    SDBError.SDB_LOB_IS_IN_USE.getErrorCode(),
                    lobErrMsg + readDbClTask.getErrorMsg()
            );
        }
    }
}
