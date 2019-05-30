package com.sequoiadb.fulltext;

/**
 * @Description seqDB-12020:hash切分表中创建全文索引并切分后再插入记录
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

public class Fulltext12020 extends SdbTestBase {
    private Sequoiadb sdb = null;
    private DBCollection cl;
    private String clName = "ES_cl_12020";
    private String fullTextIndexName = "fullIndex12020";
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

        cl = sdb.getCollectionSpace(csName).createCollection(clName,
                (BSONObject) JSON.parse("{ShardingType:'hash', ShardingKey:{a:1}, Group:'" + srcGroup + "'}"));
        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
    }

    @Test
    public void test() throws Exception {
        cl.createIndex(fullTextIndexName, (BSONObject) JSON.parse("{a : 'text', b : 'text', c : 'text', d : 'text'}"),
                false, false);
        cappedName = FullTextDBUtils.getCappedName(cl, fullTextIndexName);
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullTextIndexName);
        cl.split(srcGroup, desGroup, 50);
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);
        Assert.assertEquals(FullTextDBUtils.getCLGroups(cl).size(), 2);
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullTextIndexName, FullTextUtils.INSERT_NUMS));
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
