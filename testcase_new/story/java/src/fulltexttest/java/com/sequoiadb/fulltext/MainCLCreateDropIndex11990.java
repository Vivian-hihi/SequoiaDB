package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import org.elasticsearch.client.*;

/**
 * FileName: MainCLCreateDropIndex11990.java test content: 主子表中创建/删除全文索引
 * 
 * @author liuxiaoxuan
 * @Date 2018.11.27
 */
public class MainCLCreateDropIndex11990 extends SdbTestBase {

    private Sequoiadb sdb = null;
    private CollectionSpace cs = null;
    private DBCollection maincl = null;
    private String mainCLName = "ES_11990_maincl";
    private String subCLName1 = "ES_11990_subcl_1";
    private String subCLName2 = "ES_11990_subcl_2";
    private String srcGroupName = "";
    private String destGroupName = "";
    private Client esClient = null;

    @BeforeClass
    public void setUp() {
        esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (CommLib.isStandAlone(sdb)) {
            throw new SkipException("skip StandAlone");
        }
        ArrayList<String> groupsName = CommLib.getDataGroupNames(sdb);
        if (groupsName.size() < 2) {
            throw new SkipException("current environment less than tow groups ");
        }

        srcGroupName = groupsName.get(0);
        destGroupName = groupsName.get(1);
        // create maincl and subcls
        cs = sdb.getCollectionSpace(csName);
        maincl = cs.createCollection(mainCLName,
                (BSONObject) JSON.parse("{ShardingKey:{a:1}, ShardingType:'range', IsMainCL:true}"));
        cs.createCollection(subCLName1);
        DBCollection subcl = cs.createCollection(subCLName2,
                (BSONObject) JSON.parse("{ShardingKey:{a0:1}, ShardingType:'hash', Group:'" + srcGroupName + "'}"));
        subcl.split(srcGroupName, destGroupName, 50);
    }

    @AfterClass
    public void tearDown() {
        FullTextDBUtils.dropCollection(cs, subCLName1);
        FullTextDBUtils.dropCollection(cs, subCLName2);
        FullTextDBUtils.dropCollection(cs, mainCLName);
        sdb.close();
        esClient.close();
    }

    @Test
    public void test() {
        // attach CL
        BSONObject options1 = (BSONObject) JSON.parse("{LowBound:{a:'testa'}, UpBound:{a:'testa 999999'}}");
        BSONObject options2 = (BSONObject) JSON.parse("{LowBound:{a:'zzza'}, UpBound:{a:'zzza 999999'}}");
        maincl.attachCollection(csName + "." + subCLName1, options1);
        maincl.attachCollection(csName + "." + subCLName2, options2);

        // insert one group
        insertDataOneGroup(maincl, FullTextUtils.INSERT_NUMS);

        // create fulltext of maincl shardingkey
        String textIndexName = "fulltext11990";
        BSONObject indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        maincl.createIndex(textIndexName, indexObj, false, false);

        // get esIndexNames of each subcl
        List<String> subCLFullNames = FullTextDBUtils.getSubCLNames(sdb, csName + "." + mainCLName);
        List<String> esIndexNames = new ArrayList<>();
        for (String subCLFullName : subCLFullNames) {
            String subCSName = subCLFullName.split("\\.")[0];
            String subCLName = subCLFullName.split("\\.")[1];
            esIndexNames.addAll(FullTextDBUtils.getESIndexNames(sdb, subCSName, subCLName, textIndexName));
        }

        FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, textIndexName,
                FullTextUtils.INSERT_NUMS);
        FullTextUtils.checkMainCLConsistency(sdb, csName + "." + mainCLName);
        System.out.println("check fulltext of maincl shardingkey success when datas in one group!");

        // create fulltext of subcl shardingkey and non-shardingkey
        FullTextDBUtils.dropFullTextIndex(maincl, textIndexName);
        FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);

        indexObj = new BasicBSONObject();
        indexObj.put("a0", "text");
        indexObj.put("b", "text");
        indexObj.put("c", "text");
        indexObj.put("d", "text");
        indexObj.put("e", "text");
        indexObj.put("f", "text");
        maincl.createIndex(textIndexName, indexObj, false, false);

        FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, textIndexName,
                FullTextUtils.INSERT_NUMS);
        FullTextUtils.checkMainCLConsistency(sdb, csName + "." + mainCLName);
        System.out.println("check fulltext of subcl shardingkey and non-shardingkey success when datas in one group!");

        FullTextDBUtils.dropFullTextIndex(maincl, textIndexName);
        FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);
        maincl.truncate();

        // insert more groups
        insertDataMoreGroups(maincl, FullTextUtils.INSERT_NUMS);

        // create fulltext of maincl shardingkey
        indexObj = new BasicBSONObject();
        indexObj.put("a", "text");
        maincl.createIndex(textIndexName, indexObj, false, false);

        FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, textIndexName,
                FullTextUtils.INSERT_NUMS);
        FullTextUtils.checkMainCLConsistency(sdb, csName + "." + mainCLName);
        System.out.println("check fulltext of maincl shardingkey success when datas in more groups!");

        // create fulltext of subcl shardingkey and non-shardingkey
        FullTextDBUtils.dropFullTextIndex(maincl, textIndexName);
        FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);

        indexObj = new BasicBSONObject();
        indexObj.put("a0", "text");
        indexObj.put("b", "text");
        indexObj.put("c", "text");
        indexObj.put("d", "text");
        indexObj.put("e", "text");
        indexObj.put("f", "text");
        maincl.createIndex(textIndexName, indexObj, false, false);

        FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, textIndexName,
                FullTextUtils.INSERT_NUMS);
        FullTextUtils.checkMainCLConsistency(sdb, csName + "." + mainCLName);
        System.out
                .println("check fulltext of subcl shardingkey and non-shardingkey success when datas in more groups!");

        FullTextDBUtils.dropFullTextIndex(maincl, textIndexName);
        FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);
    }

    public void insertDataOneGroup(DBCollection cl, int insertNums) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 100; j++) {
                insertObjs.add((BSONObject) JSON.parse("{a: 'testa " + i * j + "', a0:" + "'test_11990 " + i * j
                        + "', b: '" + FullTextUtils.getRandomString(32) + "', c: '" + FullTextUtils.getRandomString(64)
                        + "', d: '" + FullTextUtils.getRandomString(64) + "', e: '" + FullTextUtils.getRandomString(128)
                        + "', f: '" + FullTextUtils.getRandomString(128) + "'}"));
            }
            cl.insert(insertObjs, 0);
            insertObjs.clear();
        }
    }

    public void insertDataMoreGroups(DBCollection cl, int insertNums) {
        List<BSONObject> insertObjs = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < insertNums / 2 / 100; j++) {
                insertObjs.add((BSONObject) JSON.parse("{a: 'testa " + i * j + "', a0:" + "'test_11990 " + i * j
                        + "', b: '" + FullTextUtils.getRandomString(32) + "', c: '" + FullTextUtils.getRandomString(64)
                        + "', d: '" + FullTextUtils.getRandomString(64) + "', e: '" + FullTextUtils.getRandomString(128)
                        + "', f: '" + FullTextUtils.getRandomString(128) + "'}"));
            }
            for (int j = 0; j < insertNums / 2 / 100; j++) {
                insertObjs.add((BSONObject) JSON.parse("{a: 'zzza " + i * j + "', a0:" + "'test_11990 " + i * j
                        + "', b: '" + FullTextUtils.getRandomString(32) + "', c: '" + FullTextUtils.getRandomString(64)
                        + "', d: '" + FullTextUtils.getRandomString(64) + "', e: '" + FullTextUtils.getRandomString(128)
                        + "', f: '" + FullTextUtils.getRandomString(128) + "'}"));
            }
            cl.insert(insertObjs, 0);
            insertObjs.clear();
        }
    }
}
