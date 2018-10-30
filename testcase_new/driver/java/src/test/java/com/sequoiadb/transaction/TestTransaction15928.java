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
import com.sequoiadb.testcommon.SdbConfTestBase;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName:TestTransaction15928 test query() with QUERY_FLG_FOR_UPDATE
 * @author wangkexin
 * @Date 2018-10-29
 * @version 1.00
 */
public class TestTransaction15928 extends SdbTestBase{
    private Sequoiadb sdb;
    private Sequoiadb sdb2;
    private CollectionSpace cs;
    private DBCollection cl;
    private String clName = "cl15928";
    private String commCSName;
    private ArrayList<BSONObject> insertRecods;
    List<BSONObject> actualList = new ArrayList<BSONObject>();
    
   /* @Override
    protected void setNodeConf(){
        dataConf.put("transactionon", true);
        dataConf.put("transisolation", 1);
        stdalnConf.put("transactionon", true);
    }*/
    
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
            Assert.fail("Sequoiadb driver TestTransaction15928 setUp error, error description:" + e.getMessage());
        }
    }
    
    @Test
    public void testTrans15928() {
        if (!Util.isCluster(this.sdb)) {
            return ;
        }
        //test query + update
        test(1);
        //clean up cl and insert data
        this.cs.dropCollection(clName);
        this.cl = this.cs.createCollection(clName, new BasicBSONObject("ReplSize", 0));
        insertData();
        //test queryOne + update
        test(2);
    }
    public void test(int i){
    	try {
    		DBCursor cursor = null;
    		BSONObject obj = null;
        	this.sdb.beginTransaction();
		    this.sdb2.beginTransaction();
		    DBCollection cl1 = this.sdb.getCollectionSpace(commCSName).getCollection(clName);
		    DBCollection cl2 = this.sdb2.getCollectionSpace(commCSName).getCollection(clName);
		    if(i == 1){
		    	cursor = cl1.query("","","","", DBQuery.FLG_QUERY_FOR_UPDATE);
		    }else{
		    	obj = cl1.queryOne(null,null,null,null, DBQuery.FLG_QUERY_FOR_UPDATE);
		    }
		    DBQuery query = new DBQuery();
		    query.setModifier((BSONObject) JSON.parse("{$set:{num:22}}"));
		    try{
		    	cl2.update(query);
		    }catch(BaseException e){
		    	Assert.assertEquals(e.getErrorCode(), -13);
		    	actualList.clear();
		    	if(i == 1){
		    		while(cursor.hasNext()) {
			            actualList.add(cursor.getNext());
			        }
				    cursor.close();
		            Assert.assertEquals(actualList, this.insertRecods);
		    	}else{
		    		BSONObject expobj =  new BasicBSONObject();
		    		expobj.put("_id", 0);
		    		expobj.put("num", 0);
		    		Assert.assertEquals(obj,expobj);
		    	}
		    	this.sdb.commit();
		    	cl2.update(query);
		    	this.sdb2.commit();
		    	checkResultAfterUpdate(cl2);
		    }
        }catch (BaseException e) { 
            Assert.fail("Sequoiadb driver TestTransaction15928 testTrans15928 error, error description:" + e.getMessage());
        }
    }
    public void checkResultAfterUpdate(DBCollection cl){
    	DBCursor cursor = cl.query();
    	actualList.clear();
    	while(cursor.hasNext()) {
            actualList.add(cursor.getNext());
        }
	    cursor.close();
	    
	    List<BSONObject> expectedList = new ArrayList<BSONObject>();
	    for (int i =0; i < this.insertRecods.size(); i++) {
            BSONObject obj = new BasicBSONObject();
            obj = this.insertRecods.get(i);
            obj.put("num", 22);
            expectedList.add(obj);
        }
        Assert.assertEquals(actualList, expectedList);
    }
    
    public void insertData(){
        try{
            BSONObject bson;
            this.insertRecods = new ArrayList<BSONObject>();
            for (int i = 0; i < 10000; i++) {
                bson = new BasicBSONObject();
                bson.put("_id", i);
                bson.put("num", i);
                this.insertRecods.add(bson);
            } 
            this.cl.insert(this.insertRecods, 0 );
        }catch (BaseException e) {
            Assert.fail("Sequoiadb driver TestTransaction15928 insertData error, error description:" + e.getMessage());
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
