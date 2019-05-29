package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

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
 * @Description seqDB-15799:主表执行truncate清空记录
 * @author yinzhen
 * @date 2018/11/28
 */
public class Fulltext15799 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection mainCL;
    private String mainCLName = "maincl15799";
    private String fullIndexName = "fullIndex15799";
    private Client esClient = null;
    private CollectionSpace cs = null;
    private String subCLName1 = "subcl15799A";
    private String subCLName2 = "subcl15799B";
    private List<String> esIndexNames01;
    private List<String> esIndexNames02;
    private String cappedCSName01;
    private String cappedCSName02;

    @BeforeClass
    public void setUp() {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        ArrayList<String> groupsName = CommLib.getDataGroupNames(sdb);
        if (groupsName.size() < 2) {
            throw new SkipException("Current environment less than tow groups");
        }
        cs = sdb.getCollectionSpace(SdbTestBase.csName);
        mainCL = cs.createCollection(mainCLName,
                (BSONObject) JSON.parse("{ShardingKey:{a:1}, ShardingType:'range', IsMainCL:true}"));
        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
    }

    @Test
    public void test() throws Exception {
        // 创建主子表，子表覆盖：普通表、切分表
        ArrayList<String> groupNames = CommLib.getDataGroupNames(sdb);
        DBCollection subCL1 = cs.createCollection(subCLName1);
        DBCollection subCL2 = cs.createCollection(subCLName2,
                (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'hash',Group:'" + groupNames.get(0) + "'}"));
        subCL2.split(groupNames.get(0), groupNames.get(1), 50);
        mainCL.attachCollection(csName + "." + subCLName1,
                (BSONObject) JSON.parse("{LowBound:{a:0}, UpBound:{a:114298}}"));
        mainCL.attachCollection(csName + "." + subCLName2,
                (BSONObject) JSON.parse("{LowBound:{a:114298}, UpBound:{a:200001}}"));

        // 创建全文索引，索引字段覆盖：子表分区键、子表普通字段
        mainCL.createIndex(fullIndexName, "{\"b\":\"text\", \"c\":\"text\"}", false, false);
        esIndexNames01 = FullTextDBUtils.getESIndexNames(subCL1, fullIndexName);
        esIndexNames02 = FullTextDBUtils.getESIndexNames(subCL2, fullIndexName);
        cappedCSName01 = FullTextDBUtils.getCappedName(subCL1, fullIndexName);
        cappedCSName02 = FullTextDBUtils.getCappedName(subCL2, fullIndexName);

        // 插入包含全文索引字段的记录
        insertData(FullTextUtils.INSERT_NUMS);
        Assert.assertTrue(
                FullTextUtils.isMainCLIndexCreated(esClient, mainCL, fullIndexName, FullTextUtils.INSERT_NUMS));

        mainCL.truncate();
        Assert.assertTrue(FullTextUtils.isFulltextRebuild(esClient, subCL1, fullIndexName));
        Assert.assertTrue(FullTextUtils.isFulltextRebuild(esClient, subCL2, fullIndexName));
        Assert.assertTrue(FullTextUtils.isMainCLIndexCreated(esClient, mainCL, fullIndexName, 0));
    }

    @AfterClass
    public void tearDown() throws InterruptedException {
        CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
        FullTextDBUtils.dropCollection(cs, mainCLName);
        FullTextDBUtils.dropCollection(cs, subCLName1);
        FullTextDBUtils.dropCollection(cs, subCLName2);
        esIndexNames01.addAll(esIndexNames02);
        List<String> cappedNames = new ArrayList<>();
        cappedNames.add(cappedCSName01);
        cappedNames.add(cappedCSName02);
        Assert.assertTrue(FullTextUtils.isIndexDeleted(sdb, esClient, esIndexNames01, cappedNames));
    }

    private void insertData(int insertNums) {
        List<BSONObject> records = new ArrayList<BSONObject>();
        Random random = new Random();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                BSONObject record = (BSONObject) JSON
                        .parse("{a:" + random.nextInt(200000) + ",b:'b" + i + "" + j + "', c:'c" + i + "" + j + "'}");
                records.add(record);
            }
            mainCL.insert(records);
            records.clear();
        }
    }
}