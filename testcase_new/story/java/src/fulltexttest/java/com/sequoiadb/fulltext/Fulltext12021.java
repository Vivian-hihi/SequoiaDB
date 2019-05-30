package com.sequoiadb.fulltext;

/**
 * @Description seqDB-12021: range切分表中创建全文索引并切分后再插入记录
 * @author xiaoni Zhao
 * @date 2018/11/23
 */
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
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

public class Fulltext12021 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private DBCollection cl;
    private String clName = "ES_cl_12021";
    private String fullTextIndexName = "fullIndex12021";
    private Client esClient = null;
    private String srcGroup = null;
    private String desGroup = null;
    private String cappedName = null;
    private String esIndexName = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        if (CommLib.OneGroupMode(sdb)) {
            throw new SkipException("current environment less than tow groups ");
        }

        List<String> groupsName = CommLib.getDataGroupNames(sdb);
        srcGroup = groupsName.get(0);
        desGroup = groupsName.get(1);

        cl = sdb.getCollectionSpace(csName).createCollection(clName, (BSONObject) JSON
                .parse("{ShardingType:'range', ShardingKey:{recordId:1, a:1}, Group:'" + srcGroup + "'}"));
        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
    }

    @Test
    public void test() throws Exception {
        cl.createIndex(fullTextIndexName, (BSONObject) JSON.parse("{a : 'text', b : 'text', c : 'text', d : 'text'}"),
                false, false);
        cappedName = FullTextDBUtils.getCappedName(cl, fullTextIndexName);
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullTextIndexName);
        cl.split(srcGroup, desGroup, (BSONObject) JSON.parse("{recordId:0}"), (BSONObject) JSON.parse("{recordId:10}"));
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
        checkData(desGroup, "{recordId:{$gte:0,$lt:10}}", 10);
        checkData(srcGroup, "{recordId:{$gte:" + 10 + ",$lt:" + FullTextUtils.INSERT_NUMS + "}}",
                ( FullTextUtils.INSERT_NUMS - 10 ));
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullTextIndexName, FullTextUtils.INSERT_NUMS));
    }

    public void checkData(String group, String matcher, int expectedCount) {
        Sequoiadb dataDb = null;
        DBCollection cl = null;
        long count;
        try {
            dataDb = sdb.getReplicaGroup(group).getMaster().connect();
            cl = dataDb.getCollectionSpace(csName).getCollection(clName);
            count = cl.getCount(matcher);
            Assert.assertEquals(count, expectedCount);
        } finally {
            dataDb.close();
        }
    }

    @AfterClass
    public void tearDown() throws Exception {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            FullTextDBUtils.dropCollection(cs, clName);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedName));
        } finally {
            sdb.close();
            esClient.close();
        }
    }
}
