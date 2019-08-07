package com.sequoiadb.fulltext.killnode;

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
 * @Description seqDB-12010:replSize设置不为1时插入/更新/删除记录
 * @author xiaoni Zhao
 * @date 2018/12/3
 */
public class Fulltext12010 extends SdbTestBase {
    private String clName = "ES_cl_12010";
    private CollectionSpace cs;
    private DBCollection cl;
    private GroupMgr groupMgr = null;
    private Sequoiadb sdb = null;
    private String groupName;
    private String fullIndexName = "fullIndex_12010";

    @BeforeClass()
    public void setUp() throws Exception {
        try {
            sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            if (CommLib.isStandAlone(sdb)) {
                throw new SkipException("StandAlone environment!");
            }
            groupMgr = GroupMgr.getInstance();

            if (!groupMgr.checkBusiness(20)) {
                throw new SkipException("checkBusiness return false");
            }
            groupName = groupMgr.getAllDataGroupName().get(0);
            cs = sdb.getCollectionSpace(csName);
            cl = cs.createCollection(clName, (BSONObject) JSON.parse("{Group:'" + groupName + "', ReplSize : 7}"));
        } catch (ReliabilityException e) {
            throw e;
        }
    }

    @AfterClass()
    public void tearDown() {
        try {
            cs.dropCollection(clName);
        } finally {
            sdb.close();
        }
    }

    @Test()
    public void test() throws Exception {
        cl.createIndex(fullIndexName, "{'a':'text'}", false, false);
        cl.insert("{a:'text'}");
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 1));
        NodeWrapper node = groupMgr.getGroupByName(groupName).getSlave();
        try {
            node.stop();
            try {
                cl.insert("{a:'insert'}");
                throw new BaseException(1000, "INSERT_NEED_ERR");
            } catch (BaseException e) {
                if (e.getErrorCode() != -105 && e.getErrorCode() != -252) {
                    throw e;
                }
            }

            try {
                cl.update(null, "{$set:{a:'update'}}", null, 0);
                throw new BaseException(1001, "UPDATE_NEED_ERR");
            } catch (BaseException e) {
                if (e.getErrorCode() != -105 && e.getErrorCode() != -252) {
                    throw e;
                }
            }

            try {
                cl.delete("");
                throw new BaseException(1002, "DELETE_NEED_ERR");
            } catch (BaseException e) {
                if (e.getErrorCode() != -105 && e.getErrorCode() != -252) {
                    throw e;
                }
            }
            node.start();
        } finally {
            node.start();
        }
        Assert.assertEquals(groupMgr.checkBusinessWithLSN(600), true);
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, fullIndexName, 1));
    }
}
