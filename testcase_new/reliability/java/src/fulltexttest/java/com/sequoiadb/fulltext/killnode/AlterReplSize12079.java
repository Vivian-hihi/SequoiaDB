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
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fulltext.FullTextESUtils;
import com.sequoiadb.fulltext.FullTextUtils;
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
        	sdb = new Sequoiadb(SdbTestBase.coordUrl,"","");
    		CommLib commLib = new CommLib();
    		
    		if (commLib.isStandAlone(sdb)) {
    			throw new SkipException("StandAlone environment!");
    		}
            System.out.println(
                    "the TestCase Name:" + getClass().getName() + ". the TestCase begin at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            groupMgr = GroupMgr.getInstance();

            if (!groupMgr.checkBusiness(20)) {
                throw new SkipException("checkBusiness return false");
            }
            groupName = groupMgr.getAllDataGroupName().get(0);
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
        for(int i = 0; i < 5000; i++) {
            BSONObject record = (BSONObject)JSON.parse("{a:'a"+i+"',b:'b"+i+"'}");
            records.add(record);
            System.out.println(i);
        }
        cl.insert(records);
	}
    
    
    @Test
    public void test() {
        try {
		    cl.createIndex(fullIndexName, "{'a':'text'}", false, false);
        	insertData();
        	FullTextUtils.checkFullSyncToES(esClient, sdb, SdbTestBase.csName, clName, fullIndexName, 5000);
            System.out.println("start alter");
        	NodeWrapper cLGroupMaster = groupMgr.getGroupByName(groupName).getMaster();
            cl.setAttributes((BSONObject)JSON.parse("{ReplSize : 3}"));
            cLGroupMaster.stop();
            try{
                insertData(); 
                throw new SkipException("insert error!");
            }catch(BaseException e){
                if(e.getErrorCode() != -105){
                    Assert.fail(e.getMessage());
                }
            }            
            cLGroupMaster.start();
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
