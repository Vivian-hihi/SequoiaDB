package com.sequoiadb.sessionaccess;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.Random;

import static org.testng.Assert.assertEquals;

/**
* @TestLink: seqDB-14142
* @describe: 设置会话访问属性，指定instanceid和timeout属性
* @author wangkexin
* @Date   2019.02.16
* @version 1.00
*/

public class SessionAccess14142 extends SdbTestBase {
    private String clname = "cl14142";
    private Sequoiadb db;
    private DBCollection dbcl;
    private Random random = new Random();
    private BasicBSONList nodes;
    private String rgName = "sessionAccessRG14142";

    @BeforeClass
    public void setup() {
        db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        nodes = CommLib.createRG(db, rgName);
        BSONObject options = new BasicBSONObject("Group", rgName);
        options.put("ReplSize", -1);
        dbcl = db.getCollectionSpace(SdbTestBase.csName).createCollection(clname, options);
        CommLib.insertRecords(dbcl);
    }

    @AfterClass
    public void teardown() throws InterruptedException {
        db.getCollectionSpace(SdbTestBase.csName).dropCollection(clname);
        db.removeReplicaGroup(rgName);
        db.close();
    }

    @Test
    public void test14142() {
        int expectid = getRandomInstanceid();
        BasicBSONObject options = new BasicBSONObject("PreferedInstance", expectid).append("Timeout", 200L);
        db.setSessionAttr(options);
        //TODO:1、create lob走主节点，这里的instanceid不一定是主节点，如果是备节点，无法覆盖测试点。
        try {
            DBLob lob = dbcl.createLob();
            lob.write(new byte[1024 * 1024 * 10]);
            lob.close();
        } catch (BaseException e) {
        	Assert.assertEquals(e.getErrorCode(), -13);
        }
      //TODO:1、同todo1：如果是备节点，无法覆盖测试点。
        options.put("Timeout", 20000L);
        db.setSessionAttr(options);
        DBLob lob = dbcl.createLob();
        lob.write(new byte[1024 * 1024 * 10]);
        lob.close();
        db.setSessionAttr(options);
        String actualNodeName = CommLib.getActualDataNodeName(dbcl);
        //TODO:2、这个测试点没有意义，已经选取了instanceid，然后使用了又比较instanceid是否正确，这个点是测试什么？如果比较访问节点直接获取访问节点比较
        assertEquals(CommLib.getInstanceidByNodeName(nodes, actualNodeName), expectid);
        //TODO:3、变量命名建议符合实际用意，如果不好命名可以加描述信息
        BSONObject actual = db.getSessionAttr();
        options.append("PreferedInstanceMode", "random");
        //TODO:4、这里比较结果还包括insertid和timeout参数
        assertEquals(actual, options);
    }
    
    private int getRandomInstanceid() {
    	BasicBSONObject randomNode = (BasicBSONObject)nodes.get(random.nextInt(nodes.size()));
        return Integer.parseInt(randomNode.getString("instanceid"));
    }
    
}
