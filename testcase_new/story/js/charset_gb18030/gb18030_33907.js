/************************************
*@Description: 设置字符集为GB18030，执行数据切分
*@author:      chenzejia
*@createdate:  2023.12.16
*@testlinkCase:seqDB-33907
**************************************/

testConf.skipStandAlone = true;
testConf.skipOneGroup = true;

main( test );
function test ()
{
   var csName = "集合空间_33907";
   var clName = "集合_33907";
   db.setCharsets( "GB18030" );
   var dataGroupNames = commGetDataGroupNames( db );

   var recsNum = 100;
   var docs = [];
   for( var i = 0; i < recsNum; i++ )
   {
      docs.push( { "a": "测试数据" + i, "b": i } );
   }

   // create range cl
   commDropCL( db, csName, clName, true );
   var options = { "ShardingKey": { "a": 1 }, "ShardingType": "range", "Group": dataGroupNames[0] };
   var cl = commCreateCL( db, csName, clName, options, true );
   cl.insert( docs );

   // subCL split to multi group
   cl.split( dataGroupNames[0], dataGroupNames[1], { "a": "测试数据50" }, { "a": { "$maxKey": 1 } } );
   // check data distribution
   var findCond = { "a": { "$gte": "测试数据50" } };
   checkHitDataGroups( cl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[1]] );
   findCond = { "a": { "$lt": "测试数据50" } };
   checkHitDataGroups( cl.find( findCond ).explain( { "Run": true } ), [dataGroupNames[0]] );

   // create hash cl
   commDropCL( db, csName, clName, true );
   options = { "ShardingKey": { "a": 1 }, "ShardingType": "hash", "Partition": 256, "Group": dataGroupNames[0] };
   cl = commCreateCL( db, csName, clName, options, true );
   cl.insert( docs );

   // subCL split to multi group
   cl.split( dataGroupNames[0], dataGroupNames[1], { "Partition": 128 }, { "Partition": 256 } );
   // check data distribution
   checkHashDistribution( db, csName, clName, dataGroupNames[0], 55 );
   checkHashDistribution( db, csName, clName, dataGroupNames[1], 45 );

   commDropCS( db, csName );
}

function checkHitDataGroups ( explainCursor, datagroups )
{
   var hitDataGroups = [];
   while( explainCursor.next() )
   {
      var explainResult = explainCursor.current();
      var hitDataGroup = explainResult.toObj().GroupName;
      hitDataGroups.push( hitDataGroup );
   }
   if( hitDataGroups.length != datagroups.length )
   {
      throw new Error( "expHitDataGroupNum: " + datagroups.length + ", actHitDataGroupNum:"
         + hitDataGroups.length );
   }
   for( var i = 0; i < datagroups.length; i++ )
   {
      if( hitDataGroups.indexOf( datagroups[i] ) == -1 )
      {
         throw new Error( "actHitDataGroups:" + hitDataGroups + ", expHitDataGroups:" + datagroups );
      }
   }
}

function checkHashDistribution ( db, csName, clName, dataGroup, recordsNum )
{
   var rgDB = db.getRG( dataGroup ).getMaster().connect();
   rgDB.setCharsets( "GB18030" );
   var cl = rgDB.getCS( csName ).getCL( clName );
   if( cl.count() != recordsNum )
   {
      throw new Error( "actRecordsNum:" + cl.count() + ", expRecordsNum:" + recordsNum );
   }
}