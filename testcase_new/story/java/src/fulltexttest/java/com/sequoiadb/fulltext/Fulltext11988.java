package com.sequoiadb.fulltext;

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

/**
 * @Description seqDB-11988:hash切分表加入域使用自动切分，创建/删除全文索引
 * @author yinzhen
 * @date 2018/11/13
 */
public class Fulltext11988 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection cl;
    private String clName = "hashTableIndex11988";
    private String fullIndexName = "fullIndex11988";
    private List<String> groupNames;
    private Client esClient = null;
    private String csName = "cs11988";
    private String doMainName = "doMain11988";
    private String esIndexName;
    private String cappedCLName;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        groupNames = CommLib.getDataGroupNames(sdb);
        if (CommLib.OneGroupMode(sdb)) {
            throw new SkipException("ONE GROUP MODE");
        }

        if (sdb.isCollectionSpaceExist(csName)) {
            sdb.dropCollectionSpace(csName);
        }
        if (sdb.isDomainExist(doMainName)) {
            sdb.dropDomain(doMainName);
        }

        // hash切分表加入域使用自动切分
        sdb.createDomain(doMainName,
                (BSONObject) JSON.parse("{Groups:['" + groupNames.get(0) + "', '" + groupNames.get(1) + "']}"));
        CollectionSpace cs = sdb.createCollectionSpace(csName, (BSONObject) JSON.parse("{Domain:'doMain11988'}"));
        cl = cs.createCollection(clName,
                (BSONObject) JSON.parse("{ShardingKey:{a:1}, ShardingType:'hash', AutoSplit:true}"));
        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
    }

    @Test
    public void test() throws Exception {
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);

        // 创建全文索引，索引字段覆盖：分区键、非分区键
        String indexKey = "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\",\"g\":\"text\"}";
        cl.createIndex(fullIndexName, indexKey, false, false);
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIndexName, FullTextUtils.INSERT_NUMS));
        esIndexName = FullTextDBUtils.getESIndexName(cl, fullIndexName);
        cappedCLName = FullTextDBUtils.getCappedName(cl, fullIndexName);

        // 删除索引
        FullTextDBUtils.dropFullTextIndex(cl, fullIndexName);
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
    }

    @AfterClass
    public void tearDown() {
        try {
            FullTextDBUtils.dropCollectionSpace(sdb, csName);
            sdb.dropDomain(doMainName);
            Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedCLName));
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }
}
