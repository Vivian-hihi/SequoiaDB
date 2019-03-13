package com.sequoiadb.alter;

import java.util.List;

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
 * @FileName Alter14995.java
 * @Author luweikang
 * @Date 2019年3月11日
 */
public class Alter14995 extends SdbTestBase {
    
    private Sequoiadb sdb = null;
    private String csName = "cs_14995";
    private String clName = "cl_14995";
    private String domainName = "domain_14995";
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        List<String> groupNames = CommLib.getDataGroupNames(sdb);
        
        BSONObject options1 = new BasicBSONObject();
        String[] domain1RG = new String[1];
        domain1RG[0] = groupNames.get(0);
        options1.put("Groups", domain1RG);
        sdb.createDomain(domainName, options1);
        
        sdb.createCollectionSpace(csName);
    }
    
    @Test
    public void test(){
        SetDomain setDomain = new SetDomain();
        SetAttributes1 setAttributes1 = new SetAttributes1();
        SetAttributes2 setAttributes2 = new SetAttributes2();

        setDomain.start();
        setAttributes1.start();
        setAttributes2.start();
        
        Assert.assertTrue(setDomain.isSuccess(), setDomain.getErrorMsg());
        Assert.assertTrue(setAttributes1.isSuccess(), setAttributes1.getErrorMsg());
        Assert.assertTrue(setAttributes2.isSuccess(), setAttributes2.getErrorMsg());
        
        sdb.getCollectionSpace(csName).createCollection(clName);
        
        DBCursor infoCur = sdb.getDomain(domainName).listCSInDomain();
        BSONObject info = infoCur.getNext();
        Assert.assertTrue(info.get("Name").equals(csName));
        infoCur.close();
        
        DBCursor snapCur = sdb.getSnapshot(Sequoiadb.SDB_SNAP_COLLECTIONSPACES, new BasicBSONObject("Name", csName), null, null);
        BSONObject csInfo = snapCur.getNext();
        Assert.assertEquals(csInfo.get("PageSize"), 16384, "check cs PageSize");
        Assert.assertEquals(csInfo.get("LobPageSize"), 32768, "check cs LobPageSize");
        snapCur.close();
    }
    
    @AfterClass
    public void tearDown(){
        sdb.dropCollectionSpace(csName);
        sdb.dropDomain(domainName);
        if(sdb != null){
            sdb.close();
        }
    }
    
    public class SetDomain extends SdbThreadBase{

        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                CollectionSpace cs = db.getCollectionSpace(csName);
                BSONObject options = new BasicBSONObject();
                options.put("Domain", domainName);
                cs.setDomain(options);
            }
        }
    }
    
    public class SetAttributes1 extends SdbThreadBase{
        
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
                options.put("LobPageSize", 32768);
                cs.setAttributes(options);
            }
        }
    }
}

