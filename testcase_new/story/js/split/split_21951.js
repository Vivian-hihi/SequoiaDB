/************************************
*@description： seqDB-21951:主子表，主表查询命中多个子表且命中同一个数据组
*@author ：2020-3-31 liyuanyue
**************************************/
testConf.skipStandAlone = true;
testConf.skipOneGroup = true;

main( test );

function test ()
{
   var dataGroupNames = commGetDataGroupNames( db );
   var testcaseID = 21951;
   var csName = "cs_" + testcaseID;
   var mclName = "mcl_" + testcaseID;
   var sclName1 = "scl_" + testcaseID + "_1";
   var sclName2 = "scl_" + testcaseID + "_2";
   var sclFullName1 = csName + "." + sclName1;
   var sclFullName2 = csName + "." + sclName2;
   var recsNum = 200;

   commDropCS( db, csName );

   var options = { ShardingKey: { a: 1 }, IsMainCL: true };
   var mcl = commCreateCL( db, csName, mclName, options );
   var options = { ShardingKey: { a: 1 }, Group: dataGroupNames[0] };
   var scl1 = commCreateCL( db, csName, sclName1, options );
   var scl2 = commCreateCL( db, csName, sclName2, options );
   mcl.attachCL( sclFullName1, { LowBound: { a: 0 }, UpBound: { a: 100 } } );
   mcl.attachCL( sclFullName2, { LowBound: { a: 100 }, UpBound: { a: 220 } } );

   var docs = [];
   for( var i = 0; i < recsNum; i++ )
   {
      docs.push( { a: i } );
   }
   mcl.insert( docs );

   // subCL split to multi group
   scl1.split( dataGroupNames[0], dataGroupNames[1], { "a": 30 }, { "a": 60 } );
   scl2.split( dataGroupNames[0], dataGroupNames[1], { "a": 130 }, { "a": 160 } );

   var findCond = { "a": { "$in": [20, 120] } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 20, 21 ).concat( docs.slice( 120, 121 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[0]] );

   commDropCS( db, csName );
}
