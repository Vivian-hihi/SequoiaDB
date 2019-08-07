package com.sequoiadb.fulltext.restartnode;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.fulltext.FullTextUtils;

/**
 * @Description seqDB-14405:正常启动DB主节点不影响全文索引功能
 * @author yinzhen
 * @date 2018/11/15
 */
public class Fulltext14405 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection cl;
    private String clName = "restartMasterNode14405";
    private String fullIndexName = "fullIndex14405";
    private List<String> groupNames;

    @BeforeClass
    public void setUp() {
        this.sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        this.groupNames = CommLib.getDataGroupNames(sdb);
        CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
        this.cl = cs.createCollection(clName, (BSONObject) JSON.parse("{Group:'" + this.groupNames.get(0) + "'}"));
    }

    @Test
    public void test() throws Exception {
        Sequoiadb preMasterNodeDB = null;
        Sequoiadb currentMasterNodeDB = null;
        try {// 创建全文索引，插入记录
            this.cl.createIndex(fullIndexName, "{\"a\":\"text\"}", false, false);
            this.insertData();
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 500000));

            // get masterNode
            Node preMasterNode = this.sdb.getReplicaGroup(this.groupNames.get(0)).getMaster();
            String preMasterHostName = preMasterNode.getHostName();
            int preMasterSvcName = preMasterNode.getPort();

            // restart masterNode
            preMasterNode.stop();
            preMasterNode.start();
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 500000));

            // get old and new masterNode
            preMasterNodeDB = new Sequoiadb(preMasterHostName, preMasterSvcName, "", "");
            currentMasterNodeDB = this.sdb.getReplicaGroup(this.groupNames.get(0)).getMaster().connect();

            // insert
            this.insertData();
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 1000000));

            // update
            this.cl.update("{a:'a01'}", "{'$set':{a:'helloworld'}}", "{'':null}");
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 1000000));

            // delete
            this.cl.delete("{a:'a11'}");
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 999998));

            // query previous masterNode
            BSONObject matcher = new BasicBSONObject();
            matcher.put("a", "a22");
            DBCursor cursor = preMasterNodeDB.getCollectionSpace(SdbTestBase.csName).getCollection(this.clName)
                    .query(matcher, null, null, null);
            int count = 0;
            while (cursor.hasNext()) {
                cursor.getNext();
                count++;
            }
            cursor.close();
            Assert.assertEquals(2, count);

            // query current masterNode
            cursor = currentMasterNodeDB.getCollectionSpace(SdbTestBase.csName).getCollection(this.clName)
                    .query(matcher, null, null, null);
            count = 0;
            while (cursor.hasNext()) {
                cursor.getNext();
                count++;
            }
            cursor.close();
            Assert.assertEquals(2, count);

            // check full sync to ES
            Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 999998));
        } finally {
            preMasterNodeDB.close();
            currentMasterNodeDB.close();
        }

    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
            cs.dropCollection(this.clName);
        } catch (BaseException e) {
            Assert.fail(e.getMessage() + "\r\n" + this.getKeyStack(e, this));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
        }
    }

    public void insertData() {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 5000; j++) {
                BSONObject record = (BSONObject) JSON.parse("{a:'a" + i + "" + j + "',g:'g" + i + "" + j + "'}");
                records.add(record);
            }
            this.cl.insert(records);
            records.clear();
        }
    }

    public String getKeyStack(Exception e, Object classObj) {
        StringBuffer stackBuffer = new StringBuffer();
        StackTraceElement[] stackElements = e.getStackTrace();
        for (int i = 0; i < stackElements.length; i++) {
            if (stackElements[i].toString().contains(classObj.getClass().getName())) {
                stackBuffer.append(stackElements[i].toString()).append("\r\n");
            }
        }
        String str = stackBuffer.toString();
        if (str.length() >= 2) {
            return str.substring(0, str.length() - 2);
        } else {
            return str;
        }
    }
}
