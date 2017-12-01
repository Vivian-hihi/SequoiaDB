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

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.logging.Logger;

/**
 * Created by laojingtang on 17-11-20.
 */
public class LobTruncate13394 extends SdbTestBase {
    Logger log = Logger.getLogger(LobTruncate13394.class.getSimpleName());
    private String csName, clName;
    Sequoiadb db;
    CollectionSpace cs;
    DBCollection dbcl;
    final int lobSize = 1024;
    final byte[] _randomDatas = RandomWriteLobUtil.getRandomBytes(lobSize);
    private List<String> groupNames;

    @BeforeClass
    public void setupClass() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        csName = SdbTestBase.csName;
        clName = "cl_" + this.getClass().getSimpleName();
        cs = db.getCollectionSpace(csName);
        groupNames = RandomWriteLobUtil.getDataGroups(db);
        dbcl = cs.createCollection(
                clName,
                (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\",Group:'" + groupNames.get(0) + "'}")
        );
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }


    /**
     * 1、多个连接多线程并发如下操作：
     * （1）创建lob，写入lob数据
     * （2）truncate删除lob（lob写入过程中并发truncate操作）
     * 2、检查写入和truncatelob结果
     * 1、写入lob成功，truncate删除lob失败，返回对应错误信息，查询写入lob数据正确
     */
    @Test
    public void testLob13394() throws InterruptedException {
        final ObjectId id = ObjectId.get();

        final AtomicBoolean canTruncate = new AtomicBoolean(false);

        DbClOperateTask createDbClTask = new DbClOperateTask(csName, clName) {
            @Override
            protected void exec() throws Exception {
                try (DBLob lob = this.dbcl.createLob(id)) {
                    canTruncate.set(true);
                    lob.write(_randomDatas);
                }
            }
        };

        DbClOperateTask truncateLob = new DbClOperateTask(csName, clName) {
            @Override
            protected void exec() throws Exception {
                while (!canTruncate.get()) {
                    Thread.sleep(100);
                }
                this.dbcl.truncateLob(id, 100);
            }
        };

        createDbClTask.start();
        truncateLob.start();
        createDbClTask.join();
        truncateLob.join();

        String lobErrMsg = "lob id: " + id.toString();
        Assert.assertTrue(createDbClTask.isTaskSuccess(), createDbClTask.getErrorMsg());

        if (truncateLob.isTaskSuccess())
            RandomWriteLobUtil.assertByteArrayEqual(RandomWriteLobUtil.readLob(dbcl, id), Arrays.copyOf(_randomDatas, 100), lobErrMsg);
        else {
            Assert.assertEquals(truncateLob.getSdbErrCode(), SDBError.SDB_LOB_IS_IN_USE.getErrorCode(), lobErrMsg);
            RandomWriteLobUtil.assertByteArrayEqual(RandomWriteLobUtil.readLob(dbcl, id), _randomDatas, lobErrMsg);
        }
    }

}
