package com.sequoiadb.lob.randomwrite;

import static com.sequoiadb.lob.utils.RandomWriteLobUtil.assertByteArrayEqual;
import static com.sequoiadb.lob.utils.RandomWriteLobUtil.createEmptyLob;
import static com.sequoiadb.lob.utils.RandomWriteLobUtil.getRandomBytes;
import static com.sequoiadb.lob.utils.RandomWriteLobUtil.readLob;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.logging.Logger;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * Created by laojingtang on 17-12-1.
 */
public class RewriteLob13268 extends SdbTestBase {
    Logger log = Logger.getLogger(RewriteLob13268.class.getName());
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

        dbcl = cs.createCollection(clName, (BSONObject) JSON
                .parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\",Group:'" + groupNames.get(0) + "'}"));
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }

    /**
     * 1、共享模式下，多个连接多线程并发如下操作: (1)打开已存在lob对象，seek指定偏移范围，执行lock锁定数据段，向锁定数据段写入lob
     * 多个并发线程中锁定数据段范围不冲突 2、读取lob，检查操作结果
     * 1、所有线程写入lob成功，查询lob信息按指定位置写入数据，且写入数据信息正确（比较MD5值）
     */
    public void testLob13268() throws InterruptedException {
        final ObjectId id = createEmptyLob(dbcl);

        List<byte[]> datas = new ArrayList<>(10);
        List<DbLobWriteTask> tasks = new ArrayList<>(10);
        int step = 1024;
        for (int i = 0; i < 10; i++) {
            byte[] data = getRandomBytes(step);
            datas.add(data);
            tasks.add(new DbLobWriteTask(csName, clName, id, i * step, data));
        }

        for (DbLobWriteTask task : tasks)
            task.start();
        for (DbLobWriteTask task : tasks)
            task.join();
        for (DbLobWriteTask task : tasks)
            Assert.assertTrue(task.isTaskSuccess(), task.getErrorMsg());

        byte[] actualData = readLob(dbcl, id);
        for (int i = 0; i < 10; i++) {
            assertByteArrayEqual(Arrays.copyOfRange(actualData, i * step, i * step + step), datas.get(i));
        }
    }
}
