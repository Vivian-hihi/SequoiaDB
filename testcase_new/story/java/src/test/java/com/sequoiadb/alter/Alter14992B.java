package com.sequoiadb.alter;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @FileName Alter14992.java
 * @Author luweikang
 * @Date 2019年3月11日
 */
public class Alter14992B extends SdbTestBase {
    
    private Sequoiadb sdb = null;
    private String clName = "cl_14992B";
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        BSONObject options = new BasicBSONObject();
        options.put("Compressed", true);
        options.put("CompressionType", "snappy");
        sdb.getCollectionSpace(csName).createCollection(clName, options);
    }
    
    @Test
    public void test(){
        AlterCL alter = new AlterCL();
        alter.start(10);
        
        Assert.assertTrue(alter.isSuccess(), alter.getErrorMsg());
        
        DBCursor snap = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, new BasicBSONObject("Name", csName + "." + clName), null, null);
        BSONObject clOption = snap.getNext();
        int compressionType = (int) clOption.get("CompressionType");
        String compressionTypeDesc = (String)clOption.get("CompressionTypeDesc");
        snap.close();
        
        Assert.assertEquals(compressionType, 1, "check compressionType");
        Assert.assertEquals(compressionTypeDesc, "lzw", "check compressionTypeDesc");
    }
    
    @AfterClass
    public void tearDown(){
        sdb.getCollectionSpace(csName).dropCollection(clName);
        if(sdb != null){
            sdb.close();
        }
    }
    
    public class AlterCL extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                BSONObject alterOption = new BasicBSONObject("CompressionType", "lzw");
                cl.alterCollection(alterOption);
            }
        }
    }
}
