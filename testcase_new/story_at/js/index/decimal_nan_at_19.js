/***************************************************************************************************
 * @Description: decimal 索引顺序
 * @ATCaseID: <填写 story 文档中验收用例的用例编号>
 * @Author: He Guoming
 * @TestlinkCase: 无（由测试人员维护，在测试阶段如果有测试场景引用本和例，则在此处填写 Testlink 用例编号，
 *                    并在 Testlink 系统中标记本用例文件名）
 * @Change Activity:
 * Date       Who           Description
 * ========== ============= =========================================================
 * 07/20/2023 HGM           Init
 **************************************************************************************************/

/*********************************************测试用例***********************************************
 * 环境准备：
 * 测试场景：
 *    decimal的NaN在索引中的顺序
 * 测试步骤：
 *    1.创建集合
 *    2.在集合上创建索引
 *    3.插入NaN、-Inf、Inf、Max、Min等decimal和double记录
 *
 * 期望结果：
 *    NaN在-Inf/Min之前
 *
 **************************************************************************************************/

testConf.clName = COMMCLNAME + "_decimal_nan_at_19";

main(test)

function testValue( cursor, expValue, expNum )
{
   var count = 0
   while( cursor.next() )
   {
      var curObj = cursor.current().toObj();
      if ( "NaN" == expValue )
      {
         assert.equal(
            (JSON.stringify(curObj["a"]) == JSON.stringify({$decimal:"NaN"}) ||
            (JSON.stringify(curObj["a"]) == JSON.stringify(NaN))), true )
      }
      else if ( "MIN" == expValue )
      {
         assert.equal(
            (JSON.stringify(curObj["a"]) == JSON.stringify({$decimal:"MIN"}) ||
            (JSON.stringify(curObj["a"]) == JSON.stringify(-Infinity))), true )
      }
      else if ( "MAX" == expValue )
      {
         assert.equal(
            (JSON.stringify(curObj["a"]) == JSON.stringify({$decimal:"MAX"}) ||
            (JSON.stringify(curObj["a"]) == JSON.stringify(Infinity))), true )
      }
      ++ count
      if ( count == expNum )
      {
         break;
      }
   }
}

function test(testPara)
{
   db.setSessionAttr({PreferredInstance:"m"}) ;
   var cl = testPara.testCL;
   cl.createIndex('a',{a:1});

   // case 1
   cl.insert({a:-Infinity})
   cl.insert({a:{$decimal:"MAX"}})
   cl.insert({a:{$decimal:"MIN"}})
   cl.insert({a:{$decimal:"NaN"}})
   cl.insert({a:NaN})
   cl.insert({a:Infinity})

   var cursor = cl.find({},{a:1}).sort({a:1}).hint({"":"a"})
   testValue(cursor, "NaN", 2)
   testValue(cursor, "MIN", 2)
   testValue(cursor, "MAX", 2)

   var cursor = cl.find({a:NaN},{a:1}).hint({"":"a"})
   testValue(cursor, "NaN", 2)
   var cursor = cl.find({a:{$decimal:"NaN"}},{a:1}).hint({"":"a"})
   testValue(cursor, "NaN", 2)
   var cursor = cl.find({a:NaN},{a:1}).hint({"":null})
   testValue(cursor, "NaN", 2)
   var cursor = cl.find({a:{$decimal:"NaN"}},{a:1}).hint({"":null})
   testValue(cursor, "NaN", 2)

   cl.truncate({SkipRecycleBin:true})

   // case 2
   cl.insert({a:NaN})
   cl.insert({a:{$decimal:"NaN"}})
   cl.insert({a:{$decimal:"MIN"}})
   cl.insert({a:{$decimal:"MAX"}})
   cl.insert({a:-Infinity})
   cl.insert({a:Infinity})

   var cursor = cl.find({},{a:1}).sort({a:1}).hint({"":"a"})
   testValue(cursor, "NaN", 2)
   testValue(cursor, "MIN", 2)
   testValue(cursor, "MAX", 2)

   var cursor = cl.find({a:NaN},{a:1}).hint({"":"a"})
   testValue(cursor, "NaN", 2)
   var cursor = cl.find({a:{$decimal:"NaN"}},{a:1}).hint({"":"a"})
   testValue(cursor, "NaN", 2)
   var cursor = cl.find({a:NaN},{a:1}).hint({"":null})
   testValue(cursor, "NaN", 2)
   var cursor = cl.find({a:{$decimal:"NaN"}},{a:1}).hint({"":null})
   testValue(cursor, "NaN", 2)
}
