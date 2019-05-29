package com.sequoiadb.autoincrement.killnode;
<<<<<<< .mine
/**
 * @FileName:seqDB-15976： CacheSize及AcquireSize均设置为1，不指定自增字段插入时coord主节点异常重启 
 * 预置条件：集合中已存在自增字段且CacheSize及AcquireSize均设置为1
 * 测试步骤：1.不指定自增字段插入记录，同时coord异常 2.待节点正常后，不指定自增字段继续插入记录
 * 预期结果：1.coord异常后，插入失败，错误信息正确 2.记录插入成功，自增字段值从起始到结束值唯一且递增，值正确，主备节点数据一致
 * @Author zhaoxiaoni
 * @Date 2018-11-19
 * @Version 1.00
 */
=======

>>>>>>> .r41294
import java.util.ArrayList;
import java.util.List;
import org.bson.BSONObject;
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
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @FileName:seqDB-15976： CacheSize及AcquireSize均设置为1，不指定自增字段插入时coord主节点异常重启
 *                        预置条件：集合中已存在自增字段且CacheSize及AcquireSize均设置为1
 *                        测试步骤：1.不指定自增字段插入记录，同时coord异常 2.待节点正常后，不指定自增字段继续插入记录
 *                        预期结果：1.coord异常后，插入失败，错误信息正确
 *                        2.记录插入成功，自增字段值从起始到结束值唯一且递增，值正确，主备节点数据一致
 * @Author zhaoxiaoni
 * @Date 2018-11-19
 * @Version 1.00
 */

public class Insert15976 extends SdbTestBase {
    private Sequoiadb sdb = null;
<<<<<<< .mine
	private GroupMgr groupMgr = null;
	private String clName = "cl_15976";
	private int cacheSize = 1;
=======
    private GroupMgr groupMgr = null;
    private DBCollection cl = null;
    private String clName = "cl_15976";
    private int cacheSize = 1;
>>>>>>> .r41294
    private int acquireSize = 1;
    private List<Long> records = new ArrayList<Long>();
<<<<<<< .mine
	
	@BeforeClass
	public void setUP(){
        	try {
                sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                sdb.getCollectionSpace(csName).createCollection(clName,(BSONObject) JSON.parse
                        ("{AutoIncrement:{Field:'id',CacheSize:" + cacheSize + ",AcquireSize:"+acquireSize+"}}"));
                groupMgr = GroupMgr.getInstance();
                if (!groupMgr.checkBusiness()) {
                    throw new SkipException("checkBusiness failed");
                }
            } catch (ReliabilityException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
	}
	
	@SuppressWarnings("resource")
    @Test
	public void test(){
		Sequoiadb db = null; 
		DBCollection cl = null;
		try {
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			FaultMakeTask faultTask = KillNode.getFaultMakeTask(db.getHost(), String.valueOf(db.getPort()), 1);
	        TaskMgr mgr = new TaskMgr();
	        InsertTask insertTask = new InsertTask();
	        mgr.addTask(insertTask);
	        mgr.addTask(faultTask);
	        mgr.execute();
	        
	        Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());
	        if (!groupMgr.checkBusinessWithLSN(600)) {
	        	Assert.fail("checkBusinessWithLSN() occurs timeout"); 
	        }
	        
	        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
	        insertData(cl, 100);
            
			//获取实际记录并按照id排序（由于驱动生成_id,coord生成自增字段值，因此无法通过_id排序来校验自增字段），则自增字段会从当前值严格递增
			DBCursor cursor = cl.query("", "", "{id:1}", "");
			while(cursor.hasNext()){
				BSONObject record = cursor.getNext();
				records.add((long) record.get("id"));
			}
			for(int i = 0; i < records.size()-1; i++)
			{
				if(records.get(i) >= records.get(i+1))
				{
					Assert.fail("records error!"); 
				}
			}
		} catch (ReliabilityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}finally {
			if (db != null){
				db.close();
			}
=======

    @BeforeClass
    public void setUP() {
        try (Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")) {
            groupMgr = GroupMgr.getInstance();
            if (!groupMgr.checkBusiness()) {
                throw new SkipException("checkBusiness failed");
            }
            cl = db.getCollectionSpace(csName).createCollection(clName, (BSONObject) JSON
                    .parse("{AutoIncrement:{Field:'id',CacheSize:" + cacheSize + ",AcquireSize:" + acquireSize + "}}"));
        } catch (ReliabilityException e) {
            Assert.fail(this.getClass().getName() + " setUp error, error description:" + e.getMessage() + "\r\n"
                    + Utils.getKeyStack(e, this));
>>>>>>> .r41294
        }
    }

    @Test
    public void test() {
        Sequoiadb db = null;
        try {
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            FaultMakeTask faultTask = KillNode.getFaultMakeTask(db.getHost(), String.valueOf(db.getPort()), 1);
            TaskMgr mgr = new TaskMgr();
            InsertTask insertTask = new InsertTask();
            mgr.addTask(insertTask);
            mgr.addTask(faultTask);
            mgr.execute();

            // faultTask.start();
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());
            if (!groupMgr.checkBusinessWithLSN(600)) {
                Assert.fail("checkBusinessWithLSN() occurs timeout");
            }

            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            insertData(cl, 100);

            // 获取实际记录并按照id排序（由于驱动生成_id,coord生成自增字段值，因此无法通过_id排序来校验自增字段），则自增字段会从当前值严格递增
            DBCursor cursor = cl.query("", "", "{id:1}", "");
            while (cursor.hasNext()) {
                BSONObject record = cursor.getNext();
                records.add((long) record.get("id"));
            }
            for (int i = 0; i < records.size() - 1; i++) {
                if (records.get(i) >= records.get(i + 1)) {
                    Assert.fail("records error!");
                }
            }
        } catch (ReliabilityException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            Assert.fail(e.getMessage());
        } finally {
            if (db != null) {
                db.close();
            }
        }
    }

    private class InsertTask extends OperateTask {
        @Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            try{
                db = new Sequoiadb(coordUrl, "", "");
                CollectionSpace cs = db.getCollectionSpace(csName);
                DBCollection cl = cs.getCollection(clName);
<<<<<<< .mine
                for(int i=0; i< 10000; i++){
        			BSONObject obj = (BSONObject) JSON.parse("{a:" + i + "}");
        			cl.insert(obj);
        		}
            }catch (BaseException e){
                e.printStackTrace();
=======
                for (int i = 0; i < 10000; i++) {
                    BSONObject obj = (BSONObject) JSON.parse("{a:" + i + "}");
                    cl.insert(obj);
                }
            } catch (BaseException e) {
                e.printStackTrace();
>>>>>>> .r41294
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }

    public void insertData(DBCollection cl, int insertNum) {
        ArrayList<BSONObject> arrList = new ArrayList<BSONObject>();
        for (int i = 0; i < insertNum; i++) {
            BSONObject obj = (BSONObject) JSON.parse("{mustCheckAutoIncrement:" + i + "}");
            arrList.add(obj);
        }
        cl.insert(arrList);
    }

    @AfterClass
    public void tearDown() {
<<<<<<< .mine
	    @SuppressWarnings("resource")
        Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
	    CollectionSpace cs = db.getCollectionSpace(csName);
        if(cs.isCollectionExist(clName)){
            cs.dropCollection(clName);
        }
        if (!sdb.isClosed()) {
            sdb.close();
        }
	}
=======
        Sequoiadb sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb.getCollectionSpace(csName).dropCollection(clName);
        sdb.close();
    }
>>>>>>> .r41294
}
