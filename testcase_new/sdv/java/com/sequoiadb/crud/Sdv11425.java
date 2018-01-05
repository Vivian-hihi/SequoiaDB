package com.sequoiadb.crud;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

/**
 * Created by laojingtang on 18-1-4.
 */
public class Sdv11425 extends SdbTestBase {
    private Sequoiadb db = null;
    private static final String CLNAME = Sdv11425.class.getSimpleName();
    private DBCollection dbcl;

    @BeforeClass
    public void setup() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        CollectionSpace cs = db.getCollectionSpace(SdbTestBase.csName);
        dbcl = cs.createCollection(CLNAME);
    }

    @AfterClass
    public void teardown() {
        if (db != null) {
            db.disconnect();
        }
    }

    @Test
    public void test() {
        SdbThreadBase insert = new SdbThreadBase() {
            @Override
            public void exec() throws Exception {
                Sequoiadb db = null;
                try {
                    db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                    DBCollection cl = db.getCollectionSpace(SdbTestBase.csName)
                            .getCollection(CLNAME);
                    for (int i = 0; i < 100; i++) {
                        cl.insert(new BasicBSONObject("b", i));
                    }
                } catch (BaseException e) {
                    if (e.getErrorCode() != -23) throw e;
                } finally {
                    if (db != null)
                        db.disconnect();
                }
            }
        };

        insert.start(20);

        db.getCollectionSpace(SdbTestBase.csName)
                .dropCollection(CLNAME);

        Assert.assertTrue(insert.isSuccess(), insert.getErrorMsg());
    }
}
