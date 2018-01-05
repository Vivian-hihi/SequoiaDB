package com.sequoiadb.index;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * 用例要求：
 * 1、向cl中插入大量数据（如1千万条记录）
 * 2、创建离线方式ID索引，执行创建索引命令createIdIndex({SortBufferSize:16})
 * 3、创建索引过程中向该CL更新数据
 * 4、查看创建索引结果和数据更新结果
 *
 * @author huangwenhua
 * @version 1.00
 * @Date 2016.12.14
 */
public class IdIndex6614 extends SdbTestBase {
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String clName = "c6614";

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        createCL();
        insertData();
    }

    @Test
    public void upsertData() {
        UpdateTask updateTask = new UpdateTask();
        CreateIndex indexThread = new CreateIndex();
        updateTask.start();
        indexThread.start();

        updateTask.join();
        indexThread.join();
        Assert.assertTrue(indexThread.isSuccess());
    }

    class UpdateTask extends SdbThreadBase {

        public void exec() throws Exception {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(IdIndex6614.this.csName).getCollection(IdIndex6614.this.clName);
                cl.update(null, "{$set:{name:\"kkkk\"}}", null);
            } finally {
                if (db != null)
                    db.disconnect();
            }
        }
    }

    /**
     * 并发创建索引
     */
    class CreateIndex extends SdbThreadBase {
        @Override
        public void exec() throws BaseException {

            Sequoiadb db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            try {
                DBCollection cl1 = db1.getCollectionSpace(csName)
                        .getCollection(clName);
                BSONObject indexObj = (BSONObject) JSON
                        .parse("{SortBufferSize:16}");
                cl1.createIdIndex(indexObj);
                checkIdIndex(cl1);
            } finally {
                if (db1 != null) {
                    db1.disconnect();
                }
            }
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        } finally {
            if (sdb != null) {
                sdb.disconnect();
            }
        }
    }

    void createCL() {
        try {
            if (!sdb.isCollectionSpaceExist(SdbTestBase.csName)) {
                sdb.createCollectionSpace(SdbTestBase.csName);
            }
        } catch (BaseException e) {
            // -33 CS exist,ignore exceptions
            Assert.assertEquals(-33, e.getErrorCode(), e.getMessage());
        }
        try {
            String clOptions = "{ShardingKey:{no:1},ShardingType:'hash',Partition:1024,"
                    + "ReplSize:0,Compressed:true,AutoIndexId:false}";
            BSONObject options = (BSONObject) JSON.parse(clOptions);

            cs = sdb.getCollectionSpace(SdbTestBase.csName);
            cl = cs.createCollection(clName, options);
        } catch (BaseException e) {
            Assert.assertTrue(false, "create cl fail " + e.getErrorType() + ":"
                    + e.getMessage());
        }
    }

    void insertData() {
        for (int i = 0; i < 100000; i++) {
            BSONObject bson = new BasicBSONObject();
            bson.put("age", i);
            bson.put("name", "Json");
            cl.insert(bson);
        }
    }

    /**
     * 检查索引
     */
    void checkIdIndex(DBCollection cl) {
        DBCursor cursor1 = null;
        try {
            // 通过explain，判断是否走索引
            cursor1 = cl.explain(null, null, null,
                    (BSONObject) JSON.parse("{'':'$id'}"), 0, -1, 0, null);
            String scanType = null;
            String indexName = null;
            while (cursor1.hasNext()) {
                BSONObject record = cursor1.getNext();
                if (record.get("Name").equals(SdbTestBase.csName + "." + clName)) {
                    scanType = (String) record.get("ScanType");
                    indexName = (String) record.get("IndexName");
                }
            }
            Assert.assertEquals(scanType, "ixscan");
            Assert.assertEquals(indexName, "$id");
        } finally {
            if (cursor1 != null) {
                cursor1.close();
            }
        }
    }
}
