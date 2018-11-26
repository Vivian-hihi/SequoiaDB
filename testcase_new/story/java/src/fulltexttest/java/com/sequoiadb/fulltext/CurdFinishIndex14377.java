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

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import org.elasticsearch.client.*;

/**
* FileName: CurdFinishIndex14377.java
* test content: 已处理完固定集合中记录，插入/修改/删除/查询集合中的记录  
* @author liuxiaoxuan
    * @Date    2018.11.21
*/
public class CurdFinishIndex14377 extends SdbTestBase{

      private Sequoiadb sdb = null;
      private CollectionSpace cs = null;
      private DBCollection cl = null;
      private String clName = "ES_14377";
      private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");

      private Client esClient = null;

      @BeforeClass
      public void setUp() {
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
           // create fulltext
           String textIndexName = "fulltext14377";
           BSONObject indexObj = new BasicBSONObject();
           indexObj.put("a", "text");
           cl.createIndex(textIndexName, indexObj, false, false);

           List<String> esIndexNames = FullTextDBUtils.getESIndexNames(sdb, csName, clName, textIndexName);

           int insertNums = 500000; //50w
           boolean isSuccess = insertData(cl, insertNums);
           if(!isSuccess) {
                throw new SkipException("---insert has an err:SEQUOIADBMAINSTREAM-3827---");
           }

           // check consistency before insert/update/delete
           FullTextUtils.checkFullSyncToES(esClient, sdb, csName, clName, textIndexName, insertNums);

           // insert/update/delete 
           insertData(cl, 50000);
           updateData(cl);
           removeData(cl); 

           // check consistency after insert/update/delete
           FullTextUtils.checkFullSyncToES(esClient, sdb, csName, clName, textIndexName, (int)cl.getCount());           

           // drop fulltext
           FullTextDBUtils.dropFullTextIndex(cl, textIndexName);

           // check fulltext deleted
           FullTextUtils.checkIndexNotExistInES(esClient, esIndexNames);
      }
	
      public boolean insertData(DBCollection cl, int insertNums) {
           List<BSONObject> insertObjs = new ArrayList<>();
           try {
                for(int i = 0; i < 100; i++){
                    for (int j = 0; j < insertNums/100; j++) {
                         insertObjs.add((BSONObject) JSON.parse("{a: 'test_14377_" + i*j + "', b:" + (i*10000 + j) + "}"));
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

      public void updateData(DBCollection cl){
           BSONObject modifier = new BasicBSONObject();
           BSONObject value = new BasicBSONObject();
           BSONObject matcher = new BasicBSONObject();
           BSONObject subMatcher = new BasicBSONObject();			
           value.put("b", "-1");
           modifier.put("$set", value);
           subMatcher.put("$lt", 100000);
           matcher.put("b", subMatcher);
           cl.update(matcher, modifier, null);	
      }

      public void removeData(DBCollection cl){
           BSONObject matcher = new BasicBSONObject();
           BSONObject subMatcher = new BasicBSONObject();
           subMatcher.put("$gt", 100000);
           matcher.put("b", subMatcher);	
           cl.delete(matcher);
      }
}
