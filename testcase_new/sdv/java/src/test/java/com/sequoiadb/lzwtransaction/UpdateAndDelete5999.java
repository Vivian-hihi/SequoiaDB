package com.sequoiadb.lzwtransaction;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * test content: 多个事务并发，同时更新/删除cl中不同记录并提交事务_SD.transaction.010
 * testlink-case: seqDB-5999
 * @author wangkexin
 * @Date 2019.03.15
 * @version 1.00
 */
public class UpdateAndDelete5999 extends SdbConfTestBase{
	private String clName = "cl5999";
	private Sequoiadb sdb = null;
	private DBCollection cl = null;
	private DBCollection cl1 = null;
	private DBCollection cl2 = null;
	private BSONObject del_matcher = new BasicBSONObject();
	
	@Override
    protected void setNodeConf(){
        dataConf.put("transactionon", true);
        stdalnConf.put("transactionon", true);
    }
	
	@BeforeClass
    public void setup() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		LzwTransUtils util = new LzwTransUtils();
		cl = sdb.getCollectionSpace(SdbTestBase.csName).createCollection(clName);
		
		//insertData(DBCollection cl,int start, int recSum, int strLength)
		util.insertData(cl, 0, 100, 10);
		del_matcher.put("_id", 10);
		long count = cl.getCount(del_matcher);
		Assert.assertEquals(count, 1, "Matching data does not exist");
	}
	
	@Test
	private void test() {
		Sequoiadb db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		Sequoiadb db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		
		db1.beginTransaction();
		db2.beginTransaction();
		
		cl1 = db1.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
        cl2 = db2.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
        
        UpdateThread updateThread = new UpdateThread();
        DeleteThread deleteThread = new DeleteThread();
        updateThread.start();
        deleteThread.start();
        
        db1.commit();
        db2.commit();
        
        Assert.assertTrue(updateThread.isSuccess(), updateThread.getErrorMsg());
        Assert.assertTrue(deleteThread.isSuccess(), deleteThread.getErrorMsg());
		
        db1.close();
        db2.close();
        checkResult();
	}
	
	@AfterClass
	public void teardown() {
		sdb.getCollectionSpace(csName).dropCollection(clName);
		sdb.close();
	}
	
	private class UpdateThread extends SdbThreadBase {
        @Override
        public void exec() throws BaseException {
        	BSONObject matcher = new BasicBSONObject();
        	BSONObject modifyObj = new BasicBSONObject();
        	BSONObject modifier = new BasicBSONObject();
        	matcher.put("age", 50);
        	modifyObj.put("age", 5999);
        	modifier.put("$set", modifyObj);
            cl1.update(matcher, modifier, null);
        }
    }
	
	private class DeleteThread extends SdbThreadBase {
        @Override
        public void exec() throws BaseException {
            cl2.delete(del_matcher);
        }
    }
	
	private void checkResult(){
		long actCount = cl.getCount(del_matcher);
		Assert.assertEquals(actCount, 0, "Deleted data still exists!");
		
		BSONObject matcher = new BasicBSONObject();
		matcher.put("age", 5999);
		actCount = cl.getCount(matcher);
		Assert.assertEquals(actCount, 1, "Update data does not exist!");
	}
}
