package com.sequoiadb.sessionaccess;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.SdbTestBase;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import java.util.ArrayList;
import java.util.List;

import static org.testng.Assert.assertEquals;
import static org.testng.Assert.fail;

/**
* @TestLink: seqDB-14145
* @describe: 设置会话访问属性指定多个instanceid，其中节点选择模式为随机选取
* @author wangkexin
* @Date   2019.02.16
* @version 1.00
*/
public class SessionAccess14145 extends SdbTestBase {
	private String clname = "cl14145";
    private Sequoiadb db;
    private DBCollection dbcl;
    private BasicBSONList nodes;
    private String rgName = "sessionAccessRG14145";

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
    public void test14145() {
    	List<Integer> instanceidList = new ArrayList<Integer>();
    	//TODO:1、获取instanceid已经有公共方法，这里没有必要在实现一次
        for (int i=0 ; i< nodes.size() ; i++) {
        	BasicBSONObject node = (BasicBSONObject)nodes.get(i);
        	instanceidList.add(Integer.parseInt(node.getString("instanceid")));
        }
        
    	int[] id = new int[]{instanceidList.get(0), instanceidList.get(1)};

        BSONObject options = new BasicBSONObject("PreferedInstance", id).append("PreferedInstanceMode", "random");
        db.setSessionAttr(options);
        String actualNodeName = CommLib.getActualDataNodeName(dbcl);
        //TODO:2、结果比较时验证访问节点，不要把测试点放在匹配instanceid上
        int actualId = CommLib.getInstanceidByNodeName(nodes, actualNodeName);
        if (actualId != id[0] && actualId != id[1]) {
            fail("actual:" + actualId + " expect: " + id[0] + " or " + id[1]);
        }

        //TODO:3、请注意规范命名，这里actual、expect一般认为是匹配的，实际意义却不一致
        BSONObject actual = db.getSessionAttr();
        BasicBSONList actualIdList= (BasicBSONList) actual.get("PreferedInstance");
        BasicBSONList expect=new BasicBSONList();
        for (int i : id) {
            expect.add(i);
        }
        assertEquals(actualIdList,expect);

        assertEquals(actual.get("PreferedInstanceMode"),"random");
    }
}
