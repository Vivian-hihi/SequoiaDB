/******************************************************************************
@Description : seqDB-21949:主子表，主表分区键为单字段正序，带匹配符查询 
@Author : YuanYue Li 2020-04-14
******************************************************************************/
testConf.skipStandAlone = true;
testConf.skipOneGroup = true;

main( test );
function test ( arg )
{
   var dataGroupNames = commGetDataGroupNames( db );
   var testcaseID = 21949;
   var csName = "cs_" + testcaseID;
   var mclName = "mcl_" + testcaseID;
   var sclName1 = "scl_" + testcaseID + "_1";
   var sclName2 = "scl_" + testcaseID + "_2";
   var sclFullName1 = csName + "." + sclName1;
   var sclFullName2 = csName + "." + sclName2;
   var recsNum = 200;

   commDropCS( db, csName );

   // ready main-sub cl
   var mclOptions = { "ShardingKey": { "a": 1 }, "IsMainCL": true };
   var mcl = commCreateCL( db, csName, mclName, mclOptions );
   var sclOptions = { "ShardingKey": { "a": 1 }, "ShardingType": "range", "Group": dataGroupNames[0] };
   var scl1 = commCreateCL( db, csName, sclName1, sclOptions );
   var scl2 = commCreateCL( db, csName, sclName2, sclOptions );
   mcl.attachCL( sclFullName1, { "LowBound": { "a": 0 }, "UpBound": { "a": 100 } } );
   mcl.attachCL( sclFullName2, { "LowBound": { "a": 100 }, "UpBound": { "a": 200 } } );

   // insert
   var docs = [];
   for( var i = 0; i < recsNum; i++ )
   {
      docs.push( { "a": i, "b": { "c": i, "d": 'dd' + i }, "e": [i, i + 1000], "f": '222' + i } );
   }
   mcl.insert( docs );

   // subCL split to multi group
   scl1.split( dataGroupNames[0], dataGroupNames[1], { "a": 30 }, { "a": 60 } );
   scl2.split( dataGroupNames[0], dataGroupNames[1], { "a": 130 }, { "a": 160 } );

   // $gt   > 160
   var findCond = { "a": { "$gt": 160 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 161 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0]], true, [[sclFullName2]] );

   // $lt   < 30
   var findCond = { "a": { "$lt": 30 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 30 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0]], true, [[sclFullName1]] );

   // $gte   >=160
   var findCond = { "a": { "$gte": 160 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 160 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0]], true, [[sclFullName2]] );

   // $lte  <=30
   var findCond = { "a": { "$lte": 30 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 31 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName1], [sclFullName1]] );

   // $et   ==130
   var findCond = { "a": { "$et": 130 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 130, 131 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[1]], true, [[sclFullName2]] );

   // $ne    !=0
   var findCond = { "a": { "$ne": 0 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 1 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $in    =20、30、140
   var findCond = { "a": { "$in": [20, 30, 140, 260] } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 20, 21 ).concat( docs.slice( 30, 31 ) ).concat( docs.slice( 140, 141 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $nin   not in
   var findCond = { "a": { "$nin": [20, 30, 140, 260] } };
   var tmpDocs = docs.concat();
   tmpDocs.splice( 140, 1 );
   tmpDocs.splice( 30, 1 );
   tmpDocs.splice( 20, 1 );
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), tmpDocs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $and  
   // [60,100)，相交
   var findCond = { "$and": [{ "a": { "$gte": 60 } }, { "a": { "$lt": 100 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 60, 100 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0]], true, [[sclFullName1]] );

   // <=60 && >100,empty，相离 
   var findCond = { "$and": [{ "a": { "$lte": 60 } }, { "a": { "$gt": 100 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), [] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), ['SYSCoord'] );

   // >=100 && =140，包含
   var findCond = { "$and": [{ "a": { "$in": [20, 30, 140, 260] } }, { "a": { "$gt": 100 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 140, 141 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2], [sclFullName2]] );

   // $not
   // =20、30、140 || !=null 相交
   var findCond = { "$not": [{ "a": { "$nin": [20, 30, 140, 260] } }, { "a": { "$isnull": 1 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // <60 || >=100，相离
   var findCond = { "$not": [{ "a": { "$gte": 60 } }, { "a": { "$lt": 100 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 0, 60 ).concat( docs.slice( 100 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // >=60 || >30，包含
   var findCond = { "$not": [{ "a": { "$lte": 30 } }, { "a": { "$lt": 60 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 31 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $or
   // >=60 || <100，相交
   var findCond = { "$or": [{ "a": { "$gte": 60 } }, { "a": { "$lt": 100 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // <=60 || >100 相离
   var findCond = { "$or": [{ "a": { "$lte": 60 } }, { "a": { "$gt": 100 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 0, 61 ).concat( docs.slice( 101 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // <60 || <=30 包含
   var findCond = { "$or": [{ "a": { "$lte": 30 } }, { "a": { "$lt": 60 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 60 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName1], [sclFullName1]] );

   // $all
   var findCond = { "a": { "$all": [30] } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 30, 31 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[1]], true, [[sclFullName1]] );

   // $isnull 
   // = 0,字段值非null
   var findCond = { "a": { "$isnull": 0 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // = 0,字段值null
   var findCond = { "w": { "$isnull": 0 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), [] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // = 1,字段值非null
   var findCond = { "a": { "$isnull": 1 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), [] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), ['SYSCoord'] );

   // = 1,字段值为null
   var findCond = { "w": { "$isnull": 1 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $exists
   // = 0,字段值存在
   var findCond = { "a": { "$exists": 0 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), [] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), ['SYSCoord'] );

   // = 0,字段值不存在
   var findCond = { "w": { "$exists": 0 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // = 1,字段值存在
   var findCond = { "a": { "$exists": 1 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // = 1,字段值不存在
   var findCond = { "w": { "$exists": 1 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), [] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $elemMatch
   var findCond = { "b": { "$elemMatch": { "c": 60, "d": "dd60" } } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 60, 61 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $+标识符
   var findCond = { "e.$1": 130 };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 130, 131 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // type
   var findCond = { "a": { "$type": 1, "$et": 16 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // size
   var findCond = { "e": { "$size": 1, "$et": 2 } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $regex
   var findCond = { "f": { "$regex": '2223[1-9]\d*', "$options": 'i' } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 31, 40 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $mod
   var findCond = { "a": { "$mod": [150, 10] } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 10, 11 ).concat( docs.slice( 160, 161 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $expand
   var findCond = { "$and": [{ "a": { "$et": 60 } }, { "e": { "$expand": 1 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "e": 1 } )
      , [{ "a": 60, "b": { "c": 60, "d": "dd60" }, "e": 60, "f": "22260" }, { "a": 60, "b": { "c": 60, "d": "dd60" }, "e": 1060, "f": "22260" },] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0]], true, [[sclFullName1]] );

   // $returnMatch
   var findCond = { "e": { $returnMatch: [0, 2], "$in": [80, 1080] } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 80, 81 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $et+$field组合查询
   var findCond = { a: { $et: { $field: "b.c" } } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // $regex + $elemMatch组合查询
   var findCond = { "b": { "$elemMatch": { "d": { "$regex": '^dd[1-9]\d*$', "$options": 'i' } } } };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 1, 10 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // 多个$and组合查询
   // >=60 && <100 && !=null 相交
   var findCond = { "$and": [{ "a": { "$gte": 60 } }, { "a": { "$lt": 100 } }, { a: { $isnull: 0 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 60, 100 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0]], true, [[sclFullName1]] );

   // <=60 && >100 && ,empty，相离 
   var findCond = { "$and": [{ "a": { "$lte": 60 } }, { "a": { "$gt": 100 } }, { a: { $in: [70, 80, 90] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), [] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), ['SYSCoord'] );

   // >=100 && =140 && >=130 ，包含
   var findCond = { "$and": [{ "a": { "$in": [20, 30, 140, 260] } }, { "a": { "$gt": 100 } }, { "a": { "$gte": 130 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 140, 141 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2], [sclFullName2]] );

   // 多个$not组合查询
   // =20、30、140 || <30 || !=null 相交
   var findCond = { "$not": [{ "a": { "$nin": [20, 30, 140, 260] } }, { "a": { "$gte": 30 } }, { "a": { "$isnull": 1 } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // <60 || >=100 || =60、80，相离
   var findCond = { "$not": [{ "a": { "$gte": 60 } }, { "a": { "$lt": 100 } }, { "a": { "$nin": [60, 80] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 0, 61 ).concat( docs.slice( 80, 81 ) ).concat( docs.slice( 100 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // >=60 || >30 || =60,100,130，包含
   var findCond = { "$not": [{ "a": { "$lte": 30 } }, { "a": { "$lt": 60 } }, { "a": { "$nin": [60, 100, 130] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 31 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // 多个$or组合查询
   // >=60 || <100 || =60,100,130 相交
   var findCond = { "$or": [{ "a": { "$gte": 60 } }, { "a": { "$lt": 100 } }, { "a": { "$in": [60, 100, 130] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // <=60 || >100 || =70,80,90 相离
   var findCond = { "$or": [{ "a": { "$lte": 60 } }, { "a": { "$gt": 100 }, }, { "a": { "$in": [70, 80, 90] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 0, 61 ).concat( docs.slice( 70, 71 ) ).concat( docs.slice( 80, 81 ) ).concat( docs.slice( 90, 91 ) ).concat( docs.slice( 101 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // <60 || <=30 || =30,0 包含
   var findCond = { "$or": [{ "a": { "$lte": 30 } }, { "a": { "$lt": 60 } }, { "a": { "$in": [30, 0] } }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 0, 60 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName1], [sclFullName1]] );

   // $and + $or + $not组合查询
   // >=30 && < 130 && (a>=60 || <100) && <=80  [30,80]
   var findCond = {
      "$and": [{ "a": { "$gte": 30 } }, { "a": { "$lt": 130 } }
         , { "$or": [{ "a": { "$gte": 60 } }, { "a": { "$lt": 100 } }] }, { "$not": [{ "a": { "$gt": 80 } }] }]
   };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } ), docs.slice( 30, 81 ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName1]] );

   // $not+$and+$or组合查询
   // <30  || <80 || >=160 ||  (<=130 && >=100)  [0,80)∪[100,130]∪[160,200) 
   var findCond = {
      "$not": [{ "a": { "$gte": 30 } }, { "$and": [{ "a": { "$gte": 80 } }, { "a": { "$lt": 160 } }] }
         , { "$or": [{ "a": { "$gt": 130 } }, { "a": { "$lt": 100 } }] }]
   };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 0, 80 ).concat( docs.slice( 100, 131 ) ).concat( docs.slice( 160, 200 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2, sclFullName1]] );

   // 多个$and+$多个$or+多个$not
   // (>=30 || <160) && (>60 && <=200) && (<=160) && (>130 || <=100) && (>=80)  [80,100]∪(130,160]
   var findCond = { "$and": [{ "$or": [{ "a": { "$gte": 30 } }, { "a": { "$lt": 160 } }] }, { "$and": [{ "a": { "$gt": 60 } }, { "a": { "$lte": 200 } }] }, { "$not": [{ "a": { "$gt": 160 } }] }, { "$or": [{ "a": { "$gt": 130 } }, { "a": { "$lte": 100 } }] }, { "$not": [{ "a": { "$lt": 80 } }] }] };
   commCompareResults( mcl.find( findCond ).sort( { "a": 1 } )
      , docs.slice( 80, 101 ).concat( docs.slice( 131, 161 ) ) );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } )
      , [dataGroupNames[0], dataGroupNames[1]], true, [[sclFullName2, sclFullName1], [sclFullName2]] );

   commDropCS( db, csName );
}