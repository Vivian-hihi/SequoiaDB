/************************************
*@Description: seqDB-11354:rtnPredicate为[valA, valA]，唯一索引下的索引选择
*@author:      chimanzhao
*@createdate:  2020.5.6
*@testlinkCase: seqDB-11354
**************************************/
testConf.clName = COMMCLNAME + "_11354";

main( test )

function test(testPara)
{
   var dbcl = testPara.testCL;
   dbcl.createIndex( "a", {a:1}, true );
   dbcl.createIndex( "b", {b:-1}, true );

   //设置查询条件,构造valA在mcv中存在统计信息的场景(不计算IO代价时,a以5为周期选入mcv中;计算IO代价时，a存入mcv的值为250，506，761)
   var conds = [{ a: { $et: 250 } },{a: { $in: [ 250, 506 ] }},{a: { $all: [ 250 ] }},{a: { $exists: 0 } },{a: { $isnull: 1 } }  ];
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
   var indexName = "a" ;
   var scanType  = "ixscan" ;
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