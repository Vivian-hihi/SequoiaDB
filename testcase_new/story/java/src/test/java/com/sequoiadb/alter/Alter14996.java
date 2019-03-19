package com.sequoiadb.alter;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @FileName Alter14996.java
 * @Author luweikang
 * @Date 2019年3月11日
 */
public class Alter14996 extends SdbTestBase {
    
    private Sequoiadb sdb = null;
    private String csName = "cs_14996";
    private String clName = "cl_14996";
    
    @BeforeClass(enabled = false)
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        sdb.createCollectionSpace(csName, new BasicBSONObject("PageSize", 4096));
    }
    
    //TODO SEQUOIADBMAINSTREAM-4274
    @Test(enabled = false)
    public void test(){
        AlterCS alterCS = new AlterCS();
        CreateCL createCL = new CreateCL();

        alterCS.start();
        createCL.start();
        
        boolean alterCSResult = alterCS.isSuccess();
        boolean createCLResult = createCL.isSuccess();
        
        if(alterCSResult&&createCLResult){
            Assert.assertTrue(sdb.getCollectionSpace(csName).isCollectionExist(clName), "check cl exist");
            DBCursor snapCur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONSPACES, new BasicBSONObject("Name", csName), null, null);
            BSONObject csInfo = snapCur.getNext();
            Assert.assertEquals(csInfo.get("PageSize"), 16384, "check cs PageSize");
            snapCur.close();
        }else if(alterCSResult&&!createCLResult){
            Assert.assertFalse(sdb.getCollectionSpace(csName).isCollectionExist(clName), "check cl exist");
            sdb.getCollectionSpace(csName).createCollection(clName);
            DBCursor snapCur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONSPACES, new BasicBSONObject("Name", csName), null, null);
            BSONObject csInfo = snapCur.getNext();
            Assert.assertEquals(csInfo.get("PageSize"), 16384, "check cs PageSize");
            snapCur.close();
        }else if(!alterCSResult&&createCLResult){
            Assert.assertTrue(sdb.getCollectionSpace(csName).isCollectionExist(clName), "check cl exist");
            DBCursor snapCur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONSPACES, new BasicBSONObject("Name", csName), null, null);
            BSONObject csInfo = snapCur.getNext();
            Assert.assertEquals(csInfo.get("PageSize"), 4096, "check cs PageSize");
            snapCur.close();
        }else{
            Assert.fail("setAttr and createCL all failed, setAttr: "+ alterCS.getErrorMsg() +", createCL: " + createCL.getErrorMsg());
        }
    }
    
    @AfterClass(enabled = false)
    public void tearDown(){
        sdb.dropCollectionSpace(csName);
        if(sdb != null){
            sdb.close();
        }
    }
    
    public class AlterCS extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                CollectionSpace cs = db.getCollectionSpace(csName);
                BSONObject options = new BasicBSONObject();
                options.put("PageSize", 16384);
                cs.alterCollectionSpace(options);
            }
        }
    }
    
    public class CreateCL extends SdbThreadBase{
        
        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                CollectionSpace cs = db.getCollectionSpace(csName);
                cs.createCollection(clName);
            }
        }
    }
}

