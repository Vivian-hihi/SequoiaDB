package com.sequoiadb.fulltext.killnode;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.fulltext.FullTextESUtils;
import com.sequoiadb.fulltext.FullTextUtils;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;
/**
 * @Description seqDB-12079: 集合中存在全文索引，修改普通集合的副本数 
 * @author xiaoni Zhao
 * @date 2018/12/3
 */
public class AlterReplSize12079 extends SdbTestBase {
	private String clName = "ES_cl_12079";
    private CollectionSpace cs;
    private DBCollection cl;
    private GroupMgr groupMgr = null;
    private Sequoiadb sdb = null;
    private String groupName;
    private Client esClient = null;
    private String fullIndexName = "fullIndex12079";

    @BeforeClass()
    public void setUp() {
        try {
            System.out.println(
                    "the TestCase Name:" + getClass().getName() + ". the TestCase begin at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = new GroupMgr();

            // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
            if (!groupMgr.checkBusiness(20)) {
                throw new SkipException("checkBusiness return false");
            }
            groupName = groupMgr.getAllDataGroupName().get(0);

            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cs = sdb.getCollectionSpace(csName);
            cl = cs.createCollection(clName, (BSONObject) JSON
                    .parse("{Group:'"+groupName+"', ReplSize : 1}"));
            esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName, Integer.parseInt(SdbTestBase.esServiceName));
        }
        catch (ReliabilityException e) {
            if (sdb != null) {
                sdb.close();
            }
            Assert.fail(this.getClass().getName() + " setUp error, error description:"
                    + e.getMessage());
        }
    }

    public void insertData() {
		List<BSONObject> records = new ArrayList<BSONObject>();
		try {
			for(int i = 0; i < 5000; i++) {
				BSONObject record = (BSONObject)JSON.parse("{a:'a"+i+"',b:'b"+i+"'}");
				records.add(record);
			}
			cl.insert(records);
			records.clear();
		} catch (BaseException e) {
			if (-321 == e.getErrorCode() || e.getErrorCode() != -129) {
				throw new SkipException("---insert has an err:SEQUOIADBMAINSTREAM-3827---");
			}
		}
	}
    
    
    @Test
    public void test() {
        try {
		    cl.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
        	insertData();
        	FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, clName, fullIndexName, 5000);
        	GroupWrapper cLGroup = groupMgr.getGroupByName(groupName);
            NodeWrapper cLGroupMaster = cLGroup.getMaster();
            TaskMgr mgr = new TaskMgr(); 
            FaultMakeTask faultTask = KillNode.getFaultMakeTask(cLGroupMaster.hostName(),
            		cLGroupMaster.svcName(), 1);  
            AlterTask alterTask = new AlterTask();
            mgr.addTask(faultTask);
            mgr.addTask(alterTask);
            mgr.execute();
            insertData();
            Assert.assertEquals(mgr.isAllSuccess(), true, mgr.getErrorMsg());
            Assert.assertEquals(groupMgr.checkBusiness(600), true);
            DBCursor cursor = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{Name:'" +csName+ "." +clName+ "'}", null, null);
            if(!cursor.getNext().get("ReplSize").equals(3) || cl.getCount() != 5000){
            	Assert.fail("alter failed or records count error!");
            }
        }
        catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
        finally {
            sdb.closeAllCursors();
        }

    }

    private class AlterTask extends OperateTask {
		@Override
        public void exec() throws Exception {
            Sequoiadb db = null;
            try {
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                cl.setAttributes((BSONObject)JSON.parse("{ReplSize : 3}"));
                }catch (BaseException e){
                	e.printStackTrace();
            } finally {
                if (db != null) {
                    db.close();
                }
            }
        }
    }
    
    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace commCS = sdb.getCollectionSpace(csName);
            commCS.dropCollection(clName);
        }
        catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
        finally {
            if (sdb != null) {
                sdb.close();
            }
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }
    
}
