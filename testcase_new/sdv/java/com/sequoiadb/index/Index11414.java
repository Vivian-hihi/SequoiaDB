package com.sequoiadb.index;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;

import static org.testng.Assert.*;

/**
 * Created by laojingtang on 18-1-2.
 */
public class Index11414 extends SdbTestBase {
    final String clName = "index11414";
    private Sequoiadb db = null;
    private DBCollection dbcl;

    @BeforeClass
    public void setup() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        dbcl = db.getCollectionSpace(SdbTestBase.csName).createCollection(clName);
    }

    private void prepareData() {
        //prepare data
        List<BSONObject> list = new ArrayList<>(10000);
        for (int i = 0; i < 10000; i++) {
            BSONObject obj = new BasicBSONObject()
                    .append("a", i)
                    .append("b", i)
                    .append("c", i)
                    .append("d", i);
            list.add(obj);
        }
        dbcl.insert(list);
    }

    @AfterClass
    public void teardown() {
        if (db != null)
            db.disconnect();
    }

    /**
     * 1、向cl中插入大量数据
     * 2、创建索引时并发删除记录，分别覆盖两种删除场景：
     * a、删除cl中所有记录
     * b、单条记录删除（单条删除集合中索引的记录）
     * 3、检查操作结果
     */
    @Test
    public void testCreateIndexAndRemoveAllRecord() {
        prepareData();

        SdbThreadBase createIndexTasks = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11414.this.clName);
                    cl.createIndex("index11414", new BasicBSONObject("a", 1), false, false);
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };
        SdbThreadBase removeAllRecordTask = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11414.this.clName);
                    cl.delete(new BasicBSONObject());
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };

        createIndexTasks.start();
        removeAllRecordTask.start(20);

        assertTrue(removeAllRecordTask.isSuccess(), removeAllRecordTask.getErrorMsg());
        assertTrue(createIndexTasks.isSuccess(), createIndexTasks.getErrorMsg());

        DBCursor cursor = dbcl.getIndex("index11414");
        BSONObject object = cursor.getNext();
        cursor.close();
        assertNotNull(object, "index11414");
    }

    @Test
    public void testCreateIndexAndRemoveRecords() {
        prepareData();

        SdbThreadBase createIndexTask = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11414.this.clName);
                    cl.createIndex("b_index", new BasicBSONObject("b", 1), false, false);
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };
        SdbThreadBase removeRecordTask = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11414.this.clName);
                    BasicBSONObject obj = (BasicBSONObject) cl.queryOne();
                    ObjectId id = obj.getObjectId("_id");
                    cl.delete(new BasicBSONObject("_id", id));
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };
        createIndexTask.start();
        removeRecordTask.start(20);

        assertTrue(createIndexTask.isSuccess(), createIndexTask.getErrorMsg());
        assertTrue(removeRecordTask.isSuccess(), removeRecordTask.getErrorMsg());

        DBCursor cur = dbcl.getIndex("b_index");
        assertNotNull(cur.getNext());
        cur.close();
    }
}
