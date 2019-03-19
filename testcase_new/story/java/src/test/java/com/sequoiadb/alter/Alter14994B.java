package com.sequoiadb.alter;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @FileName Alter14994.java
 * @Author luweikang
 * @Date 2019年3月11日
 */
public class Alter14994B extends SdbTestBase {
    
    private Sequoiadb sdb = null;
    private String csName = "cs_14994B";
    private String clName = "cl_14994B";
    private int newPageSize = 16384;
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        sdb.createCollectionSpace(csName, new BasicBSONObject("PageSize", 4096));
    }
    
    @Test
    public void test(){
        SetAttributes setAttributes = new SetAttributes();
        setAttributes.start(10);
        
        Assert.assertTrue(setAttributes.isSuccess(), setAttributes.getErrorMsg());
        
        sdb.getCollectionSpace(csName).createCollection(clName);
        
        DBCursor snapCur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONSPACES, new BasicBSONObject("Name", csName), null, null);
        BSONObject csInfo = snapCur.getNext();
        Assert.assertEquals(csInfo.get("PageSize"), newPageSize, "check cs PageSize");
        snapCur.close();
    }
    
    @AfterClass
    public void tearDown(){
        sdb.dropCollectionSpace(csName);
        if(sdb != null){
            sdb.close();
        }
    }
    
    public class SetAttributes extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                CollectionSpace cs = db.getCollectionSpace(csName);
                BSONObject options = new BasicBSONObject();
                options.put("PageSize", newPageSize);
                cs.setAttributes(options);
            }
        }
    }
    
}

