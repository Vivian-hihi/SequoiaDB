/************************************
*@Description: seqDB-11353:seqDB-11353:rtnPredicate为[$minKey, $maxKey]的索引选择
*@author:      chimanzhao
*@createdate:  2020.4.25
*@testlinkCase: seqDB-11353
**************************************/
testConf.clName = COMMCLNAME + "_11353";

main( test );

function test (testPara)
{
   var dbcl = testPara.testCL;
   dbcl.createIndex( "a", {a:1} );
 
   //设置查询条件
   var conds = [{b:1},{$or:[{a:1},{c:1}]},{$not:[{a:1},{c:1}]}];
   
   //不计算IO代价
   var docs=[];
   for (var i = 0; i < 1000; i++ )
   {
      docs.push( { a:i, b:i, c:-i } )
   }
   dbcl.insert( docs );
   
   testExplain( conds, dbcl );
   db.analyze();
   testExplain( conds, dbcl );
   
   //计算IO代价
   //添加数据使数据页数大于optestcachesize（20）
   var docs=[];
   for (var i = 0; i < 50000; i++ )
   {
      docs.push( { d:i } )
   }
   dbcl.insert( docs );

   testExplain( conds, dbcl );
   db.analyze();
   testExplain( conds, dbcl );
}

function testExplain( conds, dbcl )
{
   var indexName = "" ;
   var scanType  = "tbscan" ;
   for ( var i = 0; i < conds.length; ++i )
   {
      checkExplain( dbcl, conds[i], indexName, scanType );
   }
}

function checkExplain( dbcl, cond, expIndexName, expScanType )
{
   var explainObj = dbcl.find( cond ).explain().next().toObj();
   var IndexName  = explainObj.IndexName;
   var ScanType   = explainObj.ScanType;
   if(expIndexName !== IndexName || expScanType !== ScanType)
   {
      throw new Error("索引选择错误！")
   }
}