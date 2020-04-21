/******************************************************************************
@Description : seqDB-21950:主子表，主表分区键为多字段正逆序混合，带匹配符查询
@Author : YuanYue Li 2020-04-20
******************************************************************************/
testConf.skipStandAlone = true;
testConf.skipOneGroup = true;

main( test );
function test ( arg )
{
   var dataGroupNames = commGetDataGroupNames( db );
   var testcaseID = 21950;
   var csName = "cs_" + testcaseID;
   var mclName = "mcl_" + testcaseID;
   var sclName1 = "scl_" + testcaseID + "_1";
   var sclName2 = "scl_" + testcaseID + "_2";
   var sclFullName1 = csName + "." + sclName1;
   var sclFullName2 = csName + "." + sclName2;
   var recsNum = 200;

   commDropCS( db, csName );

   // ready main-sub cl
   var mclOptions = { "ShardingKey": { "a": 1, "b": -1 }, "IsMainCL": true };
   var mcl = commCreateCL( db, csName, mclName, mclOptions );
   var sclOptions = { "ShardingKey": { "a": 1, "b": -1 }, "ShardingType": "range", "Group": dataGroupNames[0] };
   var scl1 = commCreateCL( db, csName, sclName1, sclOptions );
   var scl2 = commCreateCL( db, csName, sclName2, sclOptions );
   mcl.attachCL( sclFullName1, { "LowBound": { "a": 0, "b": 200 }, "UpBound": { "a": 100, "b": 100 } } );
   mcl.attachCL( sclFullName2, { "LowBound": { "a": 100, "b": 100 }, "UpBound": { "a": 200, "b": 0 } } );

   // insert
   var docs = [];
   for( var i = 0; i < recsNum; i++ )
   {
      docs.push( { "a": i, "b": i, "c": { "d": "test" + i } } );
   }
   mcl.insert( docs );

   // subCL split to multi group
   scl1.split( dataGroupNames[0], dataGroupNames[1], 50 );
   scl2.split( dataGroupNames[0], dataGroupNames[1], 50 );

   // $gt 
   var findCond = { "a": { "$gt": 100 }, "b": { "$gt": 100 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 101 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2], [sclFullName2]] );

   // $lt  
   var findCond = { "a": { "$lt": 100 }, "b": { "$lt": 100 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 100 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName1], [sclFullName1]] );

   // $gte
   var findCond = { "a": { "$gte": 100 }, "b": { "$gte": 100 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 100 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2], [sclFullName2, sclFullName1]] );

   // $lte
   var findCond = { "a": { "$lte": 100 }, "b": { "$lte": 100 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 101 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName1]] );

   // $et
   var findCond = { "a": { "$et": 100 }, "b": { "$et": 100 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 100, 101 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0]], true, [[sclFullName2]] );

   // $ne
   var findCond = { "a": { "$ne": 100 }, "b": { "$ne": 100 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 100 ).concat( docs.slice( 101 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $in
   var findCond = { "a": { "$in": [99, 100] }, "b": { "$in": [99, 100] } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 99, 101 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2], [sclFullName1]] );

   // $nin
   var findCond = { "a": { "$nin": [20, 30, 140, 260] }, "b": { "$nin": [20, 30, 140, 260] } };
   var tmpDocs = docs.concat();
   tmpDocs.splice( 140, 1 );
   tmpDocs.splice( 30, 1 );
   tmpDocs.splice( 20, 1 );
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), tmpDocs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $and 子集相交
   var findCond = { "$and": [{ "a": { "$gte": 100 } }, { "b": { "$lt": 150 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 100, 150 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2], [sclFullName2, sclFullName1]] );

   // $and 子集包含
   var findCond = { "$and": [{ "a": { "$gte": 150 } }, { "b": { "$gte": 151 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 151 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2], [sclFullName2]] );

   // $and 子集相离
   var findCond = { "$and": [{ "a": { "$gt": 100 } }, { "b": { "$in": [100] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), [] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2], [sclFullName2]] );

   // $not 子集相交
   var findCond = { "$not": [{ "a": { "$lt": 100 } }, { "b": { "$nin": [100, 130, 120] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 100 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $not 子集包含
   var findCond = { "$not": [{ "a": { "$lt": 100 } }, { "b": { "$lte": 150 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 100 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $not 子集相离
   var findCond = { "$not": [{ "a": { "$lt": 100 } }, { "b": { "$gt": 50 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 51 ).concat( docs.slice( 100 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $or 子集相交
   var findCond = { "$or": [{ "a": { "$lt": 100 } }, { "b": { "$gt": 50 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $or 子集包含
   var findCond = { "$or": [{ "a": { "$gte": 100 } }, { "b": { "$in": [120, 150, 170] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 100 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $or 子集相离
   var findCond = { "$or": [{ "a": { "$lt": 100 } }, { "b": { "$gt": 150 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 100 ).concat( docs.slice( 151 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $all
   var findCond = { "a": { "$all": [100] }, "b": { "$all": [100] } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 100, 101 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0]], true, [[sclFullName2]] );

   // $isnull
   var findCond = { "a": { "$isnull": 0 }, "b": { "$isnull": 0 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $exists
   var findCond = { "a": { "$exists": 1 }, "b": { "$exists": 1 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $et+$field
   var findCond = { "a": { "$et": { "$field": "b" } } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $regex + $elemMatch
   var findCond = { "c": { "$elemMatch": { "d": { "$regex": "test3[1-9]\d*", "$options": "i" } } } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 31, 40 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $and+$or+$not组合查询
   var findCond = {
      "$and": [{ "a": { "$gte": 30 }, "b": { "$gte": 30 } }
         , { "$or": [{ "a": { "$gte": 60 }, "b": { "$gte": 60 } }, { "a": { "$lt": 100 }, "b": { "$lt": 100 } }] }
         , { "$not": [{ "a": { "$gt": 80 }, "b": { "$gt": 80 } }] }]
   };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 30, 81 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $not+$and+$or组合查询
   var findCond = {
      "$not": [{ "a": { "$gte": 30 }, "b": { "$gte": 30 } }
         , { "$and": [{ "a": { "$gte": 80 }, "b": { "$gte": 80 } }, { "a": { "$lt": 160 }, "b": { "$lt": 160 } }] }
         , { "$or": [{ "a": { "$gt": 130 }, "b": { "$gt": 130 } }, { "a": { "$lt": 100 }, "b": { "$lt": 100 } }] }]
   };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 0, 80 ).concat( docs.slice( 100, 131 ) ).concat( docs.slice( 160, 200 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   commDropCS( db, csName );
}