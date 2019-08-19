package com.sequoiadb.fulltext.killnode;

import java.util.ArrayList;
import java.util.List;
import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import com.sequoiadb.base.CollectionSpace;
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
 * @Description seqDB-15925:cs下存在多个cl，部分cl无效，全量同步clean cs节点清理无效集合对应的全文索引processor 
 * @date 2019/8/13
 */
public class Fulltext15925 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private GroupMgr groupMgr = null;
    private String groupName = "";
    private List<String> clNames = new ArrayList<String>();
    private List<String> indexNames = new ArrayList<String>();
    private List<DBCollection> collections = new ArrayList<DBCollection>();
    
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
        for(int i=0; i<10; i++){
            clNames.add("cl_15925_" + i);
            indexNames.add("indexName_15925_" + i);
            collections.add(sdb.getCollectionSpace(csName).createCollection(clNames.get(i), (BSONObject)JSON.parse("{Group: '"+ groupName +"'}")));
            if(i < 5){
                collections.get(i).createIndex(indexNames.get(i), "{a:'text'}", false, false);
            }
        }
    }
    
    @Test()
    public void Test() throws Exception{
        Node slave = sdb.getReplicaGroup(groupName).getSlave();
        String remoteHostName = slave.getHostName();
        Ssh ssh = new Ssh(remoteHostName, "root", SdbTestBase.rootPwd);
        String command = "lsof -iTCP:11790 -sTCP:LISTEN| sed '1d' | awk '{print $2}'";
        ssh.exec(command);  
        System.out.println("ssh.getStdout():"+ssh.getStdout());
        String pid = ssh.getStdout().substring(0, ssh.getStdout().length() - 1);
        command = "ls -l /proc/" + pid + "/exe | awk '{print $11}'" ;
        ssh.exec(command);
        command = ssh.getStdout().substring(0, ssh.getStdout().length() - 1) + "top";
        ssh.exec(command);
        
        for(int i=0; i<10; i++){
            FullTextDBUtils.insertData(collections.get(i), 10000);
        }

        int remotePort = slave.getPort();
        command = "lsof -iTCP:" + remotePort + " -sTCP:LISTEN | sed '1d' | awk '{print $2}'";
        ssh.exec(command);
        pid = ssh.getStdout().substring(0, ssh.getStdout().length() -1);
        command = "kill -9 " + pid;
        ssh.exec(command);
                
        for(int i=0; i<10; i++){
            FullTextDBUtils.insertData(collections.get(i), 10000);
        }

        CollectionSpace cs = sdb.getCollectionSpace(csName);
        for(int i = 0; i < 5; i++){
            FullTextDBUtils.dropCollection(cs, clNames.get(i));
        }
        
        collections.clear();
        for(int i = 0; i < 5; i++ ){
            collections.add(sdb.getCollectionSpace(csName).createCollection(clNames.get(i), (BSONObject)JSON.parse("{Group: '"+ groupName +"'}")));
            collections.get(i).createIndex(indexNames.get(i), "{a:'text'}", false, false);
            collections.get(i).insert("{a : 'Only one record'}");
        }
        
        
        command = "service sdbcm start";
        ssh.exec(command);
        
        Assert.assertTrue(groupMgr.checkBusinessWithLSN(600));
        for(int i = 0; i < 5; i++){
            Assert.assertTrue(FullTextUtils.isIndexCreated(collections.get(i), indexNames.get(i), 1));
        } 
    }
    
    @AfterClass()
    public void tearDown(){
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        for(int i = 0; i < 10; i++){
            FullTextDBUtils.dropCollection(cs, clNames.get(i));
        }
        sdb.close();
    }
}
