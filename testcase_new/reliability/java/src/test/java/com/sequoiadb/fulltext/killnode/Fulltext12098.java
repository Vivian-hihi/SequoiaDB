package com.sequoiadb.fulltext.killnode;

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
import com.sequoiadb.exception.ReliabilityException;
import com.sequoiadb.fault.KillNode;
import com.sequoiadb.fulltext.FullTextDBUtils;
import com.sequoiadb.fulltext.FullTextUtils;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;
import com.sequoiadb.task.TaskMgr;

/**
 * @Description seqDB-12098: 数据操作时备节点异常重启
 * @author xiaoni Zhao
 * @date 2019/8/10
 */
public class Fulltext12098 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private GroupMgr groupMgr = null;
    private DBCollection cl = null;
    private List<String> groupNames = null;
    private String groupName = "";
    private String clName = "cl_12098";
    private String indexName = "fullTextIndex_12098";

    @BeforeClass
    public void setUp() throws ReliabilityException {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        groupMgr = GroupMgr.getInstance();
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("isStandAlone() TRUE, STANDALONE MODE");
        }
        if (!groupMgr.checkBusiness(120)) {
            throw new SkipException("checkBusiness() FAIL, GROUP ERROR");
        }
        groupNames = CommLib.getDataGroupNames(sdb);
        groupName = groupNames.get(0);
        cl = sdb.getCollectionSpace(csName).createCollection(clName,
                (BSONObject) JSON.parse("{Group: '" + groupName + "'}"));
        cl.createIndex(indexName, "{a:'text'}", false, false);
        FullTextDBUtils.insertData(cl, 10);
        // TODO:此处插入记录量太少，建议至少1w条
    }

    @Test
    public void Test() throws Exception {
        NodeWrapper node = groupMgr.getGroupByName(groupName).getSlave();
        TaskMgr taskMgr = new TaskMgr();
        OperationTask operationTask = new OperationTask();
        FaultMakeTask faultMakeTask = KillNode.getFaultMakeTask(node, 60);
        // TODO:延迟启动时间不需要这么长，设置为1秒就够了
        taskMgr.addTask(operationTask);
        taskMgr.addTask(faultMakeTask);
        taskMgr.execute();

        Assert.assertTrue(taskMgr.isAllSuccess(), taskMgr.getErrorMsg());
        Assert.assertTrue(groupMgr.checkBusinessWithLSN(600));
        Assert.assertTrue(FullTextUtils.isIndexCreated(cl, indexName, 10));
    }

    @AfterClass
    public void tearDown() {
        CollectionSpace cs = sdb.getCollectionSpace(csName);
        FullTextDBUtils.dropCollection(cs, clName);
        sdb.close();
    }

    // TODO:需要同时增删改查并发线程处理，可以参考用例12084
    private class OperationTask extends OperateTask {
        private Sequoiadb db = null;
        private DBCollection cl = null;

        @Override
        public void exec() throws Exception {
            // TODO Auto-generated method stub
            db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(clName);
            try {
                for (int i = 0; i < 10; i++) {
                    cl.insert("{a:'fullText_" + i + "'}");
                    cl.update("{a:'fullText_" + i + "'}", "{$set: {a:'fullText_" + 10 + i + "'}}", null);
                    cl.delete("{a:'fullText_" + 10 + i + "'}");
                    cl.query();
                }
            } finally {
                db.close();
            }
        }
    }
}
