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

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.utils.FullTextDBUtils;
import com.sequoiadb.utils.FullTextESUtils;
import com.sequoiadb.utils.FullTextUtils;

/**
 * @testcase seqDB-18260:增量同步时，删除原始集合
 * @date 2019-4-26
 * @author yinzhen
 *
 */
public class FullText18260 extends SdbTestBase {
    private static final String CLNAME = "cl18260";
    private Sequoiadb sdb;
    private DBCollection cl;
    private String fullIdxName = "idx18260";
    private Client esClient;
    private String groupName;

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
                (BSONObject) JSON.parse("{'Group':'" + groupName + "'}"));
    }

    @Test
    public void test() throws InterruptedException {
        cl.createIndex(fullIdxName, "{'a':'text','b':'text','c':'text','d':'text','e':'text','f':'text'}", false,
                false);
        insertData(cl, FullTextUtils.INSERT_NUMS);
        List<String> esIndexNames = FullTextDBUtils.getESIndexNames(sdb, csName, CLNAME, fullIdxName);
        String cappedCLName = FullTextDBUtils.getCappedCLName(cl, fullIdxName);
        Assert.assertTrue(FullTextESUtils.isExistIndexInES(esClient, esIndexNames.get(0)));
        sdb.getCollectionSpace(csName).dropCollection(CLNAME);
        FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);
    }

    @AfterClass
    public void tearDown() {
        sdb.close();
    }

    private void insertData(DBCollection cl, int insertNums) {
        List<BSONObject> records = new ArrayList<BSONObject>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                BSONObject record = (BSONObject) JSON.parse("{a: 'test_18260_" + i * j + "', b: '"
                        + FullTextUtils.getRandomString(32) + "', c: '" + FullTextUtils.getRandomString(64) + "', d: '"
                        + FullTextUtils.getRandomString(64) + "', e: '" + FullTextUtils.getRandomString(128) + "', f: '"
                        + FullTextUtils.getRandomString(128) + "'}");
                records.add(record);
            }
            cl.insert(records);
            records.clear();
        }
    }
}