package com.sequoiadb.rename;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 *  @FileName: TestRenameCS16864
 *  @content 主子表，修改主表cs名，查看访问计划被清除
 *  @author chensiqin
 *  @Date 2018-12-20
 *  @version 1.00
 */
public class TestRenameCS16864 extends SdbTestBase{

    private String mainCSName = "maincs16864";
    private String newMainCSName = "newmaincs16864";
    private String subCSName = "subcs16864";
    private String mainCLName = "maincl16864";
    private String subCLName = "subcl16864";
    private Sequoiadb sdb = null;
    
    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if(sdb.isCollectionSpaceExist(mainCSName)){
            sdb.dropCollectionSpace(mainCSName);
        }
        if(sdb.isCollectionSpaceExist(newMainCSName)){
            sdb.dropCollectionSpace(newMainCSName);
        }
        if(sdb.isCollectionSpaceExist(subCSName)){
            sdb.dropCollectionSpace(subCSName);
        }
        
    }
    
    @Test
    public void test16864() {
        prepareCSCL();
        
        BSONObject matcher = new BasicBSONObject();
        matcher.put("CollectionSpace", mainCSName);
        BSONObject selector = new BasicBSONObject();
        selector.put("Collection", 1);
        DBCursor cur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_ACCESSPLANS, matcher, selector, null);
        while(cur.hasNext()) {
            Assert.assertEquals(cur.getNext().toString(), "{ \"Collection\" : \""+mainCSName+"."+mainCLName+"\" }");
        }
        cur.close();
        
        sdb.renameCollectionSpace(mainCSName, newMainCSName);
        
        cur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_ACCESSPLANS, matcher, selector, null);
        while(cur.hasNext()) {
            Assert.fail("expected no records !");
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
            if(sdb.isCollectionSpaceExist(newMainCSName)){
                sdb.dropCollectionSpace(newMainCSName);
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
