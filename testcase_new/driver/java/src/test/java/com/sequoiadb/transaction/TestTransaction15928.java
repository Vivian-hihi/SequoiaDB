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
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName:TestTransaction15928 test query() with QUERY_FLG_FOR_UPDATE
 * @author wangkexin
 * @Date 2018-10-29
 * @version 1.00
 */
public class TestTransaction15928 extends SdbConfTestBase{
    private Sequoiadb sdb;
    private Sequoiadb sdb2;
    private CollectionSpace cs;
    private DBCollection cl;
    private String clName = "cl15928";
    private String commCSName ;
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
            Assert.fail("Sequoiadb driver TestTransaction15928 setUp error, error description:" + e.getMessage());
        }
    }
    
    @Test
    private void testTrans15928() {
    	String flagCom = "commit";
    	String flagRoll = "rollback";
    	
        //test query + update
        testQueryAndUpdate(flagCom);
        //clean up cl and insert data
        cleanAndInsert();
        
        testQueryAndUpdate(flagRoll);
        cleanAndInsert();
        
        //test queryOne + update
        testQueryOneAndUpdate(flagCom);
        cleanAndInsert();
        
        testQueryOneAndUpdate(flagRoll);
    }
    
    private void testQueryAndUpdate(String flag){
    	try {
    		sdb.beginTransaction();
		    sdb2.beginTransaction();
		    DBCollection cl1 = sdb.getCollectionSpace(commCSName).getCollection(clName);
		    DBCollection cl2 = sdb2.getCollectionSpace(commCSName).getCollection(clName);
		    
		    DBCursor cursor = cl1.query("","","","", DBQuery.FLG_QUERY_FOR_UPDATE);
		    List<BSONObject> actualList = new ArrayList<BSONObject>();
    		while(cursor.hasNext()) {
	            actualList.add(cursor.getNext());
	        }
		    cursor.close();
            Assert.assertEquals(actualList, insertRecods);
            
		    DBQuery query = new DBQuery();
		    query.setModifier((BSONObject) JSON.parse("{$set:{num:22}}"));
		    
    		switch (flag) {
			case "commit":
				try{
			    	cl2.update(query);
			    	Assert.fail("update should fail");
			    }catch(BaseException e){
			    	Assert.assertEquals(e.getErrorCode(), -13);
			    	
			    	sdb.commit();
			    	cl2.update(query);
			    	checkResultAfterUpdate(cl2);
			    }
				break;
			case "rollback":
				try{
			    	cl2.update(query);
			    	Assert.fail("update should fail");
			    }catch(BaseException e){
			    	Assert.assertEquals(e.getErrorCode(), -13);
			    	
			    	sdb.rollback();
			    	cl2.update(query);
			    	checkResultAfterUpdate(cl2);
			    }
				break;

			default:
				Assert.fail("The parameter is not commit or rollback,please check it again!");
				break;
			}
        }catch (BaseException e) { 
            Assert.fail("Sequoiadb driver TestTransaction15928 testQueryAndUpdate error, error description:" + e.getMessage());
        }
    }
    
    private void testQueryOneAndUpdate(String flag){
    	try {
    		sdb.beginTransaction();
		    sdb2.beginTransaction();
		    DBCollection cl1 = sdb.getCollectionSpace(commCSName).getCollection(clName);
		    DBCollection cl2 = sdb2.getCollectionSpace(commCSName).getCollection(clName);
		    
		    BSONObject obj = cl1.queryOne(null,null,null,null, DBQuery.FLG_QUERY_FOR_UPDATE);
		    DBQuery query = new DBQuery();
		    query.setModifier((BSONObject) JSON.parse("{$set:{num:22}}"));
		    
    		switch (flag) {
			case "commit":
				try{
			    	cl2.update(query);
			    	Assert.fail("update should fail");
			    }catch(BaseException e){
			    	Assert.assertEquals(e.getErrorCode(), -13);
			    	
			    	BSONObject expobj =  new BasicBSONObject();
		    		expobj.put("_id", 0);
		    		expobj.put("num", 0);
		    		Assert.assertEquals(obj,expobj);
			    	
			    	sdb.commit();
			    	cl2.update(query);
			    	checkResultAfterUpdate(cl2);
			    }
				break;
			case "rollback":
				try{
			    	cl2.update(query);
			    	Assert.fail("update should fail");
			    }catch(BaseException e){
			    	Assert.assertEquals(e.getErrorCode(), -13);
			    	
			    	BSONObject expobj =  new BasicBSONObject();
		    		expobj.put("_id", 0);
		    		expobj.put("num", 0);
		    		Assert.assertEquals(obj,expobj);
			    	
			    	sdb.rollback();
			    	cl2.update(query);
			    	checkResultAfterUpdate(cl2);
			    }
				break;

			default:
				Assert.fail("The parameter is not commit or rollback,please check it again!");
				break;
			}
        }catch (BaseException e) { 
            Assert.fail("Sequoiadb driver TestTransaction15928 testQueryOneAndUpdate error, error description:" + e.getMessage());
        }
    }
    
    private void cleanAndInsert(){
    	cs.dropCollection(clName);
        cl = cs.createCollection(clName, new BasicBSONObject("ReplSize", 0));
        insertData();
    }
    
    private void checkResultAfterUpdate(DBCollection cl){
    	List<BSONObject> actualList = new ArrayList<BSONObject>();
    	DBCursor cursor = cl.query();
    	while(cursor.hasNext()) {
            actualList.add(cursor.getNext());
        }
	    cursor.close();
	    
	    List<BSONObject> expectedList = new ArrayList<BSONObject>();
	    for (int i =0; i < insertRecods.size(); i++) {
            BSONObject obj = new BasicBSONObject();
            obj = insertRecods.get(i);
            obj.put("num", 22);
            expectedList.add(obj);
        }
        Assert.assertEquals(actualList, expectedList);
    }
    
    private void insertData(){
        try{
            BSONObject bson;
            insertRecods = new ArrayList<BSONObject>();
            for (int i = 0; i < 10000; i++) {
                bson = new BasicBSONObject();
                bson.put("_id", i);
                bson.put("num", i);
                insertRecods.add(bson);
            } 
            cl.insert(insertRecods, 0 );
        }catch (BaseException e) {
            Assert.fail("Sequoiadb driver TestTransaction15928 insertData error, error description:" + e.getMessage());
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
