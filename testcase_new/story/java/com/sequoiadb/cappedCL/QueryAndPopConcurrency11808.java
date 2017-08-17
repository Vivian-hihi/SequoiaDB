package com.sequoiadb.cappedCL;

import java.util.List;
import java.util.Random;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.cappedCL.Commlib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
* FileName: QueryAndPopConcurrency11808.java
* test content:test concurrentcy query and pop at the same time for cappedCL
* @author liuxiaoxuan
    * @Date    2017.8.16
*/
public class QueryAndPopConcurrency11808 extends SdbTestBase{
	
	private Sequoiadb sdb = null;
	private DBCollection cappedCL_11808 = null;
	private String cappedCSName_11808 = "story_java_cappedCS_11808";
	private String cappedCLName_11808 = "cappedCL_11808";
	private List<Long> lids = new ArrayList<>();
	private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	
	
	@BeforeClass
	public void setUp() {
		System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
		try {
			boolean isCapped = true;
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
			sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
			cappedCL_11808 = Commlib.createCL(sdb, cappedCSName_11808, cappedCLName_11808, isCapped);
			int recordNums = 100;
			insertRecords(recordNums);
		}catch(BaseException e) {
			Assert.fail(e.getMessage());
		}
	}
	
	@Test
	public void testGreatConcurrencyQuery() {
		QueryThread queryThread = new QueryThread();
		PopThread popThread = new PopThread();
		
		queryThread.start();	
		popThread.start();
		
		Assert.assertTrue(queryThread.isSuccess(),queryThread.getErrorMsg());
		Assert.assertTrue(popThread.isSuccess(),popThread.getErrorMsg());
	}
	
	@AfterClass
	public void tearDown() {
		try {
			CollectionSpace cs = sdb.getCollectionSpace(cappedCSName_11808);
			if(cs != null && cs.isCollectionExist(cappedCLName_11808)) {
				sdb.dropCollectionSpace(cappedCSName_11808);
			}
		}catch (BaseException e) {
			Assert.fail(e.getMessage());
		}finally {
			sdb.close();
			System.out.println(this.getClass().getName()+" end at "+sdf.format(new Date()));
		}
	}
	
	public void insertRecords(int recordNums) {
		 for(int i = 0; i < recordNums; i++) { 
            BSONObject obj = (BSONObject)JSON.parse("{ a :" + i + "}"); 
            cappedCL_11808.insert(obj);
	     }
		 
		 //save logincalIds
		 BSONObject orderBy = new BasicBSONObject();
         orderBy.put("_id", 1);
         DBCursor cursor = cappedCL_11808.query(null,null,orderBy,null);
        
         while(cursor.hasNext()) {
         	long _id = (long)cursor.getNext().get("_id");
         	lids.add(_id);
         }
         cursor.close();
	}
	
	private class QueryThread extends SdbThreadBase{
    	
    	@Override
        public void exec() throws Exception{
            Sequoiadb db = null;
            DBCollection cl = null;
            try{
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                db.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
                cl = db.getCollectionSpace(cappedCSName_11808).getCollection(cappedCLName_11808);
                
                int stringLength = 1;
                Commlib.checkLogicalID(cl, stringLength, Thread.currentThread().getName());
                
            }catch(BaseException e){
                if(e.getErrorCode() != -23 || e.getErrorCode() != -34){
                    throw e;
                }
            }finally{
                db.close();
            }
        }
	}

	
	 private class PopThread extends SdbThreadBase{
	    	
	    	@Override
	        public void exec() throws Exception{
	            Sequoiadb db = null;
	            DBCollection cl = null;
	            try{
	                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
	                cl = db.getCollectionSpace(cappedCSName_11808).getCollection(cappedCLName_11808);
	                 
	                //start from middle , find a random logicalID
	                int min = lids.size() / 2;
	                int range = lids.size() / 2;
	                int pos = min + new Random().nextInt(range);
	                long logicalID = lids.get(pos);
	                System.out.println("random logicalID: " + logicalID);
	                // pop records 	 
	                BSONObject popObj = new BasicBSONObject();
	                popObj.put("LogicalID", logicalID);
	                popObj.put("Direction", -1);
	                cl.pop(popObj);
	                
	            }catch(BaseException e){
	                if(e.getErrorCode() != -23 || e.getErrorCode() != -34){
	                    throw e;
	                }
	            }finally{
	                db.close();
	            }
	        }
		}

}
