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

import static org.testng.Assert.assertFalse;
import static org.testng.Assert.assertNotNull;
import static org.testng.Assert.assertTrue;

/**
 * Created by laojingtang on 18-1-2.
 */
public class Index11416 extends SdbTestBase {
    final String CLNAME = Index11416.class.getSimpleName();
    private Sequoiadb db = null;
    private DBCollection dbcl;


    @BeforeClass
    public void setup() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        dbcl = db.getCollectionSpace(SdbTestBase.csName).createCollection(CLNAME);
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
        if (db != null){
            db.getCollectionSpace(SdbTestBase.csName).dropCollection(CLNAME);
            db.disconnect();
        }
    }

    /**
     * 1、多个线程并发执行查询操作（带索引查询）
     * 2、查询过程中并发执行创建、删除索引操作
     * 3、检查操作结果
     */
    @Test
    public void testCreateIndexAndQuery() {
        SdbThreadBase createTasks = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11416.this.CLNAME);
                    cl.createIndex("index11413", new BasicBSONObject("a", 1), false, false);
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };

        SdbThreadBase queryTask=new QueryTask("index11413");
        queryTask.start(20);
        createTasks.start();

        assertTrue(queryTask.isSuccess(),queryTask.getErrorMsg());
        assertTrue(createTasks.isSuccess(),createTasks.getErrorMsg());

        DBCursor cursor = dbcl.getIndex("index11413");
        BSONObject object = cursor.getNext();
        cursor.close();
        assertNotNull(object, "index11413");
    }

    @Test
    public void testRemoveIndexAndQuery() {
        dbcl.createIndex("b_index", new BasicBSONObject("b", 1), false, false);

        SdbThreadBase removeTask = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11416.this.CLNAME);
                    cl.dropIndex("b_index");
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };
        SdbThreadBase queryTask=new QueryTask("b_index");
        removeTask.start();
        queryTask.start(20);

        assertTrue(removeTask.isSuccess(),removeTask.getErrorMsg());
        assertTrue(queryTask.isSuccess(),queryTask.getErrorMsg());

        try (DBCursor curor = dbcl.getIndex("b_index")) {
            assertFalse(curor.hasNext(), "b_index");
        }
    }

    class QueryTask extends SdbThreadBase {

        private String indexName;

        public QueryTask(String indexName) {
            this.indexName = indexName;
        }

        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11416.this.CLNAME);
                cl.query(null, null, null, new BasicBSONObject("",indexName), 0, 10);
            } finally {
                if (db != null)
                    db.disconnect();
            }
        }
    }
}
