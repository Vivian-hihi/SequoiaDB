package com.sequoiadb.transaction.rc;

/**
 * @Description seqDB-17080:  多个原子操作与读并发，事务提交  
 * @author Zhao Xiaoni
 * @date 2019-1-21
 */
import java.util.ArrayList;
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
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;
import com.sequoiadb.transaction.TransUtils;

@Test(groups = "rc")
public class Transaction17080 extends SdbTestBase {
    private String clName = "cl_17080";
    private Sequoiadb sdb = null;
    private Sequoiadb db1 = null;
    private Sequoiadb db2 = null;
    private Sequoiadb dbT = null;
    private Sequoiadb dbI = null;
    private DBCollection cl = null;
    private DBCollection cl1 = null;
    private DBCollection cl2 = null;
    private DBCursor cursor = null;
    private List<BSONObject> insertR1s = new ArrayList<BSONObject>();
    private List<BSONObject> expList = new ArrayList<BSONObject>();
    private List<BSONObject> actList = new ArrayList<BSONObject>();

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl = sdb.getCollectionSpace(csName).createCollection(clName);
        db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl1 = db1.getCollectionSpace(csName).getCollection(clName);
        db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        cl2 = db1.getCollectionSpace(csName).getCollection(clName);
        cl.createIndex("a", "{a:1}", false, false);
    }

    @Test
    public void test() {
        insertR1s = TransUtils.insertDatas(cl, 0, 10000, 0);
        
        db1.beginTransaction();
        db2.beginTransaction();

        //事务1执行多个原子操作
        Operation operation = new Operation();
        operation.start();
        
        //事务2并发表扫描
        dbT = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        dbT.beginTransaction();
        Read read1 = new Read(dbT, "{'':null}");
        read1.start();
        
        //事务2并发索引扫描
        dbI = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        dbI.beginTransaction();
        Read read2 = new Read(dbI, "{'':'a'}");
        read2.start();
        
        if (!read1.isSuccess() || !read2.isSuccess() || !operation.isSuccess()){
        	Assert.fail(read1.getErrorMsg() + read2.getErrorMsg() + operation.getErrorMsg());
        }
        
        //非事务表扫描
        cursor = cl.query(null, null, "{_id:1}", "{'':null}");
        Assert.assertEquals(TransUtils.getReadActList(cursor), expList);
        
        //非事务索引扫描
        cursor = cl.query(null, null, "{a:1}", "{'':'a'}");
        Assert.assertEquals(TransUtils.getReadActList(cursor), expList);
        
        db1.commit();

        // 事务2表扫描记录
        cursor = cl2.query(null, null, "{_id:1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);
        actList.clear();

        // 事务2索引扫描记录
        cursor = cl2.query(null, null, "{_id:1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);
        actList.clear();

        // 非事务表扫描记录
        cursor = cl.query(null, null, "{_id:1}", "{'':null}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);
        actList.clear();

        // 非事务索引扫描记录
        cursor = cl.query(null, null, "{a:1}", "{'':'a'}");
        actList = TransUtils.getReadActList(cursor);
        Assert.assertEquals(actList, expList);

        db2.commit();
        dbI.commit();
        dbT.commit();
        cursor.close();
    }

    private class Operation extends SdbThreadBase{
		@Override
		public void exec() throws Exception {
			// TODO Auto-generated method stub
		    BSONObject insertR = (BSONObject) JSON.parse("{_id:20000,a:20000,b:20000}");
	        for(int i=0; i<10000; i++)
	        {
	            cl1.insert(insertR);
	            cl1.update("{a:20000}", "{$set:{a:20001}}", "{'':'a'}");
	            cl1.delete("{a:20001}", "{'':'a'}");
	            
	            cl1.delete("{_id:"+ i +"}");
	            cl1.insert((BSONObject) JSON.parse("{_id:"+ i +", a:"+ i +",b:"+ i +"}"));
	            cl1.update("{a:"+ i +"}","{$set:{a:"+ (i+10000) +"}}","{'':'a'}");
	            expList.add((BSONObject) JSON.parse("{_id:"+ i +", a:"+ (i+10000) +",b:"+ i +"}"));
	            System.out.println("operation"+i);
	        }
		}
    }
    
    private class Read extends SdbThreadBase{
    	private Sequoiadb db2 = null;
    	private DBCollection cl2 = null;
    	private String hint = null;
		private DBCursor cursor = null;
        private List<BSONObject> actList = new ArrayList<BSONObject>();
    	
    	public Read(Sequoiadb db2, String hint) {
			// TODO Auto-generated constructor stub
    		this.db2 = db2;
    		this.hint = hint;
		}
    	
		@Override
		public void exec() throws Exception {
			// TODO Auto-generated method stub
	        cl2 = db2.getCollectionSpace(csName).getCollection(clName);
			// 事务2扫描记录i
			for(int i=0; i<50; i++){
 		        cursor = cl2.query(null, null, "{_id:1}", hint);
		        actList = TransUtils.getReadActList(cursor);
		        Assert.assertEquals(actList, insertR1s);
			}
			cursor.close();
		}
    }

    @AfterClass
    public void tearDown() {
        if (!db1.isClosed()) {
            db1.close();
        }
        if (!db2.isClosed()) {
            db2.close();
        }
        if (!dbT.isClosed()) {
            db1.close();
        }
        if (!dbI.isClosed()) {
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
