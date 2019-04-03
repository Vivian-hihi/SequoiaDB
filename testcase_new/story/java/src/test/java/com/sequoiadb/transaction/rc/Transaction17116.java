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
import org.testng.annotations.DataProvider;
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
public class Transaction17116 extends SdbTestBase {
    private String clName = "cl_17116";
    private Sequoiadb sdb = null;
    private Sequoiadb db1 = null;
    private Sequoiadb db2 = null;
    private DBCollection cl = null;
    private DBCollection cl2 = null;
    private DBCursor cursor = null;
    private List<BSONObject> actList = new ArrayList<BSONObject>();

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        cl2 = db2.getCollectionSpace(csName).getCollection(clName);
    }
    
    @DataProvider(name = "transaction17116")
    public Object[][] createIndex(){
        List<BSONObject> insertR1s = new ArrayList<BSONObject>();
        List<BSONObject> insertR2s = new ArrayList<BSONObject>();
        List<BSONObject> insertR3s = new ArrayList<BSONObject>();
        List<BSONObject> insertR4s = new ArrayList<BSONObject>();

        //indexKey为"{a:1, b:1}"时，读老记录的预期结果
        for(int i = 0; i < 12000; i++){
            if(i < 3000 ){
                insertR1s.add((BSONObject)JSON.parse("{_id:"+ i +", a:0, b:"+ i +"}"));
            }else if(3000 <= i && i < 6000){
                insertR2s.add((BSONObject)JSON.parse("{_id:"+ i +", a:1, b:"+ i +"}"));
            }else if(6000 <= i && i < 9000){
                insertR3s.add((BSONObject)JSON.parse("{_id:"+ i +", a:2, b:"+ i +"}"));
            }else if(9000 <= i && i < 12000){
                insertR4s.add((BSONObject)JSON.parse("{_id:"+ i +", b:"+ i +"}"));
            }  
        }
        List<BSONObject> oldExp1 = new ArrayList<BSONObject>();
        oldExp1.addAll(insertR4s);
        oldExp1.addAll(insertR1s);
        oldExp1.addAll(insertR2s);
        oldExp1.addAll(insertR3s);
        
        //indexKey为"{a:-1, b:1}"时，读老记录的预期结果
        List<BSONObject> oldExp2 = new ArrayList<BSONObject>();
        oldExp2.addAll(insertR3s);
        oldExp2.addAll(insertR2s);
        oldExp2.addAll(insertR1s);
        oldExp2.addAll(insertR4s);

        //indexKey为"{a:-1, b:-1}"时，读老记录的预期结果
        insertR1s.clear();
        insertR2s.clear();
        insertR3s.clear();
        insertR4s.clear();
        for(int i = 11999; i >= 0; i--){
            if(i < 3000 ){
                insertR1s.add((BSONObject)JSON.parse("{_id:"+ i +", a:0, b:"+ i +"}"));
            }else if(3000 <= i && i < 6000){
                insertR2s.add((BSONObject)JSON.parse("{_id:"+ i +", a:1, b:"+ i +"}"));
            }else if(6000 <= i && i < 9000){
                insertR3s.add((BSONObject)JSON.parse("{_id:"+ i +", a:2, b:"+ i +"}"));
            }else if(9000 <= i && i < 12000){
                insertR4s.add((BSONObject)JSON.parse("{_id:"+ i +", b:"+ i +"}"));
            }  
        }
        List<BSONObject> oldExp3 = new ArrayList<BSONObject>();
        oldExp3.addAll(insertR3s);
        oldExp3.addAll(insertR2s);
        oldExp3.addAll(insertR1s);
        oldExp3.addAll(insertR4s);
        
        //indexKey为"{a:1, b:-1}"时，读老记录的预期结果
        List<BSONObject> oldExp4 = new ArrayList<BSONObject>();
        oldExp4.addAll(insertR4s);
        oldExp4.addAll(insertR1s);
        oldExp4.addAll(insertR2s);
        oldExp4.addAll(insertR3s);
        
        List<BSONObject> updateR1s = new ArrayList<BSONObject>();
        List<BSONObject> updateR2s = new ArrayList<BSONObject>();
        List<BSONObject> updateR3s = new ArrayList<BSONObject>();
        List<BSONObject> updateR4s = new ArrayList<BSONObject>();
        
        //indexKey为"{a:1, b:1}"时，读更新后记录的预期结果
        for(int i = 0; i < 12000; i++){
            if(i < 3000 ){
                updateR1s.add((BSONObject)JSON.parse("{_id:"+ i +", a:6, b:"+ (i-1) +"}"));
            }else if(3000 <= i && i < 6000){
                updateR2s.add((BSONObject)JSON.parse("{_id:"+ i +", b:"+ i +"}"));
            }else if(6000 <= i && i < 9000){
                updateR3s.add((BSONObject)JSON.parse("{_id:"+ i +", a:4, b:"+ i +"}"));
            }else if(9000 <= i && i < 12000){
                updateR4s.add((BSONObject)JSON.parse("{_id:"+ i +", a:5, b:"+ i +"}"));
            }   
        }
        List<BSONObject> newExp1 = new ArrayList<BSONObject>();
        newExp1.addAll(updateR2s);
        newExp1.addAll(updateR3s);
        newExp1.addAll(updateR4s);
        newExp1.addAll(updateR1s);
        
        //indexKey为"{a:-1, b:1}"时，读更新后记录的预期结果
        List<BSONObject> newExp2 = new ArrayList<BSONObject>();
        newExp2.addAll(updateR1s);
        newExp2.addAll(updateR4s);
        newExp2.addAll(updateR3s);
        newExp2.addAll(updateR2s);

        //indexKey为"{a:-1, b:-1}"时，读更新后记录的预期结果
        updateR1s.clear();
        updateR2s.clear();
        updateR3s.clear();
        updateR4s.clear();
        for(int i = 11999; 0 <= i; i--){
            if(i < 3000 ){
                updateR1s.add((BSONObject)JSON.parse("{_id:"+ i +", a:6, b:"+ (i-1) +"}"));
            }else if(3000 <= i && i < 6000){
                updateR2s.add((BSONObject)JSON.parse("{_id:"+ i +", b:"+ i +"}"));
            }else if(6000 <= i && i < 9000){
                updateR3s.add((BSONObject)JSON.parse("{_id:"+ i +", a:4, b:"+ i +"}"));
            }else if(9000 <= i && i < 12000){
                updateR4s.add((BSONObject)JSON.parse("{_id:"+ i +", a:5, b:"+ i +"}"));
            }   
        }
        List<BSONObject> newExp3 = new ArrayList<BSONObject>();
        newExp3.addAll(updateR1s);
        newExp3.addAll(updateR4s);
        newExp3.addAll(updateR3s);
        newExp3.addAll(updateR2s);
        
        //indexKey为"{a:1, b:-1}"时，读更新后记录的预期结果
        List<BSONObject> newExp4 = new ArrayList<BSONObject>();
        newExp4.addAll(updateR2s);
        newExp4.addAll(updateR3s);
        newExp4.addAll(updateR4s);
        newExp4.addAll(updateR1s);
        
        return new Object[][]{
            {"{a:1, b:1}",
             newExp1, oldExp1},
            {"{a:-1, b:1}",
             newExp2, oldExp2},
            {"{a:-1, b:-1}",
             newExp3, oldExp3},
            {"{a:1, b:-1}",
             newExp4, oldExp4}
        };
    }

    @Test(dataProvider="transaction17116")
    public void test(String indexKey, List<BSONObject> newExp, List<BSONObject> oldExp) {
        List<BSONObject> insertRs = new ArrayList<BSONObject>();
        for(int i = 0; i < 12000; i++){
            int a = i / 3000 ;
            if ( a < 3 ){
                insertRs.add((BSONObject)JSON.parse("{_id:"+ i +", a:" + a + ", b:"+ i +"}"));
            }else{
                insertRs.add((BSONObject)JSON.parse("{_id:"+ i +", b:"+ i +"}")); 
            }
        }
        try{
            Collections.shuffle(insertRs);
            cl.createIndex("a", indexKey, false, false);
            cl.insert(insertRs);

            db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            db1.beginTransaction();
            db2.beginTransaction();
            
            ReadThread readThread = new ReadThread(indexKey, oldExp);
            readThread.start();

            UpdateThread updateThread = new UpdateThread(indexKey, newExp);
            updateThread.start();
            
            if (!readThread.isSuccess() || !updateThread.isSuccess()) {
                Assert.fail(readThread.getErrorMsg()+updateThread.getErrorMsg());
            }else{
                db1.commit();
            }

            // 非事务表扫描记录
            cursor = cl.query(null, null, indexKey, "{'':null}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, newExp);
            actList.clear();

            // 非事务索引扫描记录
            cursor = cl.query(null, null, indexKey, "{'':'a'}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, newExp);
            actList.clear();

            db2.commit();

            // 非事务表扫描记录
            cursor = cl.query(null, null, indexKey, "{'':null}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, newExp);
            actList.clear();

            // 非事务索引扫描记录
            cursor = cl.query(null, null, indexKey, "{'':'a'}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, newExp);
            actList.clear();  
            cursor.close(); 
        }catch(BaseException e){
            e.printStackTrace();
        }finally {
            if(cl.isIndexExist("a")){
                cl.dropIndex("a"); 
            }
            cl.truncate();
        }
    }
    
    private class UpdateThread extends SdbThreadBase{
        private DBCollection cl1 = null;
        private DBCursor cursor = null;
        private String indexKey = null;
        private List<BSONObject> newExp = null;
        private List<BSONObject> actList = new ArrayList<BSONObject>();
        
        public UpdateThread(String indexKey, List<BSONObject> newExp) {
            // TODO Auto-generated constructor stub
            this.indexKey = indexKey;
            this.newExp = newExp; 
        }

        @Override
        public void exec() throws Exception {
            // TODO Auto-generated method stub
            try{
                cl1 = db1.getCollectionSpace(csName).getCollection(clName);
                
                cl1.update("{a:0}", "{$inc:{a:6, b:-1}}", "{'':'a'}");
                cl1.update("{a:1}", "{$unset:{a:1}}", "{'':'a'}");
                cl1.update("{a:2}", "{$set:{a:4}}", "{'':'a'}");
                cl1.update("{a:{$isnull:1}, b:{$gte: 9000, $lt: 12000}}", "{$set:{a:5}}", null);
                
                // 事务1表扫描记录
                cursor = cl1.query(null, null, indexKey, "{'':null}");
                actList = TransUtils.getReadActList(cursor);
                Assert.assertEquals(actList, newExp);
                actList.clear();
                
                // 事务1索引扫描记录
                cursor = cl1.query(null, null, indexKey, "{'':'a'}");
                actList = TransUtils.getReadActList(cursor);
                Assert.assertEquals(actList, newExp);
                actList.clear();
                cursor.close();
            }catch(BaseException e){
                e.printStackTrace();
            }
        }   
    }
    
    private class ReadThread extends SdbThreadBase{
        private List<BSONObject> actList = new ArrayList<BSONObject>();
        private DBCursor cursor = null;
        private String indexKey = null;
        private List<BSONObject> oldExp = null;
        
        public ReadThread(String indexKey, List<BSONObject> oldExp) {
            // TODO Auto-generated constructor stub
            this.indexKey = indexKey;
            this.oldExp = oldExp;
        }
        
        @Override
        public void exec() throws Exception {
            // TODO Auto-generated method stub
            // 事务2表扫描记录
            cursor = cl2.query(null, null, indexKey, "{'':null}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, oldExp);
            actList.clear();

            // 事务2走索引扫描记录
            cursor = cl2.query(null, null, indexKey, "{'':'a'}");
            actList = TransUtils.getReadActList(cursor);
            Assert.assertEquals(actList, oldExp);
            actList.clear();
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
