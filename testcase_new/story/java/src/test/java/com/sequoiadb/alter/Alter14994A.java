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
 * @FileName Alter14994.java
 * @Author luweikang
 * @Date 2019年3月11日
 */
public class Alter14994A extends SdbTestBase {
    
    private Sequoiadb sdb = null;
    private String csName = "cs_14994A";
    private String clName = "cl_14994A";
    private String domainName1 = "domain_14994_1";
    private String domainName2 = "domain_14994_2";
    
    @BeforeClass
    public void setUp(){
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        List<String> groupNames = CommLib.getDataGroupNames(sdb);
        if(groupNames.size() < 3){
            throw new SkipException("the group less than three");
        }
        
        BSONObject options1 = new BasicBSONObject();
        String[] domain1RG = new String[2];
        domain1RG[0] = groupNames.get(0);
        domain1RG[1] = groupNames.get(1);
        options1.put("Groups", domain1RG);
        sdb.createDomain(domainName1, options1);
        
        BSONObject options2 = new BasicBSONObject();
        String[] domain2RG = new String[2];
        domain2RG[0] = groupNames.get(0);
        domain2RG[1] = groupNames.get(2);
        options2.put("Groups", domain2RG);
        sdb.createDomain(domainName2, options2);
        
        sdb.createCollectionSpace(csName).createCollection(clName, new BasicBSONObject("Group", groupNames.get(0)));
    }
    
    @Test
    public void test(){
        SetDomain setDomain = new SetDomain();
        SetAttributes setAttributes = new SetAttributes();

        setDomain.start();
        setAttributes.start();
        
        Assert.assertTrue(setDomain.isSuccess(), setDomain.getErrorMsg());
        Assert.assertTrue(setAttributes.isSuccess(), setAttributes.getErrorMsg());
        
        DBCursor domain1Info = sdb.getDomain(domainName1).listCSInDomain();
        DBCursor domain2Info = sdb.getDomain(domainName2).listCSInDomain();
        if(domain1Info.hasNext()){
            BSONObject csInfo = domain1Info.getNext();
            Assert.assertTrue(csInfo.get("Name").equals(csName));
        }else if(domain2Info.hasNext()){
            BSONObject csInfo = domain2Info.getNext();
            Assert.assertTrue(csInfo.get("Name").equals(csName));
        }else{
            Assert.fail("cs set domain error");
        }
        domain1Info.close();
        domain2Info.close();
    }
    
    @AfterClass
    public void tearDown(){
        sdb.dropCollectionSpace(csName);
        sdb.dropDomain(domainName1);
        sdb.dropDomain(domainName2);
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
                options.put("Domain", domainName1);
                cs.setDomain(options);
            }
        }
    }
    
    public class SetAttributes extends SdbThreadBase{
        
        @Override
        public void exec() throws Exception {
            try(Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
                CollectionSpace cs = db.getCollectionSpace(csName);
                BSONObject options = new BasicBSONObject();
                options.put("Domain", domainName2);
                cs.setAttributes(options);
            }
        }
    }
}

