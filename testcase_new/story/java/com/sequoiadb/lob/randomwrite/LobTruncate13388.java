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

import java.util.Arrays;
import java.util.List;

/**
 * Created by laojingtang on 17-11-20.
 */
public class LobTruncate13388 extends SdbTestBase {
    String csName, clName;
    Sequoiadb db;
    CollectionSpace cs;
    DBCollection dbcl;
    final int lobSize = 1024;
    final byte[] _randomDatas = RandomWriteLobUtil.getRandomBytes(lobSize);
    List<String> groupNames;

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
     * 1、指定oid执行truncateLob操作，删除超过指定长度部分的数据
     * 2、检查操作结果（读取lob，查看lob对象长度，执行listLobs查看lobsize信息）
     * 1、执行truncateLob成功，读取lob数据为truncate后的数据一致
     * 2、执行listLobs查看lob大小为truncate操作时指定大小
     */
    @Test
    public void testLob13388() {
        ObjectId id;
        try (DBLob lob = dbcl.createLob()) {
            lob.write(_randomDatas);
            id = lob.getID();
        }

        dbcl.truncateLob(id, 100);

        try (DBLob lob = dbcl.openLob(id)) {
            Assert.assertEquals(lob.getSize(), 100);
            RandomWriteLobUtil.assertByteArrayEqual(RandomWriteLobUtil.readLob(lob,5), Arrays.copyOf(_randomDatas, 100));
        }
    }
}
