package com.story.basicoperation;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterTest;
import org.testng.annotations.BeforeTest;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBQuery;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName:TestQueryOne7090
 * queryOne (BSONObject matcher, BSONObject selector, BSONObject orderBy, BSONObject hint, int flag)
 * @author chensiqin
 * @version 1.00
 *
 */

public class TestQueryOne7090 extends SdbTestBase{
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String clName = "cl7090";
    private ArrayList<BSONObject> insertRecods;
    
    @BeforeTest
    public void setUp() {
        String coordAddr = SdbTestBase.coordUrl;
        String commCSName = SdbTestBase.csName;
        try {
            System.out.println("the TestCase Name:" + this.getClass().getName() + 
                    ". the TestCase begin at:" + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            this.sdb = new Sequoiadb(coordAddr, "", "");
            if (!this.sdb.isCollectionSpaceExist(commCSName)) {
                try{
                    this.cs = this.sdb.createCollectionSpace(commCSName); 
                } catch (BaseException e) {
                    Assert.assertEquals(-33, e.getErrorCode(), e.getMessage());
                }
            } else {
                this.cs = this.sdb.getCollectionSpace(commCSName);
            }
            if (this.cs.isCollectionExist(clName)) {
                this.cs.dropCollection(clName);
            }
            this.cl = this.cs.createCollection(clName);
            this.cl.createIndex("numIndex", (BSONObject) JSON.parse("{num:-1}"), false, false);
        }catch (BaseException e) {
            System.out.println("Sequoiadb driver TestQueryOne7090 setUp error, error description:" + e.getMessage());
            Assert.fail("Sequoiadb driver TestQueryOne7090 setUp error, error description:" + e.getMessage());
        }
    }
    
    @Test
    public void test() {
        checkQueryOneNoData();
        insertData();
        checkQueryOne();
    }
    
    public void insertData(){
        try{
            BSONObject bson;
            this.insertRecods = new ArrayList<BSONObject>();
            for (int i = 1; i <= 10; i++) {
                bson = new BasicBSONObject();
                bson.put("name", "chensiqin" + i);
                bson.put("age", i);
                bson.put("num", -1*i);
                this.insertRecods.add(bson);
            } 
            this.cl.bulkInsert(this.insertRecods, 0 );
        }catch (BaseException e) {
            System.out.println("Sequoiadb driver TestQueryOne7090 insertData error, error description:" + e.getMessage());
            Assert.fail("Sequoiadb driver TestQueryOne7090 insertData error, error description:" + e.getMessage());
        }
    }
    
    public void checkQueryOne() {
        BSONObject matcher, selector, orderBy, hint;
        try {
            matcher = (BSONObject) JSON.parse("{age:{$et:6}}");
            selector = (BSONObject) JSON.parse("{_id:{$include:0}}");
            orderBy = (BSONObject) JSON.parse("{num:-1}");
            hint = (BSONObject) JSON.parse("{\"\":\"numIndex\"}"); 
            BSONObject actualObject = 
                    this.cl.queryOne(matcher, selector, orderBy, hint, DBQuery.FLG_QUERY_FORCE_HINT);
            BSONObject expectedObject = this.insertRecods.get(5);
            expectedObject.removeField("_id");
            Assert.assertEquals(actualObject, expectedObject, "Sequoiadb driver TestQueryOne7090 checkQueryOne" +
                    "actualList:" +actualObject + "; expectedList:" + expectedObject);
        }catch (BaseException e) {
            System.out.println("Sequoiadb driver TestQueryOne7090 checkQueryOne error, error description:" + e.getMessage());
            Assert.fail("Sequoiadb driver TestQueryOne7090 checkQueryOne error:" + e.getMessage());
        }
    }
    
    public void checkQueryOneNoData() {
        BSONObject matcher, selector, orderBy, hint;
        try {
            matcher = (BSONObject) JSON.parse("{age:{$ne:6}}");
            selector = (BSONObject) JSON.parse("{_id:{$include:1}}");
            orderBy = (BSONObject) JSON.parse("{num:1}");
            hint = (BSONObject) JSON.parse("{\"\":\"numIndex\"}"); 
            BSONObject actualObject = 
                    this.cl.queryOne(matcher, selector, orderBy, hint, DBQuery.FLG_QUERY_FORCE_HINT);
            BSONObject expectedObject = null;
            Assert.assertEquals(actualObject, expectedObject, "Sequoiadb driver TestQueryOne7090 checkQueryOneNoData" +
                    "actualList:" +actualObject + "; expectedList:" + expectedObject);
        }catch (BaseException e) {
            System.out.println("Sequoiadb driver TestQueryOne7090 checkQueryOne error, error description:" + e.getMessage());
            Assert.fail("Sequoiadb driver TestQueryOne7090 checkQueryOne error:" + e.getMessage());
        }
    }
    
    @AfterTest
    public void tearDown() {
        System.out.println("the TestCase Name:" + this.getClass().getName() + 
                ". the TestCase end at:" + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
        if (this.cs.isCollectionExist(clName)) {
            this.cs.dropCollection(clName);
        }
        this.sdb.disconnect();
    }
}
