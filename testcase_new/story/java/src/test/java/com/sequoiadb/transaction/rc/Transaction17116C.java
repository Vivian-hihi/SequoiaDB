package com.sequoiadb.transaction.rc;

/**
 * @Description seqDB-17116:  更新复合索引，同时与读并发 
 * @author xiaoni Zhao
 * @date 2019-1-21
 */
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

@Test(groups = "rc")
public class Transaction17116C extends SdbTestBase {
    private String clName = "cl_17116C";
    private Sequoiadb sdb = null;
    private Sequoiadb db2 = null;
    private DBCollection cl = null;
    private DBCollection cl2 = null;
    private DBCursor cursor = null;
    private List<BSONObject> posInsertR1s = new ArrayList<BSONObject>();
    private List<BSONObject> posExpList = new ArrayList<BSONObject>();
    private List<BSONObject> actList = new ArrayList<BSONObject>();

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl2 = db2.getCollectionSpace(csName).getCollection(clName);
        cl.createIndex("a", "{a:-1, b:1}", false, false);
    }

    @Test
    public void test() {
        List<BSONObject> insertR1s = new ArrayList<BSONObject>();
        for(int i=0; i<15000; i++){
            insertR1s.add((BSONObject)JSON.parse("{_id:"+ i +", b:"+ i +"}"));
            posExpList.add((BSONObject)JSON.parse("{_id:"+ i +",a:"+ i +", b:"+ i +"}"));
        }
        posInsertR1s.addAll(insertR1s);
        Collections.shuffle(insertR1s);
        cl.insert(insertR1s);
        
        db2.beginTransaction();
        
        ReadThread readThread = new ReadThread(posInsertR1s);
        readThread.start();
        
        UpdateThread updateThread = new UpdateThread(readThread,posExpList);
        updateThread.start();
        
        if (!updateThread.isSuccess()) {
            Assert.fail(updateThread.getErrorMsg());
        }
        
        // 非事务表扫描记录、正序
        cursor = cl.query(null, null, "{a:1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, posExpList);
        actList.clear();

        // 非事务索引扫描记录、正序
        cursor = cl.query(null, null, "{a:1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, posExpList);
        actList.clear();

        db2.commit();

        // 非事务表扫描记录、正序
        cursor = cl.query(null, null, "{b:1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, posExpList);
        actList.clear();

        // 非事务索引扫描记录、正序
        cursor = cl.query(null, null, "{a:1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, posExpList);
        actList.clear();

        cursor.close();
    }
    
    private class UpdateThread extends SdbThreadBase{
        private Sequoiadb db1 = null;
        private DBCollection cl1 = null;
        private DBCursor cursor = null;
        private ReadThread readThread = null;
        private List<BSONObject> actList = new ArrayList<BSONObject>();
        private List<BSONObject> posExpList = new ArrayList<BSONObject>();
        
        public UpdateThread(ReadThread readThread, List<BSONObject> posExpList) {
            // TODO Auto-generated constructor stub
            this.readThread = readThread;
            this.posExpList = posExpList;
        }

        @Override
        public void exec() throws Exception {
            // TODO Auto-generated method stub
            try{
                db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                cl1 = db1.getCollectionSpace(csName).getCollection(clName);
                
                db1.beginTransaction();
                
                for(int i=0; i<15000; i++){
                    cl1.update("{_id:"+ i +"}", "{$set:{a:"+ i +"}}",null);
                }
                
                // 事务1表扫描记录、正序
                cursor = cl1.query(null, null, "{a:1}", "{'':null}");
                actList = TransUtils.getReadActList(cursor);
                Assert.assertEquals(actList, posExpList);
                actList.clear();
                
                // 事务1表扫描记录、正序
                cursor = cl1.query(null, null, "{b:1}", "{'':null}");
                actList = TransUtils.getReadActList(cursor);
                Assert.assertEquals(actList, posExpList);
                actList.clear();

                // 事务1索引扫描记录、正序
                cursor = cl1.query(null, null, "{a:1}", "{'':'a'}");
                actList = TransUtils.getReadActList(cursor);
                Assert.assertEquals(actList, posExpList);
                actList.clear();
                
                // 事务1索引扫描记录、正序
                cursor = cl1.query(null, null, "{a:1}", "{'':'a'}");
                actList = TransUtils.getReadActList(cursor);
                Assert.assertEquals(actList, posExpList);
                actList.clear();
                
                if (!readThread.isSuccess()) {
                    Assert.fail(readThread.getErrorMsg());
                }
                db1.commit();
            }catch(BaseException e){
                e.printStackTrace();
            }finally {
                cursor.close();
                db1.close();
            }
        }   
    }
    
    private class ReadThread extends SdbThreadBase{
        private List<BSONObject> actList = new ArrayList<BSONObject>();
        private List<BSONObject> posInsertR1s = new ArrayList<BSONObject>();
        private DBCursor cursor = null; 
        
        public ReadThread(List<BSONObject> posInsertR1s) {
            // TODO Auto-generated constructor stub
            this.posInsertR1s = posInsertR1s;
        }
        
        @Override
        public void exec() throws Exception {
            // TODO Auto-generated method stub
            // 事务2表扫描记录、正序
            cursor = cl2.query(null, null, "{b:1}", "{'':null}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, posInsertR1s);
            actList.clear();
            
            // 事务2走索引扫描记录、正序  
            cursor = cl2.query(null, null, "{b:1}", "{'':'a'}");
            actList = TransUtils.getReadActList(cursor); 
            Assert.assertEquals(actList, posInsertR1s);
            actList.clear();
            
            cursor.close();
        }
    }

    @AfterClass
    public void tearDown() {
        if (!db2.isClosed()) {
            db2.close();
        }
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        if (cs.isCollectionExist(clName)) {
            cs.dropCollection(clName);
        }
        if (!sdb.isClosed()) {
            sdb.close();
        }
    }
}
