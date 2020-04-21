/************************************
*@description：seqDB-21956:分区表，包含非分区键查询
               seqDB-21957:分区表，不带匹配符精确匹配 
*@author ：2020-3-31 liyuanyue
**************************************/
testConf.skipStandAlone = true;
testConf.skipOneGroup = true;

main( test );

function test ()
{
   var dataGroupNames = commGetDataGroupNames( db );
   var testcaseID = "21956_21957";
   var csName = "cs_" + testcaseID;
   var hashclName = "hashcl_" + testcaseID;
   var rangeclName = "rangecl_" + testcaseID;
   var mclName = "mcl_" + testcaseID;
   var sclName1 = "scl_" + testcaseID + "_1";
   var sclName2 = "scl_" + testcaseID + "_2";
   var sclFullName1 = csName + "." + sclName1;
   var sclFullName2 = csName + "." + sclName2;
   var recsNum = 200;

   commDropCS( db, csName );

   var hashcl = commCreateCL( db, csName, hashclName, { ShardingKey: { "a": 1 }, ShardingType: "range", Group: dataGroupNames[0] } );
   var rangecl = commCreateCL( db, csName, rangeclName, { ShardingKey: { "a": 1 }, ShardingType: "range", Group: dataGroupNames[0] } );
   var mcl = commCreateCL( db, csName, mclName, { ShardingKey: { "a": 1 }, IsMainCL: true } );
   var scl1 = commCreateCL( db, csName, sclName1, { ShardingKey: { "a": 1 }, "ShardingType": "range", Group: dataGroupNames[0] } );
   var scl2 = commCreateCL( db, csName, sclName2, { ShardingKey: { "a": 1 }, "ShardingType": "range", Group: dataGroupNames[0] } );

   mcl.attachCL( sclFullName1, { LowBound: { "a": 0 }, UpBound: { "a": 100 } } );
   mcl.attachCL( sclFullName2, { LowBound: { "a": 100 }, UpBound: { "a": 200 } } );

   var docs = [];
   for( var i = 0; i < recsNum; i++ )
   {
      docs.push( { "a": i, "b": i } );
   }

   hashcl.insert( docs );
   hashcl.split( dataGroupNames[0], dataGroupNames[1], { "a": 30 }, { "a": 100 } );
   var findCond = { "b": { "$gte": 80 } };
   commCompareResults( hashcl.find( new SdbQueryOption().cond( findCond ).sort( { a: 1 } ) ), docs.slice( 80 ) );
   checkHitDataGroups( hashcl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[0], dataGroupNames[1]] );
   var findCond = { "b": 80, "a": 80 };
   commCompareResults( hashcl.find( findCond ), docs.slice( 80, 81 ) );
   checkHitDataGroups( hashcl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[1]] );
   var findCond = { "a": 80 };
   commCompareResults( hashcl.find( findCond ), docs.slice( 80, 81 ) );
   checkHitDataGroups( hashcl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[1]] );

   rangecl.insert( docs );
   rangecl.split( dataGroupNames[0], dataGroupNames[1], { "a": 60 }, { "a": 160 } );
   var findCond = { "b": { "$gte": 100 } };
   commCompareResults( rangecl.find( new SdbQueryOption().cond( findCond ).sort( { a: 1 } ) ), docs.slice( 100 ) );
   checkHitDataGroups( rangecl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[0], dataGroupNames[1]] );
   var findCond = { "b": 20, "a": 20 };
   commCompareResults( rangecl.find( findCond ), docs.slice( 20, 21 ) );
   checkHitDataGroups( rangecl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[0]] );
   var findCond = { "a": 170 };
   commCompareResults( rangecl.find( findCond ), docs.slice( 170, 171 ) );
   checkHitDataGroups( rangecl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[0]] );

   mcl.insert( docs );
   scl1.split( dataGroupNames[0], dataGroupNames[1], { "a": 40 }, { "a": 80 } );
   scl2.split( dataGroupNames[0], dataGroupNames[1], { "a": 140 }, { "a": 180 } );
   var findCond = { "b": { "$gte": 120 } };
   commCompareResults( mcl.find( new SdbQueryOption().cond( findCond ).sort( { a: 1 } ) ), docs.slice( 120 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );
   var findCond = { "b": 110, "a": 110 };
   commCompareResults( mcl.find( findCond ), docs.slice( 110, 111 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[0]], true, [[sclFullName2]] );
   var findCond = { "a": 50 };
   commCompareResults( mcl.find( findCond ), docs.slice( 50, 51 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[1]], true, [[sclFullName1]] );

   commDropCS( db, csName );
}
