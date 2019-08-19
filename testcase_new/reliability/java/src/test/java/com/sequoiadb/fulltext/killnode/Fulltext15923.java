package com.sequoiadb.fulltext.killnode;

import java.util.List;
import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fulltext.FullTextDBUtils;
import com.sequoiadb.fulltext.FullTextUtils;
/**
 * @Description seqDB-15923: cs下存在1个cl，全量同步clean cs阶段清理无效集合对应的全文索引processor 
 * @date 2019/8/13
 */
public class Fulltext15923 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private GroupMgr groupMgr = null;
    private DBCollection cl = null;
    private String groupName = "";
    private String csName = "cs_15923";
    private String clName = "cl_15923";
    private String indexName = "fullTextIndex_15923";
    
    @BeforeClass()
    public void setUp() throws ReliabilityException{
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        groupMgr = GroupMgr.getInstance();
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("isStandAlone() TRUE, STANDALONE MODE");
        }
        if (!groupMgr.checkBusiness(120)) {
            throw new SkipException("checkBusiness() FAIL, GROUP ERROR");
        }
        List<String> groupNames = CommLib.getDataGroupNames(sdb);
        groupName = groupNames.get(0);
        cl = sdb.createCollectionSpace(csName).createCollection(clName, (BSONObject)JSON.parse("{Group: '"+ groupName +"'}"));
        cl.createIndex(indexName, "{a:'text'}", false, false);
    }
    
    @Test()
    public void Test() throws Exception{
        Node slave = sdb.getReplicaGroup(groupName).getSlave();
        String remoteHostName = slave.getHostName();Ssh ssh = new Ssh(remoteHostName, "root", SdbTestBase.rootPwd);
        String command = "lsof -iTCP:11790 -sTCP:LISTEN| sed '1d' | awk '{print $2}'";
        ssh.exec(command);  
        System.out.println("ssh.getStdout():"+ssh.getStdout());
        String pid = ssh.getStdout().substring(0, ssh.getStdout().length() - 1);
        command = "ls -l /proc/" + pid + "/exe | awk '{print $11}'" ;
        ssh.exec(command);
        command = ssh.getStdout().substring(0, ssh.getStdout().length() - 1) + "top";
        ssh.exec(command);
        
        FullTextDBUtils.insertData(cl, 10000);

        int remotePort = slave.getPort();
        command = "lsof -i TCP:" + remotePort + " -sTCP:LISTEN| sed '1d' | awk '{print $2}'";
        ssh.exec(command);
        pid = ssh.getStdout().substring(0, ssh.getStdout().length() -1);
        command = "kill -9 " + pid;
        ssh.exec(command);
        
        FullTextDBUtils.insertData(cl, 10000);

        FullTextDBUtils.dropCollectionSpace(sdb, csName);;
        
        cl = sdb.createCollectionSpace(csName).createCollection(clName, (BSONObject)JSON.parse("{Group: '"+ groupName +"'}"));
        cl.createIndex(indexName, "{a:'text'}", false, false);
        cl.insert("{a : 'Only one record'}");
        
        command = "service sdbcm start";
        ssh.exec(command);
        
        Assert.assertTrue(groupMgr.checkBusinessWithLSN(600));
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, indexName, 1));
    }
    
    @AfterClass()
    public void tearDown(){
        FullTextDBUtils.dropCollectionSpace(sdb, csName);
        sdb.close();
    }
}
