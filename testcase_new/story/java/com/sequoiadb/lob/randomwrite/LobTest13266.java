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
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.Arrays;
import java.util.List;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.assertByteArrayEqual;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.getRandomBytes;

/**
 * Created by laojingtang on 17-12-1.
 */
public class LobTest13266 extends SdbTestBase {
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
     * 1、共享模式下，多个连接多线程并发如下操作:
     * (1)打开已存在lob对象，seek指定偏移范围，执行lock锁定数据段，向锁定数据段写入lob
     * 多个并发线程中锁定数据段范围相同，写入数据信息不相同
     * 2、读取lob，检查操作结果
     * 1、先加锁线程写lob成功，查询lob信息按指定位置写入数据，且写入数据信息正确（比较MD5值）
     * 2、另一个线程加锁失败，返回对应错误信息
     *
     * @param lobsize
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13266(int lobsize) throws InterruptedException {
        final byte[] randomBytes = getRandomBytes(lobsize);
        DBLob lob = dbcl.createLob();
        final ObjectId oid = lob.getID();
        lob.close();

        DbLobWriteTask t1 = new DbLobWriteTask(csName, clName, oid, 10, randomBytes);
        DbLobWriteTask t2 = new DbLobWriteTask(csName, clName, oid, 10, randomBytes);

        t1.start();
        t2.start();
        t1.join();
        t2.join();

        if (t1.isTaskSuccess()) {
            Assert.assertEquals(t2.getSdbErrCode(), SDBError.SDB_LOB_LOCK_CONFLICTED.getErrorCode());
        } else {
            Assert.assertEquals(t1.getSdbErrCode(), SDBError.SDB_LOB_LOCK_CONFLICTED.getErrorCode());
        }

        lob = dbcl.openLob(oid);
        byte[] actual = new byte[lobsize + 10];
        lob.read(actual);
        lob.close();

        byte[] expect = randomBytes;
        assertByteArrayEqual(Arrays.copyOfRange(actual, 10, actual.length), expect);
    }

}
