package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.List;
import java.util.logging.Logger;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.*;

/**
 * Created by laojingtang on 17-12-1.
 */
public class TestLob13252 extends SdbTestBase {
    Logger log = Logger.getLogger(LobTest13237.class.getName());
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
     * 1、打开已存在lob对象
     * 2、多次执行锁定（lockAndSeek）、写lob操作，其中锁定数据段范围相同
     * 3、读取lob数据，检查lob数据操作结果
     * 1、写入lob成功，读取lob信息和实际插入信息一致（比较MD5值）；
     * 2、list查询lob
     * size信息正确
     *
     * @param lobsize
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13248(int lobsize) {
        ObjectId id = createEmptyLob(dbcl);

        byte[] bytes = getRandomBytes(lobsize);
        DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE);
        lob.lock(0, lobsize);
        lob.lock(0, lobsize);
        lob.lock(0, lobsize);
        lob.write(bytes);
        lob.close();

        assertByteArrayEqual(readLob(dbcl, id), bytes);
    }

}
