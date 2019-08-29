package com.sequoiadb.lob.randomwrite;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.lob.utils.RandomWriteLobUtil;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Description seqDB-13274:并发读lob，其中指定数据段范围相同
 * @author laojingtang
 * @UpdateAuthor wuyan
 * @Date 2017.11.22
 * @UpdateDate 2018.08.26
 * @version 1.10
 */
public class LobTest13274 extends SdbTestBase {
    private Sequoiadb db = null;
    private DBCollection dbcl = null;
    private CollectionSpace cs = null;
    private String clName = "lob_13274";

    @BeforeClass
    public void setUp() {
        db = new Sequoiadb(coordUrl, "", "");
        cs = db.getCollectionSpace(SdbTestBase.csName);
        dbcl = cs.createCollection(clName, (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\"}"));
    }

    @Test
    public void testLob13274() throws InterruptedException {
        int lobsize = 1024 * 1024 * 2;
        byte[] expectBytes = RandomWriteLobUtil.getRandomBytes(lobsize);
        DBLob lob = dbcl.createLob();
        final ObjectId oid = lob.getID();
        lob.write(expectBytes);
        lob.close();

        List<DbLobReadTask> lobTasks = new ArrayList<>(10);

        int begin = 1024 * 10;
        int end = 1024 * 1024;
        for (int i = 0; i < 10; i++)
            lobTasks.add(new DbLobReadTask(SdbTestBase.csName, clName, begin, end, oid));

        for (DbLobReadTask lobTask : lobTasks)
            lobTask.start();
        for (DbLobReadTask lobTask : lobTasks)
            lobTask.join();

        for (DbLobReadTask lobTask : lobTasks) {
            Assert.assertTrue(lobTask.isTaskSuccess(), lobTask.getErrorMsg());
            int b = lobTask.getBegin();
            int length = lobTask.getLength();
            byte[] expect = Arrays.copyOfRange(expectBytes, b, b + length);
            byte[] actual = lobTask.getResult();
            RandomWriteLobUtil.assertByteArrayEqual(actual, expect);
        }
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }

}
