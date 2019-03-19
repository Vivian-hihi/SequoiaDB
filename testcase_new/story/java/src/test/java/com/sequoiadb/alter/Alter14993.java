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
 * @FileName Alter14993.java
 * @Author luweikang
 * @Date 2019年3月11日
 */
public class Alter14993 extends SdbTestBase {
    
    private Sequoiadb sdb = null;
    private String clName = "cl_14993";
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        BSONObject options = new BasicBSONObject();
        options.put("ShardingType", "hash");
        options.put("ShardingKey", new BasicBSONObject("a", 1));
        options.put("Compressed", true);
        options.put("CompressionType", "snappy");
        options.put("StrictDataMode", false);
        options.put("ReplSize", 1);
        sdb.getCollectionSpace(csName).createCollection(clName, options);
    }
    
    @Test
    public void test(){
        AlterCL alterCL = new AlterCL();
        SetAttributes setAttributes = new SetAttributes();
        EnableCompression enableCompression = new EnableCompression();

        alterCL.start();
        setAttributes.start();
        enableCompression.start();
        
        Assert.assertTrue(alterCL.isSuccess(), alterCL.getErrorMsg());
        Assert.assertTrue(setAttributes.isSuccess(), setAttributes.getErrorMsg());
        Assert.assertTrue(enableCompression.isSuccess(), enableCompression.getErrorMsg());
        
        DBCursor snap = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, new BasicBSONObject("Name", csName + "." + clName), null, null);
        BSONObject clOption = snap.getNext();
        BSONObject shardingKey = (BSONObject) clOption.get("ShardingKey");
        int compressionType = (int) clOption.get("CompressionType");
        String compressionTypeDesc = (String)clOption.get("CompressionTypeDesc");
        String strictDataMode = (String) clOption.get("AttributeDesc");
        int replsize = (int) clOption.get("ReplSize");
        snap.close();
        
        Assert.assertEquals(shardingKey, new BasicBSONObject("b", -1), "check ShardingKey");
        Assert.assertEquals(compressionType, 1, "check CompressionType");
        Assert.assertEquals(compressionTypeDesc, "lzw", "check CompressionTypeDesc");
        Assert.assertEquals(strictDataMode, "Compressed | StrictDataMode", "check StrictDataMode");
        Assert.assertEquals(replsize, -1, "check ReplSize");
    }
    
    @AfterClass
    public void tearDown(){
        sdb.getCollectionSpace(csName).dropCollection(clName);
        if(sdb != null){
            sdb.close();
        }
    }
    
    public class AlterCL extends SdbThreadBase{//TODO:alter只执行一次撞到并发的概率大吗？？是否考虑和14992A的方式一样增加并发概率

        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                BSONObject options = new BasicBSONObject("ShardingKey", new BasicBSONObject("b", -1));
                cl.alterCollection(options);
            }
        }
    }
    
    public class SetAttributes extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                BSONObject options = new BasicBSONObject();
                options.put("StrictDataMode", true);
                options.put("ReplSize", -1);
                cl.setAttributes(options);
            }
        }
    }
    
    public class EnableCompression extends SdbThreadBase{
        
        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                BSONObject options = new BasicBSONObject("CompressionType", "lzw");
                cl.enableCompression(options);
            }
        }
    }
}

