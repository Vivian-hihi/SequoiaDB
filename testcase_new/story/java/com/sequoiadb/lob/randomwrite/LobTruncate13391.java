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
import java.util.logging.Logger;

/**
 * Created by laojingtang on 17-11-16.
 */
public class LobTruncate13391 extends SdbTestBase {
    Logger log = Logger.getLogger(LobTruncate13391.class.getSimpleName());
    String csName, clName;
    Sequoiadb db;
    CollectionSpace cs;
    DBCollection dbcl;
    final int lobSize = 1024;
    final byte[] _randomDatas = RandomWriteLobUtil.getRandomBytes(lobSize);
    List<String> groupNames;

    @BeforeClass
    public void setupClass() {
        db = new Sequoiadb(SdbTestBase.coordUrl,"","");
        csName = SdbTestBase.csName;
        clName = "cl_" + this.getClass().getSimpleName();
        cs = db.getCollectionSpace(csName);
        groupNames = RandomWriteLobUtil.getDataGroups(db);
        if (groupNames.size() < 2)
            throw new SkipException("");
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
     * 1、指定oid执行truncateLob操作，删除超过指定长度部分的数据，其中truncate数据切片覆盖源组和目标组
     * 2、检查操作结果（读取lob，查看lob对象长度，执行listLobs查看lobsize信息）
     * 1、执行truncateLob成功，读取lob数据为truncate后的数据一致
     * 2、执行listLobs查看lob大小为truncate操作时指定大小
     */
    @Test
    public void testLob13391() {
        dbcl.split(groupNames.get(0), groupNames.get(1), 50);
        ObjectId id;
        try (DBLob lob = dbcl.createLob()) {
            lob.write(_randomDatas);
            id = lob.getID();
        }

        dbcl.truncateLob(id, 100);

        try (DBLob lob = dbcl.openLob(id)) {
            Assert.assertEquals(lob.getSize(), 100);
            RandomWriteLobUtil.assertByteArrayEqual(RandomWriteLobUtil.readLob(lob), Arrays.copyOf(_randomDatas, 100));
        }
    }


}

