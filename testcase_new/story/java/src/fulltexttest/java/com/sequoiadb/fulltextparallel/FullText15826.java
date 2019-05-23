package com.sequoiadb.fulltextparallel;

import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.elasticsearch.client.Client;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.threadexecutor.ThreadExecutor;
import com.sequoiadb.threadexecutor.annotation.ExecuteOrder;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @testcase seqDB-15826:删除全文索引与创建/删除普通索引并发
 * @date 2019-4-30
 * @author yinzhen
 *  //TODO:用例注释需要按规范填写字段 @FileName  @Author @Date .....
 */
public class FullText15826 extends SdbTestBase {
    private static final String CLNAME = "cl15826";
    private ThreadExecutor thExecutor = new ThreadExecutor(600000);
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx15826";
    private Client esClient;
    private String groupName;
    private String cappedCLName;
    private String esIndexName;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("STANDALONE MODE");
        }

        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
        groupName = CommLib.getDataGroupNames(sdb).get(0);
        cl = sdb.getCollectionSpace(csName).createCollection(CLNAME,
                (BSONObject) JSON.parse("{Group:'" + groupName + "'}"));//TODO：没切分不需要指定组吧？
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text'}", false, false);
    }

    @Test
    public void test() throws Exception {
        thExecutor.addWorker(new DropFullIdx());
        thExecutor.addWorker(new CreateIdx());//TODO：没有检查结果
//TODO:缺少测试点，见文本用例： b.创建普通索引；c.删除普通索引；字段覆盖：重叠、不重叠。   即：创建和删除的普通索引的索引字段需要覆盖：跟全文索引键 重叠、不重叠
        thExecutor.run();
        thExecutor.display();//TODO：不需要，没配置log4j.properties，日志打印不出来
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            cs.dropCollection(CLNAME);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            sdb.close();
        }
    }

    class DropFullIdx {
        private Sequoiadb db;
        private DBCollection cl;

        private DropFullIdx() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "删除全文索引")
        private void dropFullIdx() {
            esIndexName = FullTextDBUtils.getESIndexName(cl, fullIdxName);
            cappedCLName = FullTextDBUtils.getCappedName(cl, fullIdxName);//TODO:这2步建议放到setUp，避免占用并发运行时间
            cl.dropIndex(fullIdxName);
            Assert.assertFalse(cl.isIndexExist(fullIdxName));  //TODO：这个不需要判断，setp2已经判断了
        }

        @ExecuteOrder(step = 2, desc = "主备节点上索引信息及固定集合信息一致，ES同步的索引数据正确")
        private void checkIndex() throws InterruptedException {
            try {
                Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
                Assert.assertTrue(FullTextUtils.isCLDataConsistency(cl));
                checkFullIdx();
            } finally {
                db.close();//TODO:连接建议另起一个方法关闭
            }
        }
    }

    class CreateIdx {
        private Sequoiadb db;
        private DBCollection cl;

        private CreateIdx() {
            db = new Sequoiadb(coordUrl, "", "");
            cl = db.getCollectionSpace(csName).getCollection(CLNAME);
        }

        @ExecuteOrder(step = 1, desc = "创建普通索引，删除普通索引")
        private void operatorIdx() {
            try {
                cl.createIndex("idx1", "{'d':1, 'e':1}", false, false);
                cl.createIndex("idx2", "{'c':1, 'f':1}", false, false);
                Assert.assertTrue(cl.isIndexExist("idx1"));
                Assert.assertTrue(cl.isIndexExist("idx2"));
                cl.dropIndex("idx1");
                cl.dropIndex("idx2");
                Assert.assertFalse(cl.isIndexExist("idx1"));
                Assert.assertFalse(cl.isIndexExist("idx2"));
            } finally {
                db.close();
            }
        }
    }

    private void checkFullIdx() throws InterruptedException {
        List<String> nodeAddrs = CommLib.getNodeAddress(sdb, groupName);
        for (String nodeAddr : nodeAddrs) {
            Sequoiadb data = null;
            try {
                data = new Sequoiadb(nodeAddr, "", "");
                DBCollection cl = data.getCollectionSpace(csName).getCollection(CLNAME);
                int doTimes = 0;
                while (cl.isIndexExist(fullIdxName)) {
                    doTimes++;
                    Thread.sleep(100);
                    Assert.assertNotEquals(doTimes, 600);
                }
            } finally {
                data.close();
            }
        }
    }
}