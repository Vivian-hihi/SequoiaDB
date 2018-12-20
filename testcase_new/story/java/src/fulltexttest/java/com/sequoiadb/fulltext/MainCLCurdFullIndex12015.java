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
* FileName: MainCLCurdFullIndex12015.java
* test content: 主子表中插入/更新/删除包含全文索引字段的记录   
* @author liuxiaoxuan
    * @Date    2018.11.27
*/
public class MainCLCurdFullIndex12015 extends SdbTestBase{

      private Sequoiadb sdb = null;
      private CollectionSpace cs = null;
      private DBCollection maincl = null;
      private String mainCLName = "ES_12015_maincl";
      private String subCLName1 = "ES_12015_subcl_1";
      private String subCLName2 = "ES_12015_subcl_2";

      private Client esClient = null;

      @BeforeClass
      public void setUp() {
           esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
           sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
           if (CommLib.isStandAlone(sdb)) {
                throw new SkipException("skip StandAlone");
           }

           // create maincl and subcls 
           cs = sdb.getCollectionSpace(csName);
           maincl = cs.createCollection(mainCLName, (BSONObject)JSON
                       .parse("{ShardingKey:{a:1}, ShardingType:'range', IsMainCL:true}"));
           cs.createCollection(subCLName1);
           cs.createCollection(subCLName2, (BSONObject)JSON
                     .parse("{ShardingKey:{a0:1}, ShardingType:'hash'}"));
      }
	
      @AfterClass
      public void tearDown() {
           cs.dropCollection(subCLName1);
           cs.dropCollection(subCLName2);
           cs.dropCollection(mainCLName);
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
          
           // create fulltext of maincl shardingkey and non-shardingkey
           String textIndexName = "fulltext12015";
           BSONObject indexObj = new BasicBSONObject();
           indexObj.put("a", "text");
           indexObj.put("a0", "text");
           indexObj.put("b", "text");
           indexObj.put("c", "text");
           indexObj.put("d", "text");
           indexObj.put("e", "text");
           indexObj.put("f", "text");
           maincl.createIndex(textIndexName, indexObj, false, false);

           // get esIndexNames of each subcl
           List<String> subCLFullNames = FullTextDBUtils.getSubCLNames(sdb, csName + "." + mainCLName);
           List<String> esIndexNames = new ArrayList<>();
           for(String subCLFullName: subCLFullNames){
               String subCSName = subCLFullName.split("\\.")[0];
               String subCLName = subCLFullName.split("\\.")[1];
               esIndexNames.addAll(FullTextDBUtils.getESIndexNames(sdb, subCSName, subCLName, textIndexName));
           }

           // insert
           boolean isSuccess = insertData(maincl, FullTextUtils.INSERT_NUMS);
           if(!isSuccess) {
                throw new SkipException("---insert has an err:SEQUOIADBMAINSTREAM-3827---");
           }
           FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, textIndexName, FullTextUtils.INSERT_NUMS);           

           // update, should change cl count
           update(maincl);
           insertData(maincl, 10000);
           FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, textIndexName, FullTextUtils.INSERT_NUMS + 10000);

           // delete
           remove(maincl);
           FullTextUtils.checkMainCLFullSyncToES(esClient, sdb, csName, mainCLName, textIndexName, (int)maincl.getCount());

           System.out.println("check fulltext of maincl shardingkey and non-shardingkey success!");
      }
	
      public boolean insertData(DBCollection cl, int insertNums) {
           List<BSONObject> insertObjs = new ArrayList<>();
           try {
                for(int i = 0; i < 100; i++){
                    for (int j = 0; j < insertNums/2/100; j++) {
                         insertObjs.add((BSONObject) JSON.parse("{a: 'testa " + i*j + "', a0:" + "'test_12051 " + i*j + "', b: '" + FullTextUtils.getRandomString(32)
                                      + "', c: '" + FullTextUtils.getRandomString(64) + "', d: '" + FullTextUtils.getRandomString(64)
                                      + "', e: '" + FullTextUtils.getRandomString(128) + "', f: '" + FullTextUtils.getRandomString(128) + "'}"));
                    }
                    for (int j = 0; j < insertNums/2/100; j++) {
                         insertObjs.add((BSONObject) JSON.parse("{a: 'zzza " + i*j + "', a0:" + "'test_12051 " + i*j + "', b: '" + FullTextUtils.getRandomString(32)
                                      + "', c: '" + FullTextUtils.getRandomString(64) + "', d: '" + FullTextUtils.getRandomString(64)
                                      + "', e: '" + FullTextUtils.getRandomString(128) + "', f: '" + FullTextUtils.getRandomString(128) + "'}"));
                    }
                    cl.insert(insertObjs, 0);
                    insertObjs.clear();
                }
	   } catch (BaseException e) {
                if(-321 == e.getErrorCode()) {
                     return false;
                }
                throw e;
           }

           return true;
      }

      private void update(DBCollection cl){
          BSONObject modifier = new BasicBSONObject();
          BSONObject value = new BasicBSONObject();
          BSONObject matcher = new BasicBSONObject();
          BSONObject subMatcher = new BasicBSONObject();
          value.put("a", "testa 99999");
          modifier.put("$set", value);
          subMatcher.put("$lt", "testa 10000");
          matcher.put("a", subMatcher);
          cl.update(matcher, modifier, null);
      }
 
      private void remove(DBCollection cl){
          BSONObject matcher = new BasicBSONObject();
          BSONObject subMatcher = new BasicBSONObject();
          subMatcher.put("$et", "testa 99999");
          matcher.put("a", subMatcher);
          cl.delete(matcher);
      } 
}
