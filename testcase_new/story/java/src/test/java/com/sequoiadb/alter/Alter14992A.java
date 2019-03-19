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
public class Alter14992A extends SdbTestBase {
    
    private Sequoiadb sdb = null;
    private String clName = "cl_14992A";
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        BSONObject options = new BasicBSONObject();
        options.put("ShardingType", "hash");
        options.put("ShardingKey", new BasicBSONObject("a", 1));
        sdb.getCollectionSpace(csName).createCollection(clName, options);
    }
    
    @Test
    public void test(){
        BSONObject alterOption1 = new BasicBSONObject("ShardingKey", new BasicBSONObject("b", -1));
        BSONObject alterOption2 = new BasicBSONObject("ShardingKey", new BasicBSONObject("c", 1));
        AlterCL alter1 = new AlterCL(alterOption1, alterOption2);
        
        BSONObject alterOption3 = new BasicBSONObject("ShardingKey", new BasicBSONObject("d", 1));
        BSONObject alterOption4 = new BasicBSONObject("ShardingKey", new BasicBSONObject("e", -1));
        AlterCL alter2 = new AlterCL(alterOption3, alterOption4);

        alter1.start();
        alter2.start();
        
        Assert.assertTrue(alter1.isSuccess(), alter1.getErrorMsg());
        Assert.assertTrue(alter2.isSuccess(), alter2.getErrorMsg());
        
        DBCursor snap = sdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, new BasicBSONObject("Name", csName + "." + clName), null, null);
        BSONObject clOption = snap.getNext();
        BSONObject shardingKey = (BSONObject) clOption.get("ShardingKey");
        snap.close();
        
        Assert.assertTrue(shardingKey.equals(new BasicBSONObject("c", 1))||shardingKey.equals(new BasicBSONObject("e", -1)), "check shardingKey");
    }
    
    @AfterClass
    public void tearDown(){
        sdb.getCollectionSpace(csName).dropCollection(clName);
        if(sdb != null){
            sdb.close();
        }
    }
    
    public class AlterCL extends SdbThreadBase{

        private BSONObject attrOption1 = null;
        private BSONObject attrOption2 = null;
        
        public AlterCL(BSONObject options1, BSONObject options2) {
            this.attrOption1 = options1;
            this.attrOption2 = options2;
        }
        
        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
                for (int i = 0; i < 10; i++) {
                    cl.alterCollection(attrOption1);
                    cl.alterCollection(attrOption2);
                }
            }
        }
    }
    
}
