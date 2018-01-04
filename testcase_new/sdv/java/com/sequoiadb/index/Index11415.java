package com.sequoiadb.index;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
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
public class Index11415 extends SdbTestBase {
    final String clName = Index11415.class.getSimpleName();
    private Sequoiadb db = null;
    private DBCollection dbcl;

    @BeforeClass
    public void setup() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        dbcl = db.getCollectionSpace(SdbTestBase.csName).createCollection(clName);
    }

    @AfterClass
    public void teardown() {
        if (db != null)
            db.disconnect();
    }


    /**
     * 1、向cl中插入记录，过程中并发删除索引（插入记录中包含索引键）
     * 2、检查操作结果
     */
    @Test
    public void testRemoveIndex() {
        dbcl.createIndex("b_index", new BasicBSONObject("b", 1), false, false);

        SdbThreadBase removeIndexTask = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11415.this.clName);
                    cl.dropIndex("b_index");
                } catch (BaseException e) {
                    if(e.getErrorCode()!=-47)
                        throw e;
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };
        SdbThreadBase insertClTask = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName).getCollection(Index11415.this.clName);
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
                    cl.insert(list);
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };
        insertClTask.start(10);
        removeIndexTask.start(20);

        assertTrue(insertClTask.isSuccess(), insertClTask.getErrorMsg());
        assertTrue(removeIndexTask.isSuccess(), removeIndexTask.getErrorMsg());

        DBCursor curor = dbcl.getIndex("b_index");
        assertFalse(curor.hasNext(), "b_index");
    }

}
