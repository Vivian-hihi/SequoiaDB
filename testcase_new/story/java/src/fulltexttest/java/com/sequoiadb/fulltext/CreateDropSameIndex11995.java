package com.sequoiadb.fulltext;

import java.text.SimpleDateFormat;
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
import com.sequoiadb.testcommon.SdbThreadBase;
import org.elasticsearch.client.*;

/**
* FileName: CreateDropSameIndex11995.java
* test content: 反复重建删除同一个全文索引  
* @author liuxiaoxuan
    * @Date    2018.11.20
*/
public class CreateDropSameIndex11995 extends SdbTestBase{

      private Sequoiadb sdb = null;
      private CollectionSpace cs = null;
      private DBCollection cl = null;
      private String clName = "ES_11995";
      private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");

      private Client esClient = null;

      @BeforeClass
      public void setUp() {
           System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
           esClient = FullTextESUtils.createTransportClient(esHostName, Integer.parseInt(esServiceName));
           sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
           if (CommLib.isStandAlone(sdb)) {
                throw new SkipException("skip StandAlone");
           }

           // create cl 
           cs = sdb.getCollectionSpace(csName);
           cl = cs.createCollection(clName);
      }
	
      @AfterClass
      public void tearDown() {
           cs.dropCollection(clName);
      }

      @Test
      public void test() {
           // insert large datas
           boolean isSuccess = insertData(cl, FullTextUtils.INSERT_NUMS);
           if(!isSuccess) {
                throw new SkipException("---insert has an err:SEQUOIADBMAINSTREAM-3827---");
           }

           String textIndexName = "fulltext11995";
           BSONObject indexObj = new BasicBSONObject();
           indexObj.put("a", "text");

           List<String> esIndexNames = null;

           // loop create and drop fulltext while processing origin data
           int doTimes = 10;
           while(--doTimes > 0){
               cl.createIndex(textIndexName, indexObj, false, false);
               esIndexNames = FullTextDBUtils.getESIndexNames(sdb, csName, clName, textIndexName);
               FullTextDBUtils.dropFullTextIndex(cl, textIndexName);
               FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);
           }

           // create and drop fulltext while processing cappedcl data
           cl.createIndex(textIndexName, indexObj, false, false);
           int newInsertNums = 10000;
           InsertThread insertThread = new InsertThread(newInsertNums);
           DropIndexThread dropIdxThread = new DropIndexThread();
           insertThread.start();
           dropIdxThread.start();

           Assert.assertTrue(insertThread.isSuccess(), insertThread.getErrorMsg());
           Assert.assertTrue(dropIdxThread.isSuccess(), dropIdxThread.getErrorMsg());
           FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);

           // last time create index
           cl.createIndex(textIndexName, indexObj, false, false);
           // check consistency
           FullTextUtils.checkFullSyncToES(esClient, sdb, csName, clName, textIndexName, FullTextUtils.INSERT_NUMS + newInsertNums); 

           // last time drop index
           FullTextDBUtils.dropFullTextIndex(cl, textIndexName);
           // check fulltext deleted
           FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);
      }
	
      public boolean insertData(DBCollection cl, int insertNums) {
           List<BSONObject> insertObjs = new ArrayList<>();
           try {
                for(int i = 0; i < 100; i++){
                    for (int j = 0; j < insertNums/100; j++) {
                         insertObjs.add((BSONObject) JSON.parse("{a: 'test_11995_" + i*j + "', b: 'testb_" + i*j 
                                      + "', c: 'testc_" + i*j + "', d: 'testd_" + i*j + "', e: 'teste_" + i*j 
                                      + "', f: 'testf_" + i*j + "', g: 'testg_" + i*j + "', h: 'testh_" + i*j 
                                      + "', i: 'testi_" + i*j + "', j: 'testj_" + i*j + "', k: 'testk_" + i*j +"'}"));
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

      private class InsertThread extends SdbThreadBase{
	   
          int insertNums = 0;
 	
          public InsertThread(int insertNums){
              this.insertNums = insertNums;
          }

          @Override
          public void exec() throws Exception{
              Sequoiadb db = null;
              DBCollection cl = null;
              db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
              cl = db.getCollectionSpace(csName).getCollection(clName);
              // insert records in cappedCL               
              insertData(cl, insertNums);
              db.close();
         }
    }

    private class DropIndexThread extends SdbThreadBase{
        @Override
         public void exec() throws Exception{
              Sequoiadb db = null;
              DBCollection cl = null;
              db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
              cl = db.getCollectionSpace(csName).getCollection(clName);
              String textIndexName = "fulltext11995";
              // drop fulltext
              FullTextDBUtils.dropFullTextIndex(cl, textIndexName);          
        }

    }
}
