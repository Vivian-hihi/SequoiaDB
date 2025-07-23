/*******************************************************************************

   Copyright (C) 2011-Present SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = CharsetTest.java

   Descriptive Name = N/A

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who         Description
   ====== =========== =========== =============================================
          23/07/2025  fangjiabin  Initial Draft

   Last Changed =

*******************************************************************************/
package com.sequoiadb.test.db;

import com.sequoiadb.base.*;
import com.sequoiadb.base.options.DeleteOption;
import com.sequoiadb.base.options.UpdateOption;
import com.sequoiadb.base.result.InsertResult;
import com.sequoiadb.base.result.UpdateResult;
import com.sequoiadb.test.common.BSONData;
import com.sequoiadb.test.common.Constants;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.junit.*;

import java.util.*;

public class CharsetTest {
    private final static String ADDRESS = Constants.COOR_NODE_CONN;
    private final static String USER_NAME = "";
    private final static String PASSWORD = "";
    private final static String CS_NAME = "charset_test_cs";
    private final static String CL_NAME = "charset_test_cl";
    private final static String CHINESE_CL_NAME = "ab字符集测试表cd";
    private static Sequoiadb db;
    private static CollectionSpace cs;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        db = new Sequoiadb(ADDRESS, USER_NAME, PASSWORD);

        if (db.isCollectionSpaceExist(CS_NAME)) {
            db.dropCollectionSpace(CS_NAME);
        }

        cs = db.createCollectionSpace(CS_NAME);
        cs.createCollection(CL_NAME);
        cs.createCollection(CHINESE_CL_NAME);
    }

    @AfterClass
    public static void tearDownAfterClass() throws Exception {
        try {
            db.dropCollectionSpace(CS_NAME);
        } finally {
            db.close();
        }
    }

    @Test
    public void testCharset() {
        // case 1: set charset
        ConfigOptions netConf = createNetConf(ClientCharsetEnum.GB18030);
        try (Sequoiadb db = new Sequoiadb(ADDRESS, USER_NAME, PASSWORD, netConf)){
            BSONObject obj = db.getSessionAttr(false);

            Assert.assertEquals(obj.toString(), obj.get("ClientCharset"), ClientCharsetEnum.GB18030.getName());
            Assert.assertEquals(obj.toString(), obj.get("ResultsCharset"), ClientCharsetEnum.GB18030.getName());
        }

        // case 2: not set charset
        try (Sequoiadb db = new Sequoiadb(ADDRESS, USER_NAME, PASSWORD)){
            BSONObject obj = db.getSessionAttr(false);
            String sdbCharsetName = "UTF8";

            Assert.assertEquals(obj.toString(), obj.get("ClientCharset"), sdbCharsetName);
            Assert.assertEquals(obj.toString(), obj.get("ResultsCharset"), sdbCharsetName);
        }
    }

    @Test
    public void test() {
        // case 1: UTF-8
        crudCheck(CL_NAME, ClientCharsetEnum.UTF_8);

        crudCheck(CHINESE_CL_NAME, ClientCharsetEnum.UTF_8);

        // case 2: GB18030
        crudCheck(CL_NAME, ClientCharsetEnum.GB18030);

        crudCheck(CHINESE_CL_NAME, ClientCharsetEnum.GB18030);
    }

    private ConfigOptions createNetConf(ClientCharsetEnum charsetType) {
        ClientCharset charset = new ClientCharset();
        charset.setCharsets(charsetType);

        ConfigOptions netConf = new ConfigOptions();
        netConf.setCharset(charset);

        return netConf;
    }

    private void crudCheck(String clName, ClientCharsetEnum charsetType) {
        ConfigOptions netConf = createNetConf(charsetType);

        try (Sequoiadb db = new Sequoiadb(ADDRESS, USER_NAME, PASSWORD, netConf)){
            DBCollection cl = db.getCollectionSpace(CS_NAME).getCollection(clName);

            processOneData(cl);

            processManyData(cl);
        }
    }

    private void processOneData(DBCollection cl) {
        String value = "你好，world!";
        BSONObject doc = BSONData.getAllTypeBSONObj(value, false);
        BSONObject matcher = new BasicBSONObject();
        matcher.putAll(doc);
        matcher.removeField("regex");

        // 1. insert one
        InsertResult insertResult = cl.insertRecord(doc);
        Assert.assertEquals(1, insertResult.getInsertNum());

        // 2. query check
        queryOneAndCheck(cl, doc, matcher);

        // 3. update one
        BSONObject updateDoc = new BasicBSONObject("$set", new BasicBSONObject("update", value));
        UpdateOption updateOptions = new UpdateOption();
        UpdateResult updateResult = cl.updateRecords(matcher, updateDoc, updateOptions);
        Assert.assertEquals(1, updateResult.getUpdatedNum());

        doc.put("update", value);
        matcher.put("update", value);

        // 4. query check
        queryOneAndCheck(cl, doc, matcher);

        // 5. delete one
        DeleteOption deleteOption = new DeleteOption();
        deleteOption.setFlag(DeleteOption.FLG_DELETE_ONE);
        cl.deleteRecords(doc);

        // 6. query check
        try (DBCursor cursor = cl.query(doc, null, null, null)) {
            int num = 0;
            while (cursor.hasNext()) {
                cursor.getNext();
                num++;
            }

            Assert.assertEquals(0, num);
        }

        // clean
        cl.truncate();
    }

    private void queryOneAndCheck(DBCollection cl, BSONObject data, BSONObject matcher) {
        try (DBCursor cursor = cl.query(matcher, null, null, null)) {
            int num = 0;
            while (cursor.hasNext()) {
                // test getNextRaw
                cursor.getNextRaw();
                BSONObject result = cursor.getCurrent();
                result.removeField("_id");

                BSONData.checkBSONObj(data, result);
                num++;
            }
            Assert.assertEquals(1, num);
        }
    }

    private List<BSONObject> paperDocs(String value, int num) {
        List<BSONObject> docList = new ArrayList<>();
        for (int i = 0; i < num; i++) {
            docList.add(BSONData.getAllTypeBSONObj(value, false));
        }
        return docList;
    }

    private void processManyData(DBCollection cl) {
        String value = "你好，world!";
        int insertNum = 10000;
        List<BSONObject> docList = paperDocs(value, insertNum);
        BSONObject doc = docList.get(0);

        // 1. bulk insert
        InsertResult insertResult = cl.bulkInsert(docList);
        Assert.assertEquals(insertNum, insertResult.getInsertNum());

        // 2. query
        doc.removeField("_id");
        BSONObject matcher = new BasicBSONObject();
        matcher.putAll(doc);
        matcher.removeField("regex");
        try (DBCursor cursor = cl.query(matcher, null, null, null)) {
            int num = 0;
            while (cursor.hasNext()) {
                BSONObject result = cursor.getNext();
                result.removeField("_id");

                BSONData.checkBSONObj(doc, result);
                num++;
            }

            Assert.assertEquals(insertNum, num);
        }

        // clean
        cl.truncate();
    }
}
