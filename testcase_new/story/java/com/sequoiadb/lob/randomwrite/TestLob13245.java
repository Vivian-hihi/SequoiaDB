package com.sequoiadb.lob.randomwrite;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
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
import java.util.logging.Logger;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.*;

/**
 * Created by laojingtang on 17-12-1.
 */
public class TestLob13245 extends SdbTestBase {
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
     * 1、重新打开已存在lob
     * 2、seek指定写lob偏移量
     * 3、锁定（lock（））数据段
     * 4、写入lob，其中写入lob超过锁定范围（存在不在锁定范围内的数据）
     * 5、检查操作结果
     * 1、写入lob失败，查询lob信息和原有lob信息一致
     *
     * @param lobsize
     */
    @Test(enabled = false,dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13245(long lobsize) {
        ObjectId id = createEmptyLob(dbcl);

        byte[] bytes = getRandomBytes((int) lobsize + 100);

        try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
            lob.lock(0, lobsize);
            try {
                lob.write(bytes);
                Assert.fail("should throw exception!");
            } catch (BaseException e) {
                if (e.getErrorCode() != SDBError.SDB_INVALIDARG.getErrorCode())
                    throw e;
            }
        }
        try(DBLob lob=dbcl.openLob(id)) {
            if(lob.getSize()!=0){
                byte[] actual=new byte[(int)lobsize];
                lob.read(actual);
                Assert.assertEquals(actual, Arrays.copyOf(bytes,(int)lobsize));
            }

        }
    }
}
