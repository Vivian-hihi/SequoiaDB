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
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.CommLib;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fulltext.FullTextUtils;

/**
 * @Description seqDB-12079: 集合中存在全文索引，修改普通集合的副本数
 * @author xiaoni Zhao
 * @date 2018/12/3
 */
public class Fulltext12079 extends SdbTestBase {
    private String clName = "ES_cl_12079";
    private CollectionSpace cs;
    private DBCollection cl;
    private GroupMgr groupMgr = null;
    private Sequoiadb sdb = null;
    private String groupName;
    private String fullIndexName = "fullIndex12079";
    private int insertNum = 5000;

    @BeforeClass
    public void setUp() throws Exception {
        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            if (CommLib.isStandAlone(sdb)) {
                throw new SkipException("StandAlone environment!");
            }
            groupMgr = GroupMgr.getInstance();

            if (!groupMgr.checkBusiness()) {
                throw new SkipException("checkBusiness return false");
            }
            groupName = groupMgr.getAllDataGroupName().get(0);
            cs = sdb.getCollectionSpace(csName);
            cl = cs.createCollection(clName, (BSONObject) JSON.parse("{Group:'" + groupName + "', ReplSize : 1}"));
        } catch (ReliabilityException e) {
            throw e;
        }
    }

    @AfterClass
    public void tearDown() {
        try {
            cs.dropCollection(clName);
        } finally {
            sdb.close();
        }
    }

    @Test
    public void test() throws Exception {
        cl.createIndex(fullIndexName, "{'a':'text'}", false, false);
        insertData();
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, insertNum));
        cl.setAttributes((BSONObject) JSON.parse("{ReplSize : 3}"));
        NodeWrapper node = groupMgr.getGroupByName(groupName).getSlave();
        try {
            node.stop();
            cl.insert("{a:'text'}");
            throw new BaseException(1000, "INSERT_NEED_ERR");
        } catch (BaseException e) {
            if (e.getErrorCode() != -105 && e.getErrorCode() != -252) {
                throw e;
            }
            node.start();
        } finally {
            node.start();
        }
        Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true);
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, insertNum));

    }

    public void insertData() {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i < insertNum; i++) {
            BSONObject record = (BSONObject) JSON.parse("{a:'a" + i + "',b:'b" + i + "'}");
            records.add(record);
        }
        cl.insert(records);
    }

}
