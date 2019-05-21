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
 * @description seqDB-15876:truncate集合记录与删除集合空间并发
 * @author huangxiaoni 2019.5.14
 * @modify
 */

public class FullText15876 extends SdbTestBase {
    private Random random = new Random();
    private final static String CS_NAME = "cs_es_15876";
    private final static String CL_NAME = "cl_es_15876";
    private final static String IDX_NAME = "idx_es_15876";
    private final static BSONObject IDX_KEY = new BasicBSONObject("a", "text");
    private final static int RECS_NUM = 20000;
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private String cappedCSName;
    private List<String> clRgNames;
    
    private Client esClient = null;
    private String esIndexName;
    private int lid;
    

    @BeforeClass
    private void setUp() throws Exception {
        esClient = FullTextESUtils.createTransportClient(esHostName, 
                Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("Skip standAlone mode");
        }

        cs = sdb.createCollectionSpace(CS_NAME);
        cl = cs.createCollection(CL_NAME); 
        cl.createIndex(IDX_NAME, IDX_KEY, false, false);          
        cappedCSName = FullTextDBUtils.getCappedName(cl, IDX_NAME);
        clRgNames = FullTextDBUtils.getCLGroups(cl);
        System.out.println(this.getClass().getName() + " " + clRgNames);
        esIndexName  = FullTextDBUtils.getESIndexName(cl, IDX_NAME); 
        
        FullTextDBUtils.insertData(cl, RECS_NUM);
        
        // 确保预置的数据同步到es完成，避免获取lids报索引不存在
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, RECS_NUM));
        lid = FullTextESUtils.getCommitCLLIDFromES(esClient, esIndexName);
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor();
        ThreadTruncate threadTruncate = new ThreadTruncate();
        ThreadDropCS   threadDropCS   = new ThreadDropCS();
        es.addWorker(threadTruncate);
        es.addWorker(threadDropCS);
        es.run();
        
        // check results
        if (threadDropCS.getRetCode() == 0) {
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCSName));
        } else {
            Assert.assertTrue(FullTextUtils.isFulltextRebuild(esClient, esIndexName, lid));
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, IDX_NAME, 0));
        }
    }

    @AfterClass
    private void tearDown() throws InterruptedException {
        try {
            try {
                sdb.dropCollectionSpace(CS_NAME);
            } catch (BaseException e) {
                if (e.getErrorCode() != -34) {
                    throw e;
                }
            }
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

    private class ThreadTruncate {        
        @ExecuteOrder(step = 1)
        private void truncate() {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(CS_NAME).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                cl2.truncate();
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                if (e.getErrorCode() != -23 && e.getErrorCode() != -248
                        && e.getErrorCode() != -190 && e.getErrorCode() != -147) {
                    throw e;
                }
            }
        } 
    }

    private class ThreadDropCS extends ResultStore {
        @ExecuteOrder(step = 1)
        private void dropCS() throws InterruptedException {
            Thread.sleep(random.nextInt(10));
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {      
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                db.dropCollectionSpace(CS_NAME);
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            } catch (BaseException e) {
                if (e.getErrorCode() != -147) {
                    throw e;
                }
                saveResult(-1, e);
            }
        }
    }
}