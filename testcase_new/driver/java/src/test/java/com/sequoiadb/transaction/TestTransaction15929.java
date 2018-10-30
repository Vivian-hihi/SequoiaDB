package com.sequoiadb.transaction;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterTest;
import org.testng.annotations.BeforeTest;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBQuery;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
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
    
    @BeforeTest
    public void setUp() {
        String coordAddr = SdbTestBase.coordUrl;
        this.commCSName = SdbTestBase.csName;
        try {
            System.out.println("the TestCase Name:" + this.getClass().getName() + 
                    ". the TestCase begin at:" + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            this.sdb = new Sequoiadb(coordAddr, "", "");
            this.sdb2 = new Sequoiadb(coordAddr, "", "");
            if (!this.sdb.isCollectionSpaceExist(this.commCSName)) {
                try{
                    this.cs = this.sdb.createCollectionSpace(this.commCSName); 
                } catch (BaseException e) {
                    Assert.assertEquals(-33, e.getErrorCode(), e.getMessage());
                }
            } else {
                this.cs = this.sdb.getCollectionSpace(this.commCSName);
            }
            if (this.cs.isCollectionExist(clName)) {
                this.cs.dropCollection(clName);
            }
            this.cl = this.cs.createCollection(clName, new BasicBSONObject("ReplSize", 0));
            insertData();
        }catch (BaseException e) {
            Assert.fail("Sequoiadb driver TestTransaction15929 setUp error, error description:" + e.getMessage());
        }
    }
    
    @Test
    public void testTrans15929() {
        if (!Util.isCluster(this.sdb)) {
            return ;
        }
        //test T1:query()/queryOne()  T2:commit()
        testQuery(true);
        clean();
        testQueryOne(true);
        clean();
        //test T1:query()/queryOne()  T2:rollback()
        testQuery(false);
        clean();
        testQueryOne(false);
        clean();
    }
    public void testQuery(boolean ifCommit){
    	try {
            this.sdb.beginTransaction();
            this.sdb2.beginTransaction();
            DBCollection cl1 = this.sdb.getCollectionSpace(commCSName).getCollection(clName);
            DBCollection cl2 = this.sdb2.getCollectionSpace(commCSName).getCollection(clName);
            DBCursor cursor = cl1.query();
            DBQuery query = new DBQuery();
            query.setModifier((BSONObject) JSON.parse("{$set:{age:22}}"));
            cl2.update(query);
            this.sdb.commit();
            checkQueryResult(cursor);
            if(ifCommit){
            	this.sdb2.commit();
                DBCursor cursor2 = cl2.query();
                checkResultAfterUpdate(cursor2);
            }else{
            	this.sdb2.rollback();
            	DBCursor cursor2 = cl2.query();
            	checkQueryResult(cursor2);
            }
        }catch (BaseException e) { 
            Assert.fail("Sequoiadb driver TestTransaction15929 testTrans15929 error, error description:" + e.getMessage());
        }
    }
    public void testQueryOne(boolean ifCommit){
    	try {
            this.sdb.beginTransaction();
            this.sdb2.beginTransaction();
            DBCollection cl1 = this.sdb.getCollectionSpace(commCSName).getCollection(clName);
            DBCollection cl2 = this.sdb2.getCollectionSpace(commCSName).getCollection(clName);
            BSONObject obj = cl1.queryOne();
            DBQuery query = new DBQuery();
            query.setModifier((BSONObject) JSON.parse("{$set:{age:22}}"));
            cl2.update(query);
            this.sdb.commit();
            //check result
            BSONObject expobj =  new BasicBSONObject();
    		expobj.put("_id", 0);
    		expobj.put("age", 0);
    		Assert.assertEquals(obj,expobj);
    		
            if(ifCommit){
            	this.sdb2.commit();
            	DBCursor cursor2 = cl2.query();
                checkResultAfterUpdate(cursor2);
            }else{
            	this.sdb2.rollback();
            	DBCursor cursor2 = cl2.query();
            	checkQueryResult(cursor2);
            }
        }catch (BaseException e) { 
            Assert.fail("Sequoiadb driver TestTransaction15929 testTrans15929 error, error description:" + e.getMessage());
        }
    }
    public void checkQueryResult(DBCursor cursor){
    	List<BSONObject> actualList = new ArrayList<BSONObject>();
    	while(cursor.hasNext()) {
            actualList.add(cursor.getNext());
        }
	    cursor.close();
        Assert.assertEquals(actualList, this.insertRecods);
    }
    
    public void checkResultAfterUpdate(DBCursor cursor){
    	List<BSONObject> actualList = new ArrayList<BSONObject>();
    	while(cursor.hasNext()) {
            actualList.add(cursor.getNext());
        }
	    cursor.close();
	    
	    List<BSONObject> expectedList = new ArrayList<BSONObject>();
	    for (int i =0; i < this.insertRecods.size(); i++) {
            BSONObject obj = new BasicBSONObject();
            obj = this.insertRecods.get(i);
            obj.put("age", 22);
            expectedList.add(obj);
        }
        Assert.assertEquals(actualList, expectedList);
    }
    public void clean(){
    	this.cs.dropCollection(clName);
        this.cl = this.cs.createCollection(clName, new BasicBSONObject("ReplSize", 0));
        insertData();
    }
    
    public void insertData(){
        try{
            BSONObject bson;
            this.insertRecods = new ArrayList<BSONObject>();
            for (int i = 0; i < 5; i++) {
                bson = new BasicBSONObject();
                bson.put("_id", i);
                bson.put("age", i);
                this.insertRecods.add(bson);
            } 
            this.cl.insert(this.insertRecods, 0 );
        }catch (BaseException e) {
            Assert.fail("Sequoiadb driver TestTransaction15929 insertData error, error description:" + e.getMessage());
        }
    }
    
    @AfterTest
    public void tearDown() {
        System.out.println("the TestCase Name:" + this.getClass().getName() + 
                ". the TestCase end at:" + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        if (this.cs.isCollectionExist(clName)) {
            this.cs.dropCollection(clName);
        }
        this.sdb.close();
        this.sdb2.close();
    }  
}
