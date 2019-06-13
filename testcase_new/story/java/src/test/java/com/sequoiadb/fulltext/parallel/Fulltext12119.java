package com.sequoiadb.fulltext.parallel;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.fulltext.utils.FullTextDBUtils;
import com.sequoiadb.fulltext.utils.FullTextUtils;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ResultStore;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;

/**
 * @FileName seqDB-12119:并发删除集合空间
 * @Author
 * @Date liuxiaoxuan 2019.5.10
 */
public class Fulltext12119 extends SdbTestBase {
    private final int TIMEOUT = 600000;
    private Sequoiadb db = null;
    private List<DBCollection> cls = new ArrayList<>();
    private String textIndexName = "fulltext12119";
    ThreadExecutor te = new ThreadExecutor(TIMEOUT);

    @BeforeClass
    public void setUp() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(db)) {
            throw new SkipException("skip StandAlone");
        }

        // 创建集合空间和集合，总共两个集合空间，每个集合空间对应2个集合
        for (int csNo = 0; csNo < 2; csNo++) {
            String csName = "cs12119_" + csNo;
            if (db.isCollectionSpaceExist(csName)) {
                db.dropCollectionSpace(csName);
            }
            CollectionSpace cs = db.createCollectionSpace(csName);
            for (int clNo = 0; clNo < 2; clNo++) {
                String clName = "12119_cl_" + clNo;
                DBCollection cl = cs.createCollection(clName);
                FullTextDBUtils.insertData(cl, 10000);
                if (clNo % 2 > 0) {
                    BSONObject indexObj = new BasicBSONObject();
                    indexObj.put("a", "text");
                    cl.createIndex(textIndexName, indexObj, false, false);
                }
                cls.add(cl);
            }
        }
    }

    @AfterClass
    public void tearDown() {
        if (db != null) {
            db.close();
        }
    }

    @Test
    public void test() throws Exception {
        Assert.assertTrue(FullTextUtils.isIndexCreated(cls.get(1), textIndexName, 10000));
        Assert.assertTrue(FullTextUtils.isIndexCreated(cls.get(3), textIndexName, 10000));

        List<String> cappedNames = new ArrayList<>();
        List<String> esIndexNames = new ArrayList<>();
        cappedNames.add(FullTextDBUtils.getCappedName(cls.get(1), textIndexName));
        cappedNames.add(FullTextDBUtils.getCappedName(cls.get(3), textIndexName));
        esIndexNames.add(FullTextDBUtils.getESIndexName(cls.get(1), textIndexName));
        esIndexNames.add(FullTextDBUtils.getESIndexName(cls.get(3), textIndexName));

        List<DropCSThread> dropCSThreads = new ArrayList<>();
        for (int csNo = 0; csNo < 2; csNo++) {
            String csName = "cs12119_" + csNo;
            DropCSThread dropCSThread = new DropCSThread(csName);
            te.addWorker(dropCSThread);
            dropCSThreads.add(dropCSThread);
        }

        te.run();

        for (int i = 0; i < dropCSThreads.size(); i++) {
            if (0 != dropCSThreads.get(i).getRetCode()) {
                // 删除集合空间失败，全文索引仍然存在
                DBCollection cl = cls.get(i * 2 + 1);
                BSONObject matcher = (BSONObject) JSON.parse("{'':{'$Text':{'query':{'match_all':{}}}}}");
                DBCursor cursor = cl.query(matcher, null, null, null);

                try {
                    cursor = cl.query(matcher, null, null, null);
                    Assert.fail("query should fail");
                } catch (BaseException e) {
                    if (-6 != e.getErrorCode() && -52 != e.getErrorCode()) {
                        Assert.fail("actual exception: " + e.getErrorCode());
                    }
                } finally {
                    if (cursor != null) {
                        cursor.close();
                    }
                }
            } else {
                // 删除集合空间成功，全文索引被删除
                Assert.assertTrue(FullTextUtils.isIndexDeleted(db, esIndexNames.get(i), cappedNames.get(i)));
            }
        }
    }

    class DropCSThread extends ResultStore {
        private String csName = null;

        public DropCSThread(String csName) {
            this.csName = csName;
        }

        @ExecuteOrder(step = 1, desc = "删除集合空间")
        public void dropCS() {
            System.out.println(this.getClass().getName().toString() + " begin at:"
                    + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            try (Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                sdb.dropCollectionSpace(csName);
            } catch (BaseException e) {
                if (-147 != e.getErrorCode()) {
                    Assert.fail("actual exception: " + e.getErrorCode());
                }
                saveResult(e.getErrorCode(), e);
            } finally {
                System.out.println(this.getClass().getName().toString() + " end at:"
                        + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
            }
        }
    }
}
