package com.sequoiadb.lob.randomwrite;

import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.assertByteArrayEqual;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.createEmptyLob;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.getRandomBytes;
import static com.sequoiadb.lob.randomwrite.RandomWriteLobUtil.readLob;

import java.util.logging.Logger;

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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * Created by laojingtang on 17-12-1.
 */
public class TestLob13255 extends SdbTestBase {
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

        dbcl = cs.createCollection(clName,
                (BSONObject) JSON.parse("{ShardingKey:{\"_id\":1},ShardingType:\"hash\"}"));
    }

    @AfterClass
    public void afterClass() {
        cs.dropCollection(clName);
        db.close();
    }

    /**
     * 1、重新打开lob
     * 2、指定范围锁定数据段（lockAndSeek）
     * 3、写入lob
     * 4、过程中删除lob（如在写入lob数据，未close前执行删除）
     * 5、检查写入和删除lob结果
     * 1、写入lob成功，删除lob失败，返回对应错误信息
     * 2、读取lob检查写入lob信息正确
     *
     * @param lobsize
     */
    @Test(dataProvider = "lobSizeDataProvider", dataProviderClass = RandomWriteLobUtil.LobSizedataProvider.class)
    public void testLob13252(int lobsize) {
        ObjectId id = createEmptyLob(dbcl);

        byte[] randomByte = getRandomBytes(lobsize);

        try (DBLob lob = dbcl.openLob(id, DBLob.SDB_LOB_WRITE)) {
            lob.lockAndSeek(0, lobsize);
            lob.write(randomByte);
            try {
                dbcl.removeLob(id);
                Assert.fail("should throw exception");
            } catch (BaseException e) {
                if (e.getErrorCode() != SDBError.SDB_LOB_IS_IN_USE.getErrorCode()) {
                    log.warning(e.getMessage());
                    throw e;
                }
            }
        }

        try {
            assertByteArrayEqual(readLob(dbcl, id), randomByte, "oid: " + id.toString());
        } catch (Throwable e) {
            e.printStackTrace();
            System.exit(0);
        }
    }

}
