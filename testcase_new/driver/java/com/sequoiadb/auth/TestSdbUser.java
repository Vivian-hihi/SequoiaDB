package com.sequoiadb.auth;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterTest;
import org.testng.annotations.BeforeTest;
import org.testng.annotations.Test;
import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName:TestSdbUser7119,7120
 * beginTransaction ()；commit ()
 * @author chensiqin
 * @Date 2016-09-19
 * @version 1.00
 */

public class TestSdbUser extends SdbTestBase{
    private Sequoiadb sdb;
    private CollectionSpace cs;
    private DBCollection cl;
    private String clName = "cl7119";
    private String coordAddr;
    private String commCSName;
    
    @BeforeTest
    public void setUp() {
        try {
            this.coordAddr = SdbTestBase.coordUrl;
            this.commCSName = SdbTestBase.csName;
            System.out.println("the TestCase Name:" + this.getClass().getName() + 
                    ". the TestCase begin at:" + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
            this.sdb = new Sequoiadb(this.coordAddr, "", "");
            if (!this.sdb.isCollectionSpaceExist(this.commCSName)) {
                try{
                    this.cs = this.sdb.createCollectionSpace(this.commCSName); 
                } catch (BaseException e) {
                    Assert.assertEquals(-33, e.getErrorCode(), e.getMessage());
                }
            } else {
                this.cs = this.sdb.getCollectionSpace(this.commCSName);
            }
            if (this.cs.isCollectionExist(clName)) {
                this.cs.dropCollection(clName);
            }
            this.cl = this.cs.createCollection(clName);
        }catch (BaseException e) {
            System.out.println("Sequoiadb driver TestSdbUser7119 setUp error, error description:" + e.getMessage());
            Assert.fail("Sequoiadb driver TestSdbUser7119 setUp error, error description:" + e.getMessage());
        }
    }
    
    @Test
    public void test() {
        testSdbUser();
        testSdbUser7120();
    }
    
    public void testSdbUser() {
        if (!Util.isCluster(this.sdb)) {
            return ;
        }
        try {
            BSONObject bson = new BasicBSONObject();
            bson.put("name", "xiaoming");
            bson.put("age", 6);
            this.cl.insert(bson);
            
            this.sdb.createUser("admin", "admin");
            Sequoiadb sdb1 = new Sequoiadb(coordAddr, "admin", "admin");
            System.out.println("admin conn  " + this.sdb);
            sdb1.getCollectionSpace(commCSName).getCollection(clName).delete("{age:{$et:6}}");
            //user admin to insertdata
            BSONObject bsonObject = new BasicBSONObject();
            bsonObject.put("school", "university");
            bsonObject.put("score", 99.999);
            sdb1.getCollectionSpace(commCSName).getCollection(clName).insert(bsonObject);
            DBCursor cursor = sdb1.getCollectionSpace(commCSName).getCollection(clName).query();
            BSONObject actual = new BasicBSONObject();
            while(cursor.hasNext()) {
                actual = cursor.getNext();
            }
            cursor.close();
            Assert.assertEquals(actual, bsonObject);
            Node node = null;
            try {
                ReplicaGroup replicaGroup = this.sdb.getReplicaGroup("SYSCoord");
                node = replicaGroup.getNode(this.coordAddr);
                System.out.println("node ======= " +node); 
                node.connect("admin", "admin");
                node.disconnect();
            } catch (BaseException e) {
                Assert.fail("connect or disconnect node failed, errMsg:" + e.getMessage());
            } 
//            finally {
//               node.disconnect();
//            }
            //this.sdb.removeUser("admin", "admin");
        }catch (BaseException e) {
            System.out.println("Sequoiadb driver TestSdbUser testSdbUser error, error description:" + e.getMessage());
            Assert.fail("Sequoiadb driver TestSdbUser testSdbUser error, error description:" + e.getMessage());
        }
    }
    
    
    public void testSdbUser7120() {
        if (!Util.isCluster(this.sdb)) {
            return ;
        }
        try{
            for( int i = 0; i < 3; i++) {
                this.sdb.createUser("admin1", "");
            }
            Assert.fail("Sequoiadb driver TestSdbUser7120 repeat create the same user!");
        }catch (BaseException e) {
            //this.sdb.removeUser("admin1", "");
            Assert.assertEquals(e.getErrorCode(), -295);
        }
    }
    
    @AfterTest
    public void tearDown() {
        if (this.cs.isCollectionExist(clName)) {
            this.cs.dropCollection(clName);
        }
        try {
            this.sdb.removeUser("admin", "admin");
            this.sdb.removeUser("admin1", "");
        }catch (BaseException e) {
            if (-300 !=e.getErrorCode()) {
                Assert.assertTrue(false, "drop user, errMsg: " + e.getMessage());
            }
        }
        
        this.sdb.disconnect();
        System.out.println("the TestCase Name:" + this.getClass().getName() + 
                ". the TestCase end at:" + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss.SSS").format(new Date()));
    } 
}
 