package com.sequoiadb.index;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;
import static org.testng.Assert.*;

/**
 * Created by laojingtang on 18-1-2.
 */
public class Index11413 extends SdbTestBase {
    final String clName = Index11413.class.getSimpleName();
    private Sequoiadb db = null;
    private DBCollection dbcl;


    @BeforeClass
    public void setup() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        dbcl = db.getCollectionSpace(SdbTestBase.csName).createCollection(clName);
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
     * 1、插入大量数据
     * 2、并发创建相同索引
     * 3、查看索引创建结果
     * 4、并发删除相同索引
     * 5、查看索引删除结果
     */
    @Test
    public void testCreateIndex() {

        SdbThreadBase createTasks = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11413.this.clName);
                    cl.createIndex("index11413", new BasicBSONObject("a", 1), false, false);
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };

        createTasks.start(20);
        createTasks.join();

        DBCursor cursor = dbcl.getIndex("index11413");
        BSONObject object = cursor.getNext();
        cursor.close();
        assertNotNull(object, "index11413");
    }

    @Test
    public void testRemoveIndex() {
        dbcl.createIndex("b_index", new BasicBSONObject("b", 1), false, false);

        SdbThreadBase removeTask = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11413.this.clName);
                    cl.dropIndex("b_index");
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };
        removeTask.start(20);
        removeTask.join();

        try (DBCursor curor = dbcl.getIndex("b_index")) {
            assertFalse(curor.hasNext(), "b_index");
        }
    }

}
