package com.sequoiadb.fulltextparallel;

import java.util.ArrayList;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.ObjectId;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

/**
 * @FileName seqDB-15854:集合中存在全文索引，增删改/全文检索/查询记录/lob操作时split集合
 * @Author huangxiaoni 
 * @Date 2019.5.8
 */

public class FullText15854 extends SdbTestBase {
    private final static int TIMEOUT = 300000; // 5min
    
    private final static String CL_NAME = "cl_es_15854";
    private final static String FULLTEXT_IDX_NAME = "idx_es_15854";
    private final static BSONObject FULLTEXT_IDX_KEY = 
            (BSONObject) JSON.parse("{a:'text',b:'text',c:'text'}");
    private final static int INSERT_RECS_NUM = 20000;
    private final static int INSERT_BATCH_RECS_NUM = 20000;
    private final static int LOB_NUM = 20;
    
    private Sequoiadb sdb = null;
    private CollectionSpace cs;
    private DBCollection cl;
    private String cappedCSName;
    private String srcRgName;
    private String dstRgName;
    private ArrayList<ObjectId> lobIds1 = new ArrayList<>();
    private ArrayList<ObjectId> lobIds2 = new ArrayList<>();
    
    private Client esClient = null;
    private String esIndexName;
    

    @BeforeClass
    private void setUp() throws Exception {
        esClient = FullTextESUtils.createTransportClient(esHostName, 
                Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");

        if (CommLib.isStandAlone(sdb) || CommLib.OneGroupMode(sdb)) {
            throw new SkipException("The mode is standlone, or only one group, skip the testCase.");
        }
        
        ArrayList<String> rgNames = CommLib.getDataGroupNames(sdb);
        srcRgName = rgNames.get(0);
        dstRgName = rgNames.get(1);

        cs = sdb.getCollectionSpace(SdbTestBase.csName); 
        BSONObject options = new BasicBSONObject();
        options.put("ShardingType", "hash");
        options.put("ShardingKey", new BasicBSONObject("a", 1));
        options.put("Group", srcRgName);
        cl = cs.createCollection(CL_NAME, options); 
        cl.createIndex(FULLTEXT_IDX_NAME, FULLTEXT_IDX_KEY, false, false);
        cappedCSName = FullTextDBUtils.getCappedName(cl, FULLTEXT_IDX_NAME); 
        esIndexName  = FullTextDBUtils.getESIndexName(cl, FULLTEXT_IDX_NAME);
        
        FullTextDBUtils.insertData(cl, INSERT_RECS_NUM);
        
        for (int i = 0; i < LOB_NUM; i++) {
            ObjectId lobId = createLob( cl );
            lobIds1.add(lobId);
        }
        for (int i = 0; i < LOB_NUM; i++) {
            ObjectId lobId = createLob( cl );
            lobIds2.add(lobId);
        }
        
        // 确保预置的数据同步到es完成，避免test中查询的数据未同步完成导致非预期
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, FULLTEXT_IDX_NAME, INSERT_RECS_NUM));        
    }

    @Test
    private void test() throws Exception {
        ThreadExecutor es = new ThreadExecutor(TIMEOUT);
        es.addWorker(new ThreadInsert());          
        es.addWorker(new ThreadDelete());        
        es.addWorker(new ThreadUpdate()); 
        es.addWorker(new ThreadFullTextSearch()); 

        es.addWorker(new ThreadPutLob()); 
        es.addWorker(new ThreadRemoveLob()); 
        es.addWorker(new ThreadGetLob()); 
        
        es.addWorker(new ThreadSplit());
        
        es.run();
        //分别在每个并发线程检查数据对应操作的数据正确性。在 ThreadFullTextSearch 线程 step2 检查数据一致性。
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

    private class ThreadInsert {
        @ExecuteOrder(step = 1)
        private void insert() {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                insertRecords(cl2);
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            }
        }
    }
    
    private class ThreadDelete {
        private Sequoiadb db = null;
        private DBCollection cl2;
        
        private ThreadDelete() {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
        }
        
        @ExecuteOrder(step = 1, desc = "删除数据")
        private void delete() {
            BSONObject matcher = new BasicBSONObject("recordId", 
                    new BasicBSONObject("$gte", INSERT_RECS_NUM / 2)); 
            System.out.println(new Date() + " begin " + this.getClass().getName().toString());
            cl2.delete(matcher);
            System.out.println(new Date() + " end   " + this.getClass().getName().toString());
        }
        
        @ExecuteOrder(step = 3, desc = "校验删除后的记录数")
        private void checkResults() {
            BSONObject matcher = new BasicBSONObject("recordId", 
                    new BasicBSONObject("$lt", INSERT_RECS_NUM / 2)); 
            long cnt = cl2.getCount(matcher);
            Assert.assertEquals(cnt, INSERT_RECS_NUM * 2 - INSERT_RECS_NUM / 2);
        }
        
        @ExecuteOrder(step = 4, desc = "关闭连接")
        private void closeDB() {
            if (db != null) db.close();
        }
    }

    private class ThreadUpdate {
        private String upVal = StringUtils.getRandomString( 16 );
        private Sequoiadb db = null;
        private DBCollection cl2;
        
        private ThreadUpdate() {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
        }
        
        @ExecuteOrder(step = 1, desc = "更新记录")
        private void update() {
            BSONObject matcher = new BasicBSONObject("c", new BasicBSONObject("$exists", 1));
            BSONObject modifier = new BasicBSONObject("$set", 
                    new BasicBSONObject("b", upVal));
            BSONObject hint = new BasicBSONObject("", FULLTEXT_IDX_NAME);
            System.out.println(new Date() + " begin " + this.getClass().getName().toString());
            cl2.update(matcher, modifier, hint);
            System.out.println(new Date() + " end   " + this.getClass().getName().toString());
        }
        
        @ExecuteOrder(step = 3, desc = "校验更新后的记录数")
        private void checkResults() {
            BSONObject matcher = new BasicBSONObject("b", upVal); 
            long cnt = cl2.getCount(matcher);
            Assert.assertEquals(cnt, INSERT_RECS_NUM / 2);
        }
        
        @ExecuteOrder(step = 4, desc = "关闭连接")
        private void closeDB() {
            if (db != null) db.close();
        }
    }

    private class ThreadFullTextSearch {
        private Sequoiadb db = null;
        private DBCollection cl2;
        private Client esClient2 = null;
        private BSONObject matcher;
        
        private ThreadFullTextSearch() {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
            esClient2 = FullTextESUtils.createTransportClient(esHostName, 
                    Integer.parseInt(esServiceName));
        }
        
        @ExecuteOrder(step = 1, desc = "全文检索")
        private void fullTextSearch() {
            matcher = new BasicBSONObject("", new BasicBSONObject("$Text", 
                    new BasicBSONObject("query", 
                            new BasicBSONObject("match", 
                                    new BasicBSONObject("a", CL_NAME)))));
            System.out.println(new Date() + " begin " + this.getClass().getName().toString());
            DBCursor cursor = cl2.query(matcher, null, null, null);
            int rcRecsNum = 0;
            while (cursor.hasNext()) {
                cursor.getNext();
                rcRecsNum++;
            }                
            if (rcRecsNum < INSERT_RECS_NUM / 2) {
                Assert.fail("expect records numer less, rcRecsNum: " + rcRecsNum + 
                        ", expect num: " + INSERT_RECS_NUM / 2);
            }
            System.out.println(new Date() + " end   " + this.getClass().getName().toString());
        }

        @ExecuteOrder(step = 2, desc = "检查查询返回结果")
        private void waitSync() throws Exception {
            Assert.assertTrue(FullTextUtils.isIndexCreated(esClient2, cl2, FULLTEXT_IDX_NAME, 
                    INSERT_RECS_NUM * 2 - INSERT_RECS_NUM / 2));
        }

        @ExecuteOrder(step = 3, desc = "再次全文检索")
        private void queryAgain() throws InterruptedException {  
            int rcRecsNum = 0;
            DBCursor cursor = cl2.query(matcher, null, null, null);
            while (cursor.hasNext()) {
                cursor.getNext();
                rcRecsNum++;
            }            
            Assert.assertEquals(rcRecsNum, INSERT_RECS_NUM / 2 );
        }
        
        @ExecuteOrder(step = 4, desc = "关闭连接")
        private void closeDB() {
            try {
                db.close();
            } finally {
                esClient2.close();
            }
        }
    }

    private class ThreadPutLob {
        private final static int NEW_LOB_NUM = 10;
        private ArrayList<ObjectId> lobIds3 = new ArrayList<>();
        private Sequoiadb db = null;
        private DBCollection cl2;
        
        private ThreadPutLob() {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
        }
        
        @ExecuteOrder(step = 1)
        private void pubLob() {
            System.out.println(new Date() + " begin " + this.getClass().getName().toString());
            for (int i = 0; i < NEW_LOB_NUM; i++) {
                ObjectId id = createLob( cl2 );
                lobIds3.add(id);
            }
            System.out.println(new Date() + " end   " + this.getClass().getName().toString());
        }
        
        @ExecuteOrder(step = 2, desc = "检查新增lob")
        private void checkResults() {
            DBCursor cursor = cl2.listLobs();
            int rcLobNum = 0;
            while (cursor.hasNext()) {
                cursor.getNext();
                rcLobNum++;
            }
            Assert.assertEquals(rcLobNum, LOB_NUM + NEW_LOB_NUM);

            for (ObjectId lobId : lobIds3) {
                DBLob lob = cl2.openLob(lobId);
                lob.close();
            }            
        }
        
        @ExecuteOrder(step = 3, desc = "关闭连接")
        private void closeDB() {
            if (db != null) db.close();
        }
    }

    private class ThreadRemoveLob {
        private Sequoiadb db = null;
        private DBCollection cl2;
        
        private ThreadRemoveLob() {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
        }
        
        @ExecuteOrder(step = 1)
        private void removeLob() {
            System.out.println(new Date() + " begin " + this.getClass().getName().toString());
            for (ObjectId lobId : lobIds1) {
                cl2.removeLob(lobId);
            }
            System.out.println(new Date() + " end   " + this.getClass().getName().toString());
        }
        
        @ExecuteOrder(step = 2, desc = "检查删除后的lob")
        private void checkResults() {
            for (ObjectId lobId : lobIds1) {
                try {
                    cl2.openLob(lobId);
                    Assert.fail("expect fail but succ.");
                } catch (BaseException e) {
                    if (e.getErrorCode() != -4) {
                        throw e;
                    }
                }
            }
        }
        
        @ExecuteOrder(step = 3, desc = "关闭连接")
        private void closeDB() {
            if (db != null) db.close();
        }
    }

    private class ThreadGetLob {
        @ExecuteOrder(step = 1)
        private void getLob() {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
                DBCollection cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
                System.out.println(new Date() + " begin " + this.getClass().getName().toString());
                for (ObjectId lobId : lobIds2) {
                    DBLob lob = cl2.openLob(lobId);
                    ObjectId id = lob.getID();
                    lob.close();
                    Assert.assertEquals(id, lobId);
                }
                System.out.println(new Date() + " end   " + this.getClass().getName().toString());
            }
        }
    }

    private class ThreadSplit {
        private Sequoiadb db = null;
        private DBCollection cl2;
        
        private ThreadSplit() {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl2 = db.getCollectionSpace(SdbTestBase.csName).getCollection(CL_NAME);
        }
        
        @ExecuteOrder(step = 1)
        private void createIndex() throws InterruptedException {
            System.out.println(new Date() + " begin " + this.getClass().getName().toString());
            cl2.split(srcRgName, dstRgName, 50);
            System.out.println(new Date() + " end   " + this.getClass().getName().toString());
        }
        
        @ExecuteOrder(step = 3, desc = "校验切分后的数据")
        private void checkResults() {
            Sequoiadb srcRG = null;
            Sequoiadb dstRG = null;
            try {
                srcRG = db.getReplicaGroup(srcRgName).getMaster().connect();  
                dstRG = db.getReplicaGroup(dstRgName).getMaster().connect();              
                DBCollection srcCL = srcRG.getCollectionSpace(SdbTestBase.csName)
                        .getCollection(CL_NAME);
                DBCollection dstCL = dstRG.getCollectionSpace(SdbTestBase.csName)
                        .getCollection(CL_NAME);
                
                long expCnt = INSERT_RECS_NUM + INSERT_RECS_NUM / 2;
                long srcDataCnt = srcCL.getCount();
                long dstDataCnt = dstCL.getCount();
                Assert.assertEquals(srcDataCnt + dstDataCnt, expCnt);  
            } finally {
                if (srcRG != null) srcRG.close();
                if (dstRG != null) dstRG.close();
            }
        }
        
        @ExecuteOrder(step = 4, desc = "关闭连接")
        private void closeDB() {
            if (db != null) db.close();
        }
    }
    
    private void insertRecords(DBCollection cl) {
        int num = -1;
        for (int k = 0; k < INSERT_RECS_NUM; k += INSERT_BATCH_RECS_NUM) {
            ArrayList<BSONObject> insertor = new ArrayList<>();
            for (int i = 0 + k; i < INSERT_BATCH_RECS_NUM + k; i++) {
                BSONObject bsonObj = new BasicBSONObject();
                bsonObj.put("recordId", num);
                bsonObj.put("a", StringUtils.getRandomString( 16 ));
                bsonObj.put("b", StringUtils.getRandomString( 32 ));
                insertor.add(bsonObj);
                num--;
            }
            cl.insert(insertor);
        }
    }
    
    private ObjectId createLob(DBCollection cl) {
        DBLob lob = null;
        ObjectId id = null;
        try {    
            String lobStringBuff = StringUtils.getRandomString(1024);
            lob = cl.createLob();
            lob.write(lobStringBuff.getBytes());
            id = lob.getID(); 
        } finally{
            if (lob != null){
                lob.close();              
            } 
        }
        return id;
    }
}
