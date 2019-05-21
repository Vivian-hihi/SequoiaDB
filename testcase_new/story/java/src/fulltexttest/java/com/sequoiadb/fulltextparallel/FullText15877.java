package com.sequoiadb.fulltextparallel;

import java.util.Date;
import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.elasticsearch.client.Client;
import org.testng.Assert;
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
import com.sequoiadb.utils.FullTextUtils;

/**
 * @description seqDB-15877:truncate集合记录与删除全文索引并发
 * @author huangxiaoni 2019.5.14
 * @modify
 */

public class FullText15877 extends SdbTestBase {
    private Random random = new Random();
    private final static String CL_NAME = "cl_es_15877";
    private final static String IDX_NAME = "idx_es_15877";
    private final static BSONObject IDX_KEY = new BasicBSONObject("a", "text");
    private final static int RECS_NUM = 20000;
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private String cappedCSName;
    private List<String> clRgNames;
    
    private Client esClient = null;
    private String esIndexName;
    

    @BeforeClass
    private void setUp() throws Exception {
        esClient = FullTextESUtils.createTransportClient(esHostName, 
                Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("Skip standAlone mode");
        }

        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        cl = cs.createCollection(CL_NAME);
        cl.createIndex(IDX_NAME, IDX_KEY, false, false);          
        cappedCSName = FullTextDBUtils.getCappedName(cl, IDX_NAME);
        clRgNames = FullTextDBUtils.getCLGroups(cl);
        System.out.println(this.getClass().getName() + " " + clRgNames);
        esIndexName  = FullTextDBUtils.getESIndexName(cl, IDX_NAME); 
        
        FullTextDBUtils.insertData(cl, RECS_NUM);
        
        // 确保预置的数据同步到es完成，避免test中查询的数据未同步完成导致非预期
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, RECS_NUM));
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor();
        ThreadTruncate threadTruncate = new ThreadTruncate();
        ThreadDropFullTextIndex threadDropIdx = new ThreadDropFullTextIndex();
        es.addWorker(threadTruncate);
        es.addWorker(threadDropIdx);
        es.run();
        
        // check results
        if (threadDropIdx.getRetCode() == 0) {
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCSName));  
            long cnt = cl.getCount();
            if (threadTruncate.getRetCode() == 0) {
                Assert.assertEquals(cnt, 0);
            } else {
                Assert.assertEquals(cnt, RECS_NUM); 
            }
        } 
        else if (threadDropIdx.getRetCode() != 0) {
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, 0));
        }
    }

    @AfterClass
    private void tearDown() throws InterruptedException {
        try {
            FullTextDBUtils.dropCollection(cs, CL_NAME);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCSName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    private class ThreadTruncate extends ResultStore {
        @ExecuteOrder(step = 1)
        private void truncate() throws InterruptedException {
            Thread.sleep(random.nextInt(50));
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl2.truncate();
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                if (e.getErrorCode() != -190 && e.getErrorCode() != -147) {
                    throw e;
                }
                saveResult(-1, e);
            }
        } 
    }

    private class ThreadDropFullTextIndex extends ResultStore{        
        @ExecuteOrder(step = 1)
        private void dropIndex() {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl2.dropIndex(IDX_NAME);
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                // -52 Index does not exist, maybe index rebuilding after truncate
                if (e.getErrorCode() != -321 && e.getErrorCode() != -52 
                        && e.getErrorCode() != -147) {
                    throw e;
                }
                saveResult(-1, e);
            }
        }
    }  
}