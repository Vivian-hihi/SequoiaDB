package com.sequoiadb.fulltext;

import java.util.ArrayList;
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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;
import com.sequoiadb.utils.StringUtils;

/**
 * @Description seqDB-11989:range切分表中创建/删除全文索引
 * @author yinzhen
 * @date 2018/11/13
 */
public class Fulltext11989 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection cl;
    private String clName = "rangeTableIndex11989";
    private String fullIndexName = "fullIndex11989";
    private List<String> groupNames;
    private Client esClient = null;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        groupNames = CommLib.getDataGroupNames(sdb);
        if (groupNames.size() < 2) {
            throw new SkipException("Less than two groups!");
        }

        // 创建 range 切分表并切分
        CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
        cl = cs.createCollection(clName, (BSONObject) JSON
                .parse("{ShardingKey:{a:1}, ShardingType:'range', Group:'" + groupNames.get(0) + "'}"));
        cl.split(groupNames.get(0), groupNames.get(1), (BSONObject) JSON.parse("{a:'a0'}"),
                (BSONObject) JSON.parse("{a:'a1000'}"));
        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
    }

    @Test
    public void test() throws Exception {
        // 插入数据
        FullTextDBUtils.insertData(cl, FullTextUtils.INSERT_NUMS);

        // 创建全文索引，索引字段覆盖：分区键和非分区键
        cl.createIndex(fullIndexName, "{\"a\":\"text\",\"b\":\"text\",\"c\":\"text\",\"d\":\"text\",\"e\":\"text\"}",
                false, false);
        Assert.assertTrue(FullTextUtils.isIndexCreated(esClient, cl, fullIndexName, FullTextUtils.INSERT_NUMS));
        DBCollection cappedCL = FullTextDBUtils.getCappedCLs(cl, fullIndexName).get(0);
        Assert.assertFalse(cappedCL.query().hasNext());

        String cappedName = FullTextDBUtils.getCappedName(cl, fullIndexName);
        String esIndexName = FullTextDBUtils.getESIndexName(cl, fullIndexName);
        FullTextDBUtils.dropFullTextIndex(cl, fullIndexName);
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexName, cappedName));
        Assert.assertTrue(FullTextUtils.isCLDataConsistency(cl));
    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            FullTextDBUtils.dropCollection(cs, clName);
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        } finally {
            if (sdb != null) {
                sdb.close();
            }
            if (esClient != null) {
                esClient.close();
            }
        }
    }

    public void insertData(int insertNums) {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                BSONObject record = (BSONObject) JSON.parse("{a: 'test_range11989_" + i * j + "', b: '"
                        + StringUtils.getRandomString(32) + "', c: '" + StringUtils.getRandomString(64) + "', d: '"
                        + StringUtils.getRandomString(64) + "', e: '" + StringUtils.getRandomString(128) + "', g: '"
                        + StringUtils.getRandomString(128) + "'}");
                records.add(record);
            }
            cl.insert(records);
            records.clear();
        }
    }
}
