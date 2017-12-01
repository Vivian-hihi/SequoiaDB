package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.assertByteArrayEqual;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.getRandomBytes;

/**
 * Created by laojingtang on 17-11-22.
 */
public class LobTest13233 extends SdbTestBase {
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
        db = new Sequoiadb(coordUrl,"","");
        cs = db.getCollectionSpace(csName);
        List<String> groupNames = RandomWriteLobUtil.getDataGroups(db);

        dbcl = cs.createCollection(clName,
                (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\",Group:'" + groupNames.get(0) + "'}"));
        dbcl.split(groupNames.get(0), groupNames.get(1), 50);
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }

    @DataProvider(name = "test13233DataProvider")
    public static Object[][] test13233DataProvider() {
        return new Object[][]{
                {1024, 500, 500},
                {1024, 1023, 1},
                {1024, 1024, 500}
        };
    }

    /**
     * 1、打开已存在lob对象
     * 2、未加锁，seek指定偏移写入lob数据，其中写入lob数据覆盖如下场景：
     * a、新写lob数据长度小于原有lob
     * b、新写lob数据长度等于原有lob
     * c、新写lob数据长度大于原有lob
     * 3、检查操作结果
     * 1、写入lob成功，读取lob数据正确（比较MD5值相同）；分别检查如下：
     * a场景：读取lob数据包含新插入数据和未覆盖数据
     * bc场景：读取lob数据和新插入lob一致（全覆盖写入）
     */
    @Test(dataProvider = "test13233DataProvider")
    public void testLob13233(int lobSize, int newDataSize, int offset) {
        DBLob lob = dbcl.createLob();
        byte[] randomBytes = getRandomBytes(lobSize);
        lob.write(randomBytes);
        lob.close();
        ObjectId oid = lob.getID();

        lob = dbcl.openLob(oid, DBLob.SDB_LOB_WRITE);
        lob.seek(offset, DBLob.SDB_LOB_SEEK_SET);
        byte[] newData = getRandomBytes(newDataSize);
        lob.write(newData);
        lob.close();

        lob = dbcl.openLob(oid);
        byte[] actual = new byte[(int) lob.getSize()];
        lob.read(actual);
        lob.close();

        assertByteArrayEqual(actual, RandomWriteLobUtil.appendBuff(randomBytes, newData, offset));
    }
}
