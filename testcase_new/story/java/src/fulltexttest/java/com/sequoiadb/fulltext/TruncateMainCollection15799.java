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
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @Description seqDB-15799:无存量数据，插入记录
 * @author yinzhen
 * @date 2018/11/28
 */
public class TruncateMainCollection15799 extends SdbTestBase {
    private Sequoiadb sdb;
    private DBCollection mainCL;
    private String mainCLName = "truncateMainCollection15799";
    private String fullIndexName = "fullIndex15799";
    private Client esClient = null;
    private CollectionSpace cs = null;
    private String slaveCLName1 = "slaveCL115799";
    private String slaveCLName2 = "slaveCL215799";

    @BeforeClass
    public void setUp() {
        this.sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("StandAlone environment!");
        }
        ArrayList<String> groupsName = CommLib.getDataGroupNames(sdb);
        if (groupsName.size() < 2) {
            throw new SkipException("Current environment less than tow groups");
        }
        this.cs = sdb.getCollectionSpace(SdbTestBase.csName);
        this.mainCL = cs.createCollection(mainCLName,
                (BSONObject) JSON.parse("{ShardingKey:{a:1}, ShardingType:'range', IsMainCL:true}"));
        esClient = FullTextESUtils.createTransportClient(SdbTestBase.esHostName,
                Integer.parseInt(SdbTestBase.esServiceName));
    }

    @Test
    public void test() {
        // 创建主子表，子表覆盖：普通表、切分表
        cs.createCollection(slaveCLName1);
        cs.createCollection(slaveCLName2, (BSONObject) JSON.parse("{ShardingKey:{b:1}, ShardingType:'hash'}"));
        mainCL.attachCollection(csName + "." + slaveCLName1,
                (BSONObject) JSON.parse("{LowBound:{a:0}, UpBound:{a:114298}}"));
        mainCL.attachCollection(csName + "." + slaveCLName2,
                (BSONObject) JSON.parse("{LowBound:{a:114298}, UpBound:{a:200001}}"));

        // 创建全文索引，索引字段覆盖：子表分区键、子表普通字段
        this.mainCL.createIndex(fullIndexName, "{\"b\":\"text\", \"c\":\"text\"}", false, false);

        // 插入包含全文索引字段的记录
        this.insertData(FullTextUtils.INSERT_NUMS);
        FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, fullIndexName,
                FullTextUtils.INSERT_NUMS);

        while (true) {
            try {
                mainCL.truncate();
                // drop success
                break;
            } catch (BaseException e) {
                if (-147 == e.getErrorCode()) {
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e2) {
                        e2.printStackTrace();
                    }
                    continue;
                }
            }
        }

        try {
            Thread.sleep(8000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, fullIndexName, 0);

    }

    @AfterClass
    public void tearDown() {
        try {
            CollectionSpace cs = sdb.getCollectionSpace(SdbTestBase.csName);
            cs.dropCollection(this.mainCLName);
            cs.dropCollection(this.slaveCLName1);
            cs.dropCollection(this.slaveCLName2);
        } catch (BaseException e) {
            Assert.assertEquals(e.getErrorCode(), -23, e.getMessage());
        } finally {
            if (sdb != null) {
                sdb.close();
            }
        }
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
            this.mainCL.insert(records);
            records.clear();
        }
    }
}