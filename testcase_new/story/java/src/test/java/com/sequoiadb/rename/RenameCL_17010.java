package com.sequoiadb.rename;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 *  @FileName: RenameCL_17010
 *  @content 主子表，修改主表名，查看访问计划被清除
 *  @author luweikang
 *  @Date 2018-12-29
 *  @version 1.00
 */
public class RenameCL_17010 extends SdbTestBase{

    private String mainCSName = "maincs17010";
    private String subCSName = "subcs17010";
    private String mainCLName = "maincl17010";
    private String newMainCLName = "newMainCL17010";
    private String subCLName = "subcl17010";
    private Sequoiadb sdb = null;
    
    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if(CommLib.isStandAlone(sdb)){
            throw new SkipException("skip StandAlone");
        }
        if(sdb.isCollectionSpaceExist(mainCSName)){
            sdb.dropCollectionSpace(mainCSName);
        }
        if(sdb.isCollectionSpaceExist(subCSName)){
            sdb.dropCollectionSpace(subCSName);
        }
        
    }
    
    @Test
    public void test17010() {
        prepareCSCL();
        
        BSONObject matcher = new BasicBSONObject();
        matcher.put("Collection", mainCSName + "." + mainCLName);
        BSONObject selector = new BasicBSONObject();
        selector.put("CollectionSpace", 1);
        DBCursor cur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_ACCESSPLANS, matcher, selector, null);
        while(cur.hasNext()) {
            Assert.assertEquals(cur.getNext().toString(), "{ \"CollectionSpace\" : \""+mainCSName+"\" }");
        }
        cur.close();
        
        sdb.getCollectionSpace(mainCSName).renameCollection(mainCLName, newMainCLName);
        
        cur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_ACCESSPLANS, matcher, selector, null);
        while(cur.hasNext()) {
            Assert.fail("expected no records !");
        }
        
        DBCollection newMainCL = sdb.getCollectionSpace(mainCSName).getCollection(newMainCLName);
        DBCursor cursor = newMainCL.query();
        while(cursor.hasNext()){
            Assert.assertEquals(cursor.getNext().toString(), "{ \"_id\" : 1 , \"a\" : 1 , \"b\" : 1 }");
        }
        cursor.close();
        
        BSONObject newMatcher = new BasicBSONObject();
        newMatcher.put("Collection", mainCSName + "." + newMainCLName);
        cur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_ACCESSPLANS, newMatcher, selector, null);
        while(cur.hasNext()) {
            Assert.assertEquals(cur.getNext().toString(), "{ \"CollectionSpace\" : \""+mainCSName+"\" }");
        }
        
        cur.close();
    }
    
    public void prepareCSCL()
    {
        CollectionSpace mainCS = sdb.createCollectionSpace(mainCSName);
        BSONObject options = new BasicBSONObject();
        options.put("IsMainCL", true);
        options.put("ReplSize", 0);
        options.put("ShardingKey", new BasicBSONObject("a", 1));
        DBCollection mainCL = mainCS.createCollection(mainCLName, options);

        CollectionSpace subCS = sdb.createCollectionSpace(subCSName);
        options = new BasicBSONObject();
        options.put("ShardingKey", new BasicBSONObject("b", 1));
        subCS.createCollection(subCLName, options);
        
        options = new BasicBSONObject();
        options.put("LowBound", new BasicBSONObject("a", 0));
        options.put("UpBound", new BasicBSONObject("a", 10));
        mainCL.attachCollection(subCSName+"."+subCLName, options);
        
        mainCL.insert("{'_id':1,'a':1,'b':1}");
        DBCursor query = mainCL.query();
        while(query.hasNext()){
            Assert.assertEquals(query.getNext().toString(), "{ \"_id\" : 1 , \"a\" : 1 , \"b\" : 1 }");
        }
        query.close();
    }
    
    @AfterClass
    public void tearDown() {
        try {
            if(sdb.isCollectionSpaceExist(mainCSName)){
                sdb.dropCollectionSpace(mainCSName);
            }
            if(sdb.isCollectionSpaceExist(subCSName)){
                sdb.dropCollectionSpace(subCSName);
            }
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        } finally {
            if(this.sdb != null){
                this.sdb.close();
            }
        }
    }
}
