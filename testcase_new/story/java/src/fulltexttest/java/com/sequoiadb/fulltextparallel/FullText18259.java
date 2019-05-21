package com.sequoiadb.fulltextparallel;

import java.util.Date;
import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ResultStore;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;

/**
 * @FileName seqDB-18259:同一集合并发创建删除相同的普通索引
 * @Author huangxiaoni 2019.5.9
 */

public class FullText18259 extends SdbTestBase {
    private final static int THREAD_NUM = 5;
    private final static String CL_NAME = "cl_es_18259";
    private final static String IDX_NAME = "cl_es_18259";
    private final static BSONObject IDX_KEY = 
            (BSONObject) JSON.parse("{a:1,b:-1,c:1,d:-1}");
    private final static int INSERT_RECS_NUM = 50000;
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private List<String> rgNames;
    
    private Client esClient = null;

    @BeforeClass
    private void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("Skip standAlone mode");
        }

        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        cl = cs.createCollection(CL_NAME);
        rgNames = FullTextDBUtils.getCLGroups(cl);
        FullTextDBUtils.insertData(cl, INSERT_RECS_NUM);
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor();
        for (int i = 0; i < THREAD_NUM; i++) {
            es.addWorker(new ThreadCreateIndex());
            es.addWorker(new ThreadDropIndex());
        }
        es.run();
        
        // check index
        CommLib commlib = new CommLib();
        commlib.checkIndex(sdb, IDX_NAME, CL_NAME);
        commlib.compareNodeData(sdb, rgNames.get(0), IDX_NAME, CL_NAME, null);
    }

    @AfterClass
    private void tearDown() {
        try {
            cs.dropCollection(CL_NAME);
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private class ThreadCreateIndex {
        @ExecuteOrder(step = 1)
        private void createIndex() {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl2.createIndex(IDX_NAME, IDX_KEY, false, false);
            } catch (BaseException e) {
                if (e.getErrorCode() != -247 && e.getErrorCode() != -199) {
                    throw e;
                }
            }
            System.out.println(new Date() + " end   " + this.getClass().getName().toString());
        }
    }

    private class ThreadDropIndex extends ResultStore {
        private Random random = new Random();
        
        @ExecuteOrder(step = 1)
        private void dropIndex() throws InterruptedException {
            Thread.sleep( random.nextInt(200) );
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl2.dropIndex(IDX_NAME);
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                if (e.getErrorCode() != -47 && e.getErrorCode() != -147) {
                    throw e;
                }
            }
        }
    }
}
