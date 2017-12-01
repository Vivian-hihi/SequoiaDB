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
import java.util.logging.Logger;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.*;

/**
 * Created by laojingtang on 17-11-22.
 */
public class LobTest13228 extends SdbTestBase {
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

    @DataProvider(name = "testLob13228DataProvider")
    public static Object[][] testLob13228DataProvider() {
        return new Object[][]{
                {1024, 1024, 100, DBLob.SDB_LOB_SEEK_SET},
                {1024, 1024, 100, DBLob.SDB_LOB_SEEK_CUR},
                {1024, 1024, 100, DBLob.SDB_LOB_SEEK_END},

                {1024, 1024, 1024, DBLob.SDB_LOB_SEEK_END},
                {1024, 1024, 1024, DBLob.SDB_LOB_SEEK_END},
                {1024, 1024, 1024, DBLob.SDB_LOB_SEEK_END},
        };
    }

    /**
     * 1、打开lob（创建模式新建lob）
     * 2、通过seek指定偏移量，偏移量分别验证如下场景：
     * a、同一个数据页内偏移写
     * b、跨多个数据页偏移写
     * c、偏移位置覆盖起始、中间、末尾（SDB_LOB_SEEK_SET/SDB_LOB_SEEK_CUR/SDB_LOB_SEEK_END）
     * 3、写入lob数据
     * 4、检查lob写入结果
     * 1、写入lob成功，查询lob数据信息和实际写入数据一致（比较MD5值）
     * 2、执行list查看显示lob size信息正确，包含seek size长度
     */
    @Test(dataProvider = "testLob13228DataProvider")
    public void testLob13228(int initLobSize, int appendDataSize, int seekSize, int seekType) {
        byte[] data = getRandomBytes(initLobSize);
        byte[] appendData = getRandomBytes(appendDataSize);
        ObjectId id = ObjectId.get();

        try (DBLob lob = dbcl.createLob(id)) {
            lob.write(data);
            lob.seek(seekSize, seekType);
            lob.write(appendData);
        }

        byte[] actual = readLob(dbcl,id);
        if (seekType == DBLob.SDB_LOB_SEEK_CUR)
            seekSize += initLobSize;
        if (seekType == DBLob.SDB_LOB_SEEK_END)
            seekSize = initLobSize - seekSize;
        assertByteArrayEqual(actual, RandomWriteLobUtil.appendBuff(data, appendData, seekSize));
    }

}
