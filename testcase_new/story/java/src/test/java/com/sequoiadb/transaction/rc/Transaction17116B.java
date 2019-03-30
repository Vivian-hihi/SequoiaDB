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
public class Transaction17116B extends SdbTestBase {
    private String clName = "cl_17116B";
    private Sequoiadb sdb = null;
    private Sequoiadb db2 = null;
    private DBCollection cl = null;
    private DBCollection cl2 = null;
    private DBCursor cursor = null;
    private List<BSONObject> invInsertR1s = new ArrayList<BSONObject>();
    private List<BSONObject> invExpList = new ArrayList<BSONObject>();
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
        List<BSONObject> posInsertR1s = new ArrayList<BSONObject>();
        for(int i=0; i<15000; i++){
            posInsertR1s.add((BSONObject)JSON.parse("{_id:"+ i +", a:"+ i +", b:"+ i +"}"));
            BSONObject updateR1 = (BSONObject) JSON.parse("{_id:"+ (14999-i) +", b:"+ (14999-i) +"}");
            invExpList.add(updateR1);
            invInsertR1s.add((BSONObject) JSON.parse("{_id:"+ (14999-i) +", a:"+ (14999-i) +", b:"+ (14999-i) +"}"));
        }
        Collections.shuffle(posInsertR1s);
        cl.insert(posInsertR1s);
        
        db2.beginTransaction();

        ReadThread readThread = new ReadThread();
        readThread.start();
        
        UpdateThread updateThread = new UpdateThread(readThread);
        updateThread.start();
        
        if (!updateThread.isSuccess()) {
            Assert.fail(updateThread.getErrorMsg());
        }

        // 非事务表扫描记录、逆序
        cursor = cl.query(null, null, "{b:-1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, invExpList);
        actList.clear();

        // 非事务索引扫描记录、逆序
        cursor = cl.query(null, null, "{b:-1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, invExpList);
        actList.clear();

        db2.commit();

        // 非事务表扫描记录、逆序
        cursor = cl.query(null, null, "{b:-1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, invExpList);
        actList.clear();

        // 非事务索引扫描记录、逆序
        cursor = cl.query(null, null, "{b:-1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, invExpList);
        actList.clear();

        cursor.close();
    }

    private class UpdateThread extends SdbThreadBase{
        private Sequoiadb db1 = null;
        private DBCollection cl1 = null;
        private DBCursor cursor = null;
        private ReadThread readThread = null;
        private List<BSONObject> actList = new ArrayList<BSONObject>();
        
        public UpdateThread(ReadThread readThread) {
            // TODO Auto-generated constructor stub
            this.readThread = readThread;
        }

        @Override
        public void exec() throws Exception {
            // TODO Auto-generated method stub
            try{
                db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                cl1 = db1.getCollectionSpace(csName).getCollection(clName);
                
                db1.beginTransaction();
                
                for(int i=0; i<15000; i++){
                    cl1.update("{a:"+ i +"}", "{$unset:{a:"+ i +"}}", "{'':'a'}");
                }
                
                // 事务1表扫描记录、逆序
                cursor = cl1.query(null, null, "{b:-1}", "{'':null}");
                actList = TransUtils.getReadActList(cursor);
                Assert.assertEquals(actList, invExpList);
                actList.clear();
                
                // 事务1索引扫描记录、逆序
                cursor = cl1.query(null, null, "{_id:-1}", "{'':'_id'}");
                actList = TransUtils.getReadActList(cursor);
                Assert.assertEquals(actList, invExpList);
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
        private DBCursor cursor = null; 
        
        @Override
        public void exec() throws Exception {
            // TODO Auto-generated method stub
            // 事务2表扫描记录、逆序
            cursor = cl2.query(null, null, "{a:-1}", "{'':null}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, invInsertR1s);
            actList.clear();
            
            // 事务2表扫描记录、逆序
            cursor = cl2.query(null, null, "{b:-1}", "{'':null}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, invInsertR1s);
            actList.clear();

            // 事务2走索引扫描记录、逆序  
            cursor = cl2.query(null, null, "{b:-1}", "{'':'a'}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, invInsertR1s);
            actList.clear();
            
            // 事务2走索引扫描记录、逆序 
            cursor = cl2.query(null, null, "{a:-1}", "{'':'a'}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, invInsertR1s);
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
