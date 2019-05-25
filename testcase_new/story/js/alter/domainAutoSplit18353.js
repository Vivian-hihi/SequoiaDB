/************************************
*@Description: 分区表所属domain设置AutoSplit为true，修改shardingKey和shardingType 
*@author:      wangkexin
*@createdate:  2019.5.24
*@testlinkCase:seqDB-18353
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

   var csName = CHANGEDPREFIX + "_cs_18353";
   var clName1 = CHANGEDPREFIX + "_cl_18353a";
   var clName2 = CHANGEDPREFIX + "_cl_18353b";
   var domainName = "domain18353";

   //clean environment before test
   commDropCS( db, csName, true, "drop CS in the beginning." );
   dropDomain( db, domainName, true, "drop Domain in the beginning." );

   var group1 = allGroupName[0];
   var group2 = allGroupName[1];
   db.createDomain( domainName, [group1], { AutoSplit: true } );
   db.createCS( csName, { Domain : domainName } );

   var subOption1 = {a:1};
   var optionObj1 = {ShardingKey:subOption1, ShardingType:"hash"};
   var cl_1 = commCreateCLByOption( db, csName, clName1, optionObj1);

   var subOption2 = {a:1};
   var optionObj2 = {ShardingKey:subOption2, ShardingType:"range"};
   var cl_2 = commCreateCLByOption( db, csName, clName2, optionObj2);

   //修改domain属性，domain中新增组，如增加组group2
   db.getDomain(domainName).alter({Groups:[group1, group2]});

   //test a: 分区表a修改shardingKey和shardingType，指定shardingType为range
   var shardingKey = {b:1};
   var shardingType = "range";
   cl_1.alter({ShardingKey:shardingKey,ShardingType:shardingType}); 
   checkAlterResult( csName, clName1, "ShardingKey", shardingKey ); 
   checkAlterResult( csName, clName1, "ShardingType", shardingType ); 

   //test b: 分区表b修改shardingKey和shardingType，指定shardingType为hash
   var shardingKey2 = {b:1};
   var shardingType2 = "hash";
   cl_2.alter({ShardingKey:shardingKey2,ShardingType:shardingType2}); 
   checkAlterResult( csName, clName2, "ShardingKey", shardingKey2 ); 
   checkAlterResult( csName, clName2, "ShardingType", shardingType2 );

   for( i=0; i<5000; i++ )
   {
      cl_1.insert( {a:i, b:i, c:"sequoiadb test autosplit18353"} );
      cl_2.insert( {a:i, b:i, c:"sequoiadb test autosplit18353"} );
   }

   checkDataNotSplit( csName, clName1, group1, 5000 );
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

function checkDataNotSplit( csName, clName, groupName, expDataNum )
{
   var actDataNum = 0;

   var dataNode = new Sdb(db.getRG( groupName ).getMaster());
   var checkCL = dataNode.getCS( csName ).getCL( clName );
   actDataNum = actDataNum + checkCL.count();
   if( actDataNum !== expDataNum )
   {
      throw buildException("checkData", "check field", "total num is wrong", expDataNum, actDataNum);
   }
   dataNode.close();
}