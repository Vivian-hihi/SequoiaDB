/************************************
*@Description: 普通表所属domain设置AutoSplit为true，新增shardingKey 
*@author:      wangkexin
*@createdate:  2019.5.24
*@testlinkCase:seqDB-18348
**************************************/

main();
function main()
{
   if( commIsStandalone( db ) )
   {
      println( "Run mode is standalone" ) ;
      return ;
   }
   //less two groups to split
   var allGroupName = getGroupName( db, true );
   if( allGroupName.length < 2 )
   {
      println("--least two groups");
      return ;
   }
   
   var csName = CHANGEDPREFIX + "_cs_18348";
   var clName1 = CHANGEDPREFIX + "_cl_18348a";
   var clName2 = CHANGEDPREFIX + "_cl_18348b";
   var domainName = "domain18348";
   
   //clean environment before test
   commDropCS( db, csName, true, "drop CS in the beginning." );
   dropDomain( db, domainName, true, "drop Domain in the beginning." );
   
   var group1 = allGroupName[0];
   var group2 = allGroupName[1];
   db.createDomain( domainName, [ group1, group2 ], { AutoSplit: true } );
   db.createCS( csName, { Domain : domainName } );
   var cl_1 = commCreateCL( db, csName, clName1 );
   var cl_2 = commCreateCL( db, csName, clName2 );
   
   //test a: 执行setAttributes新增shardingKey,其中shardingKey指定一个字段，如{a：1}
   var shardingKey = {a:1};
   cl_1.setAttributes({ShardingKey:shardingKey}); 
   checkAlterResult( csName, clName1, "ShardingKey", shardingKey ); 
   
   //test b: 执行enableSharding开启切分，设置shardingKey指定多个字段，如{a:1，b:-1}
   var shardingKey2 = {a:1,b:-1};
   cl_2.enableSharding({ShardingKey:shardingKey2}); 
   checkAlterResult( csName, clName2, "ShardingKey", shardingKey2 ); 
   
   for( i=0; i<5000; i++ )
   {
      cl_1.insert( {a:i, b:i, c:"sequoiadb test autosplit18348"} );
      cl_2.insert( {a:i, b:i, c:"sequoiadb test autosplit18348"} );
   }
   checkData( csName, clName1, group1, group2, 5000 );
   checkData( csName, clName2, group1, group2, 5000 );
   
   //clean
   commDropCS( db, csName, true, "clean cs" );
   dropDomain( db, domainName );
}

function checkAlterResult(csName, clName, fieldName, expFieldValue)
{
   var clFullName = csName + "." + clName;   
   var cur = db.snapshot(8,{"Name":clFullName});
   var actualFieldValue = cur.current().toObj()[fieldName];

   if (JSON.stringify(expFieldValue) !== JSON.stringify(actualFieldValue))
   {
      throw buildException("test fieldvalue1", "check field", "value is wrong", JSON.stringify(expFieldValue), JSON.stringify(actualFieldValue));
   }
}

function checkData( csName, clName, groupName1, groupName2, expDataNum )
{
   var actDataNum = 0;

   var dataNode1 = new Sdb(db.getRG( groupName1 ).getMaster());
   var checkCL1 = dataNode1.getCS( csName ).getCL( clName );
   var recordNum1 = checkCL1.count();
   actDataNum = actDataNum + recordNum1;
   dataNode1.close();

   var dataNode2 = new Sdb(db.getRG( groupName2 ).getMaster());
   var checkCL2 = dataNode2.getCS( csName ).getCL( clName );
   var recordNum2 = checkCL2.count();
   actDataNum = actDataNum + recordNum2;
   dataNode2.close();

   if( actDataNum !== expDataNum )
   {
      throw buildException("checkData", "check field", "total num is wrong", expDataNum, actDataNum);
   }
}