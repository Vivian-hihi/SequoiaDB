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
        SetAttributes1 setAttributes1 = new SetAttributes1();
        SetAttributes2 setAttributes2 = new SetAttributes2();

        setAttributes1.start();
        setAttributes2.start();
        
        Assert.assertTrue(setAttributes1.isSuccess(), setAttributes1.getErrorMsg());
        Assert.assertTrue(setAttributes2.isSuccess(), setAttributes2.getErrorMsg());
        
        sdb.getCollectionSpace(csName).createCollection(clName);
        
        DBCursor snapCur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONSPACES, new BasicBSONObject("Name", csName), null, null);
        BSONObject csInfo = snapCur.getNext();
        Assert.assertEquals(csInfo.get("PageSize"), 16384, "check cs PageSize");//TODO：多处用到的数值建议用变量存储
        snapCur.close();
    }
    
    @AfterClass
    public void tearDown(){
        sdb.dropCollectionSpace(csName);
        if(sdb != null){
            sdb.close();
        }
    }
    
    public class SetAttributes1 extends SdbThreadBase{//TODO:SetAttributes1和SetAttributes2线程类的代码一样，建议使用一份

        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                CollectionSpace cs = db.getCollectionSpace(csName);
                BSONObject options = new BasicBSONObject();
                options.put("PageSize", 16384);
                cs.setAttributes(options);
            }
        }
    }
    
    public class SetAttributes2 extends SdbThreadBase{
        
        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                CollectionSpace cs = db.getCollectionSpace(csName);
                BSONObject options = new BasicBSONObject();
                options.put("PageSize", 16384);
                cs.setAttributes(options);
            }
        }
    }
}

