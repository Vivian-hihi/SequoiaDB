/**************************************************************************
 * @Description:   seqDB-33945:测试新增设置字符集接口
 * @Modify:        chenzejia Init
 *                 2024/1/10
 **************************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include "testcommon.hpp"
#include "testBase.hpp"
#include "arguments.hpp"

using namespace sdbclient;
using namespace bson;
using namespace std;

class gb18030Test33945 : public testBase
{
protected:
    const CHAR *csName;
    const CHAR *clName;
    const CHAR *indexName;
    sdbCollectionSpace cs;
    sdbCollection cl;
    vector<BSONObj> docs;

    void SetUp()
    {
        testBase::SetUp();
        csName = "字符集GB18030测试集合空间";
        clName = "字符集GB18030测试集合";
        indexName = "测试索引ab123";
        BSONObj obj1 = BSON("_id" << 1 << "字符串"
                                  << "你好"
                                  << "数组" << BSON_ARRAY("广州"
                                                          << "日本")
                                  << "对象" << BSON("a"
                                                    << "语文"));
        BSONObj obj2 = BSON("_id" << 2 << "字符串"
                                  << "啞啟"
                                  << "数组" << BSON_ARRAY("津巴布韦") << "对象" << BSON("a"
                                                                                        << "数学"));
        BSONObj obj3 = BSON("_id" << 3 << "字符串"
                                  << "啠啢啣"
                                  << "数组" << BSON_ARRAY("" << 12) << "对象" << BSON("a"
                                                                                      << "英语"));
        BSONObj obj4 = BSON("_id" << 4 << "字符串"
                                  << ""
                                  << "数组" << BSON_ARRAY("阿森松岛"
                                                          << "abc")
                                  << "对象" << BSON("a"
                                                    << "化学"));
        BSONObj obj5 = BSON("_id" << 5 << "字符串"
                                  << " 1肯尼亚"
                                  << "数组" << BSON_ARRAY("直布罗陀"
                                                          << "英斯兰文")
                                  << "对象" << BSON("a"
                                                    << "生物"));
        docs.push_back(obj1);
        docs.push_back(obj2);
        docs.push_back(obj3);
        docs.push_back(obj4);
        docs.push_back(obj5);
    }

    void TearDown()
    {
        testBase::TearDown();
    }
};

TEST_F(gb18030Test33945, testInterface)
{
    int rc = SDB_OK;
    const CHAR *clientCharset;
    const CHAR *resultsCharset;

    // check default charset
    clientCharset = db.getClientCharset();
    ASSERT_STREQ("UTF8", clientCharset) << "the clientCharset is not expected";
    resultsCharset = db.getResultsCharset();
    ASSERT_STREQ("UTF8", resultsCharset) << "the resultsCharset is not expected";

    // test setCharsets
    rc = db.setCharsets("GB18030");
    ASSERT_EQ(SDB_OK, rc) << "fail to set charsets";
    clientCharset = db.getClientCharset();
    ASSERT_STREQ("GB18030", clientCharset) << "the clientCharset is not expected";
    resultsCharset = db.getResultsCharset();
    ASSERT_STREQ("GB18030", resultsCharset) << "the resultsCharset is not expected";

    rc = db.setCharsets("utf8");
    ASSERT_EQ(SDB_OK, rc) << "fail to set charsets";
    clientCharset = db.getClientCharset();
    ASSERT_STREQ("UTF8", clientCharset) << "the clientCharset is not expected";
    resultsCharset = db.getResultsCharset();
    ASSERT_STREQ("UTF8", resultsCharset) << "the resultsCharset is not expected";

    // test setClientCharset
    rc = db.setClientCharset("gb18030");
    ASSERT_EQ(SDB_OK, rc) << "fail to set client charset";
    clientCharset = db.getClientCharset();
    ASSERT_STREQ("GB18030", clientCharset) << "the clientCharset is not expected";
    rc = db.setClientCharset("utf8");
    ASSERT_EQ(SDB_OK, rc) << "fail to set client charset";
    clientCharset = db.getClientCharset();
    ASSERT_STREQ("UTF8", clientCharset) << "the clientCharset is not expected";

    // test setResultsCharset
    rc = db.setResultsCharset("GB18030");
    ASSERT_EQ(SDB_OK, rc) << "fail to set results charset";
    resultsCharset = db.getResultsCharset();
    ASSERT_STREQ("GB18030", resultsCharset) << "the resultsCharset is not expected";
    rc = db.setResultsCharset("UTF8");
    ASSERT_EQ(SDB_OK, rc) << "fail to set results charset";
    resultsCharset = db.getResultsCharset();
    ASSERT_STREQ("UTF8", resultsCharset) << "the resultsCharset is not expected";

    // test invalid charset
    rc = db.setCharsets("gbk");
    ASSERT_EQ(SDB_INVALIDARG, rc) << "set invalid charset rc is not expected";
    rc = db.setClientCharset("gbk");
    ASSERT_EQ(SDB_INVALIDARG, rc) << "set invalid charset rc is not expected";
    rc = db.setResultsCharset("gbk");
    ASSERT_EQ(SDB_INVALIDARG, rc) << "set invalid charset rc is not expected";
}

TEST_F(gb18030Test33945, testFunction)
{
    INT32 rc = SDB_OK;
    const static BSONObj staticObj;
    BSONObj obj;
    BSONObj result;
    sdbCursor cursor;
    BSONObj modifier;
    BSONObj matcher;
    BSONObj hint;
    vector<BSONObj> results;

    // set GB18030 charset
    rc = db.setCharsets("GB18030");
    ASSERT_EQ(SDB_OK, rc) << "fail to set charsets";

    rc = db.getCollectionSpace(csName, cs);
    if (rc != SDB_DMS_CS_NOTEXIST)
    {
        db.dropCollectionSpace(csName);
    }
    // create cs and cl
    rc = createNormalCsCl(db, cs, cl, csName, clName);
    ASSERT_EQ(SDB_OK, rc) << "fail to create cs " << csName << " cl " << clName;

    // create index, index name and field name are both Chinese
    obj = BSON("字符串" << 1);
    rc = cl.createIndex(obj, indexName, FALSE, FALSE);
    ASSERT_EQ(SDB_OK, rc) << "fail to create index " << indexName;

    // insert data
    rc = cl.insert(docs, 0, &result);
    ASSERT_EQ(SDB_OK, rc) << "fail to insert data";

    // query data
    cl.query(cursor);
    while (!(rc = cursor.next(obj)))
    {
        results.push_back(obj);
    }
    cursor.close();
    ASSERT_EQ(docs, results) << "the result is not expected";
    // query data with index
    matcher = BSON("字符串"
                   << BSON("$gte"
                           << ""));
    hint = BSON("" << indexName);
    cl.query(cursor, matcher, staticObj, staticObj, hint);
    results.clear();
    while (!(rc = cursor.next(obj)))
    {
        results.push_back(obj);
    }
    cursor.close();
    vector<BSONObj> expected;
    expected.push_back(docs[3]);
    expected.push_back(docs[4]);
    expected.push_back(docs[0]);
    expected.push_back(docs[1]);
    expected.push_back(docs[2]);
    ASSERT_EQ(expected, results) << "the result is not expected";

    // update data
    matcher = BSON("字符串"
                   << "啠啢啣"
                   << "_id" << 3);
    modifier = BSON("$set" << BSON("字符串"
                                   << "新数据"));
    rc = cl.update(modifier, matcher, staticObj, 0, &result);
    ASSERT_EQ(SDB_OK, rc) << "fail to update data";
    ASSERT_EQ(1, result.getIntField("UpdatedNum")) << "the update result is not expected";
    ASSERT_EQ(1, result.getIntField("ModifiedNum")) << "the update result is not expected";
    // check the update result
    matcher = BSON("_id" << 3);
    cl.queryOne(result, matcher);
    ASSERT_EQ("新数据", string(result.getStringField("字符串"))) << "the result is not expected";

    // delete data
    matcher = BSON("字符串" << BSON("$gt"
                                    << ""));
    rc = cl.del(matcher, staticObj, 0, &result);
    ASSERT_EQ(SDB_OK, rc) << "fail to delete data";
    ASSERT_EQ(4, result.getIntField("DeletedNum")) << "the delete result is not expected";
    // check the delete result
    cl.queryOne(result);
    ASSERT_EQ(docs[3], result) << "the result is not expected";

    // drop index
    rc = cl.dropIndex(indexName);
    ASSERT_EQ(SDB_OK, rc) << "fail to drop index " << indexName;

    // drop cl
    rc = cs.dropCollection(clName);
    ASSERT_EQ(SDB_OK, rc) << "fail to drop cl " << clName;

    // drop cs
    rc = db.dropCollectionSpace(csName);
    ASSERT_EQ(SDB_OK, rc) << "fail to drop cs " << csName;
}