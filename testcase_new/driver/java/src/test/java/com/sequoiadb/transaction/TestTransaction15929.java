package com.sequoiadb.transaction;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBQuery;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbConfTestBase;

/**
 * @FileName:TestTransaction15929 test query() without QUERY_FLG_FOR_UPDATE
 * @author wangkexin
 * @Date 2018-10-30
 * @version 1.00
 */
public class TestTransaction15929 extends SdbConfTestBase{
    private Sequoiadb sdb;
    private Sequoiadb sdb2;
    private CollectionSpace cs;
    private DBCollection cl;
    private String clName = "cl15929";
    private String commCSName;
    private ArrayList<BSONObject> insertRecods;
    
   @Override
    protected void setNodeConf(){
        dataConf.put("transactionon", true);
        dataConf.put("transisolation", 1);
        stdalnConf.put("transactionon", true);
    }
    
   @BeforeClass
   public void setUp() {
	   String coordAddr = SdbTestBase.coordUrl;
	   commCSName = SdbTestBase.csName;
       try {
           sdb = new Sequoiadb(coordAddr, "", "");
           sdb2 = new Sequoiadb(coordAddr, "", "");
           
           CommLib commlib = new CommLib();
			if (commlib.isStandAlone(sdb)) {
				throw new SkipException("skip StandAlone");
			}
			
           if (!sdb.isCollectionSpaceExist(commCSName)) {
               try{
                   cs = sdb.createCollectionSpace(commCSName); 
               } catch (BaseException e) {
                   Assert.assertEquals(-33, e.getErrorCode(), e.getMessage());
               }
           } else {
               cs = sdb.getCollectionSpace(commCSName);
           }
           if (cs.isCollectionExist(clName)) {
               cs.dropCollection(clName);
           }
           cl = cs.createCollection(clName, new BasicBSONObject("ReplSize", 0));
           insertData();
       }catch (BaseException e) {
           Assert.fail("Sequoiadb driver TestTransaction15929 setUp error, error description:" + e.getMessage());
       }
   }
    
    @Test
    public void testTrans15929() {
        
        //test T1:query()/queryOne()  T2:commit()
        testQuery(true);
        clean();
        testQueryOne(true);
        clean();
        //test T1:query()/queryOne()  T2:rollback()
        testQuery(false);
        clean();
        testQueryOne(false);
    }
    
    public void testQuery(boolean ifCommit){
    	try {
            sdb.beginTransaction();
            sdb2.beginTransaction();
            DBCollection cl1 = sdb.getCollectionSpace(commCSName).getCollection(clName);
            DBCollection cl2 = sdb2.getCollectionSpace(commCSName).getCollection(clName);
            DBCursor cursor = cl1.query();
            DBQuery query = new DBQuery();
            query.setModifier((BSONObject) JSON.parse("{$set:{age:22}}"));
            checkQueryResult(cursor);
            cl2.update(query);
            sdb.commit();
            if(ifCommit){
            	sdb2.commit();
                DBCursor cursor2 = cl2.query();
                checkResultAfterUpdate(cursor2);
            }else{
            	sdb2.rollback();
            	DBCursor cursor2 = cl2.query();
            	checkQueryResult(cursor2);
            }
        }catch (BaseException e) { 
            Assert.fail("Sequoiadb driver TestTransaction15929 testQuery error, error description:" + e.getMessage());
        }
    }
    
    public void testQueryOne(boolean ifCommit){
    	try {
            sdb.beginTransaction();
            sdb2.beginTransaction();
            DBCollection cl1 = sdb.getCollectionSpace(commCSName).getCollection(clName);
            DBCollection cl2 = sdb2.getCollectionSpace(commCSName).getCollection(clName);
            BSONObject obj = cl1.queryOne();
            DBQuery query = new DBQuery();
            query.setModifier((BSONObject) JSON.parse("{$set:{age:22}}"));
            cl2.update(query);
            sdb.commit();
            //check result
            BSONObject expobj =  new BasicBSONObject();
    		expobj.put("_id", 0);
    		expobj.put("age", 0);
    		Assert.assertEquals(obj,expobj);
    		
            if(ifCommit){
            	sdb2.commit();
            	DBCursor cursor2 = cl2.query();
                checkResultAfterUpdate(cursor2);
            }else{
            	sdb2.rollback();
            	DBCursor cursor2 = cl2.query();
            	checkQueryResult(cursor2);
            }
        }catch (BaseException e) { 
            Assert.fail("Sequoiadb driver TestTransaction15929 testQueryOne error, error description:" + e.getMessage());
        }
    }
    
    public void checkQueryResult(DBCursor cursor){
    	List<BSONObject> actualList = new ArrayList<BSONObject>();
    	while(cursor.hasNext()) {
            actualList.add(cursor.getNext());
        }
	    cursor.close();
        Assert.assertEquals(actualList, insertRecods);
    }
    
    public void checkResultAfterUpdate(DBCursor cursor){
    	List<BSONObject> actualList = new ArrayList<BSONObject>();
    	while(cursor.hasNext()) {
            actualList.add(cursor.getNext());
        }
	    cursor.close();
	    
	    List<BSONObject> expectedList = new ArrayList<BSONObject>();
	    for (int i =0; i < insertRecods.size(); i++) {
            BSONObject obj = new BasicBSONObject();
            obj = insertRecods.get(i);
            obj.put("age", 22);
            expectedList.add(obj);
        }
        Assert.assertEquals(actualList, expectedList);
    }
    
    public void clean(){
    	cs.dropCollection(clName);
        cl = cs.createCollection(clName, new BasicBSONObject("ReplSize", 0));
        insertData();
    }
    
    public void insertData(){
        try{
            BSONObject bson;
            insertRecods = new ArrayList<BSONObject>();
            for (int i = 0; i < 10000; i++) {
                bson = new BasicBSONObject();
                bson.put("_id", i);
                bson.put("age", i);
                insertRecods.add(bson);
            } 
            cl.insert(insertRecods, 0 );
        }catch (BaseException e) {
            Assert.fail("Sequoiadb driver TestTransaction15929 insertData error, error description:" + e.getMessage());
        }
    }
    
    @AfterClass
    public void tearDown(){
        try{
	        CollectionSpace cs = sdb.getCollectionSpace(csName);    
	        if(cs.isCollectionExist(clName)){
	            cs.dropCollection(clName);
	        }
        }catch(BaseException e){            
            Assert.fail(e.getMessage());
        }finally{
            sdb.close();
            sdb2.close();
        }
    }
}
