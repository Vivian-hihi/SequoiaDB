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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fulltext.FullTextDBUtils;
import com.sequoiadb.fulltext.FullTextESUtils;
import com.sequoiadb.fulltext.FullTextUtils;
/**
 * @Description seqDB-12079: 集合中存在全文索引，修改普通集合的副本数 
 * @author xiaoni Zhao
 * @date 2018/12/3
 */
public class InsertUpdateDel12010 extends SdbTestBase {
	private String clName = "ES_cl_12010";
    private CollectionSpace cs;
    private DBCollection cl;
    private GroupMgr groupMgr = null;
    private Sequoiadb sdb = null;
    private String groupName;
    private Client esClient = null;
    private String fullIndexName = "fullIndex12010";
    private int insertNum = 5000;
    private List<BSONObject> actRecords = new ArrayList<BSONObject>();

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

            // CheckBusiness(true),检测当前集群环境，若存在异常返回false，
            if (!groupMgr.checkBusiness(20)) {
                throw new SkipException("checkBusiness return false");
            }
            groupName = groupMgr.getAllDataGroupName().get(0);
            cs = sdb.getCollectionSpace(csName);
            cl = cs.createCollection(clName, (BSONObject) JSON
                    .parse("{Group:'"+groupName+"', ReplSize : 7}"));
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

    @Test
    public void test() {
    	Sequoiadb db = null;
        try {
        	db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		    cl.createIndex(fullIndexName, "{'a':'text'}", false, false);
		    List<BSONObject> insertRecords = insert();
        	FullTextUtils.checkFullSyncToES(esClient, db, csName, clName, fullIndexName, insertNum);
        	
        	//insert records
            NodeWrapper cLGroupMaster = groupMgr.getGroupByName(groupName).getMaster();
            cLGroupMaster.stop();
            insert();
            cLGroupMaster.start();
        	System.out.println("===start end===");
            Assert.assertEquals(groupMgr.checkBusiness(600), true);
            
            FullTextUtils.checkFullSyncToES(esClient, db, csName, clName, fullIndexName, insertNum);
            actRecords = FullTextDBUtils.getRecordsFromCL(cl, (BSONObject)JSON.parse("{'':{$Text:{query:{match_all:{}}}}}"), 
       		     (BSONObject)JSON.parse("{a : ''}"), (BSONObject)JSON.parse("{_id : 1}"), null, 0, insertNum);
            checkRecords(insertRecords, actRecords);
            
            //update records
            cLGroupMaster = groupMgr.getGroupByName(groupName).getMaster();
            cLGroupMaster.stop();
            update();
            cLGroupMaster.start();
            Assert.assertEquals(groupMgr.checkBusiness(600), true);
            
            FullTextUtils.checkFullSyncToES(esClient, db, SdbTestBase.csName, clName, fullIndexName, insertNum);
            actRecords = FullTextDBUtils.getRecordsFromCL(cl, (BSONObject)JSON.parse("{'':{$Text:{query:{match_all:{}}}}}"), 
            		     (BSONObject)JSON.parse("{a : ''}"), (BSONObject)JSON.parse("{_id : 1}"), null, 0, insertNum);
            checkRecords(insertRecords, actRecords);
            
            //delete records
            cLGroupMaster = groupMgr.getGroupByName(groupName).getMaster();
            cLGroupMaster.stop();
            delete();
            cLGroupMaster.start();
            Assert.assertEquals(groupMgr.checkBusiness(600), true);
            FullTextUtils.checkFullSyncToES(esClient, db, SdbTestBase.csName, clName, fullIndexName, insertNum);
            actRecords = FullTextDBUtils.getRecordsFromCL(cl, (BSONObject)JSON.parse("{'':{$Text:{query:{match_all:{}}}}}"), 
       		     (BSONObject)JSON.parse("{a : ''}"), (BSONObject)JSON.parse("{_id : 1}"), null, 0, insertNum);
            checkRecords(insertRecords, actRecords);
            
        }
        catch (ReliabilityException e) {
            e.printStackTrace();
            Assert.fail(e.getMessage());
        }
        finally {
        	db.closeAllCursors();
        }

    }
    
    public List<BSONObject> insert(){
    	Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        List<BSONObject> insertRecords = new ArrayList<BSONObject>();
         try {
             DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
             for(int i = 0; i < insertNum; i++) {
 				BSONObject record = (BSONObject)JSON.parse("{a:'a"+i+"'}");
 				insertRecords.add(record);
 			}
 			cl.insert(insertRecords);
             }catch (BaseException e){
            	if(e.getErrorCode() != -105){
            		Assert.fail("insert error!"+e.getErrorCode());
            	}
         } finally {
             if (db != null) {
            	 db.close();
             }
         }
		return insertRecords;
    }
    
    public void update() {
    	Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            for(int i = 0; i < insertNum; i++){
                cl.update("{a: 'a" +i+ "'}", "{$set: {a : 'a'}}", null);
            }
            }catch (BaseException e){
            	if(e.getErrorCode() != -105){
            		Assert.fail("insert error!");
            	}
        } finally {
            if (db != null) {
            	db.close();
            }
        }
    }
    
    public void delete() {
    	Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
            for(int i = 0; i < insertNum; i++){
                cl.delete("{a : 'a'}");
            }
            }catch (BaseException e){
            	if(e.getErrorCode() != -105){
            		Assert.fail("insert error!");
            	}
        } finally {
            if (db != null) {
            	db.close();
            }
        }
    }
    
    public void  checkRecords( List<BSONObject> expRecords, List<BSONObject> actRecords)
    {
	   if( expRecords.size() != actRecords.size())
	   {
	      Assert.fail("records count has error!");
	   }

	   for( int i = 0; i <  expRecords.size(); i++ )
	   {
           if(!expRecords.get(i).get("a").equals(actRecords.get(i).get("a"))){
        	   Assert.fail("records has error!");
           }
	   }
    }

    @AfterClass
    public void tearDown() {
    	Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        try {
            CollectionSpace commCS = db.getCollectionSpace(csName);
            commCS.dropCollection(clName);
        }
        catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
        finally {
            if (db != null) {
            	db.close();
            }
            System.out.println(
                    "the TestCase Name:" + this.getClass().getName() + ". the TestCase end at:"
                            + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        }
    }

}
