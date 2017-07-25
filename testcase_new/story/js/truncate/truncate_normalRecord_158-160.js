/******************************************************************************
*@Description: testcases for normal table insert less than/equal/large than
*              page size.
*              testcase: [truncate_07/truncate_08/truncate_09]
*@Modify list:
*              2015-5-13  xiaojun Hu   Init
******************************************************************************/

/*******************************************************************************
*@Description: 测试水平分区表写入大于多个数据页的普通记录，然后再truncate
*@Input: collection.truncate()
*@Expectation: truncate清除数据, 并且清除数据页
********************************************************************************/
function testTruncateSplitCLNormalRecord( db )
{
   var funcName = "testTruncateSplitCLNormalRecord";
   try
   {
      var clName = CHANGEDPREFIX + "_split_normal_record";
      var clOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "hash",
                       "ReplSize": 0 };
      var recordSize = 70000; // large than 65536
      var recordNum = 5;
      commDropCL( db, COMMCSNAME, clName, true, true, "drop cl begin" );
      var cl = commCreateCLByOption( db, COMMCSNAME, clName, clOption, true,
                                     true, false, "create collection begin" );
      var tableName = COMMCSNAME + "." + clName;
      var verJsonObj = { "TotalIndexPages": 4};
      truncateVerify( db, tableName, verJsonObj );
      // split group
      var groups = commGetGroups( db );
      var clGroups = commGetCLGroups( db, tableName );
      if( 1 != clGroups.length )
      {
         println( "expect cl in 1 group, actual cl in " + clGroups.length +
                  " groups" );
         throw "error collection group";
      }
      if( groups.length < 2 )
      {
         println( "don't have enough groups" );
         return ;
      }
      for( var i = 0; i < groups.length; ++i )
      {
         if( clGroups[0] != groups[i][0]["GroupName"] )
         {
            cl.split( clGroups[0], groups[i][0]["GroupName"], 30 );
            break;
         }
      }
      // insert record
      truncateInsertRecord( cl, recordNum, recordSize );

      cl.truncate();
      truncateVerify( db, tableName, verJsonObj );
      commDropCL( db, COMMCSNAME, clName, false, false, "drop cl end" );
   }
   catch( e )
   {
      throw buildException( funcName, e );
   }
   finally
   {
      //commDropCL( db, COMMCSNAME, clName, false, false, "drop cl end" );
   }
}

/*******************************************************************************
*@Description: 测试垂直分区表写入大于多个数据页的普通记录，然后再truncate
*@Input: collection.truncate()
*@Expectation: truncate清除数据, 并且清除数据页
********************************************************************************/
function testTruncateMainSubCLNormalRecord( db )
{
   var funcName = "testTruncateMainSubCLNormalRecord";
   try
   {
      var mainCS = CHANGEDPREFIX + "_largeThanPage_mainCL";
      var subCS1 = CHANGEDPREFIX + "_largeThanPage_subCL1";
      var subCS2 = CHANGEDPREFIX + "_largeThanPage_subCL2";
      var clOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "range",
                       "ReplSize":0, "IsMainCL": true };
      var recordSize = 70000; // large than 65536
      var recordNum = 3;

      commDropCS( db, subCS1, true, "drop sub cs1 begin" );
      commDropCS( db, subCS2, true, "drop sub cs2 begin" );
      commDropCS( db, mainCS, true, "drop main cs begin" );

      var mainCL = commCreateCLByOption( db, mainCS, COMMCLNAME, clOption, true,
                                         true, false, "create collection begin" );
      var subCL1 = commCreateCL( db, subCS1, COMMCLNAME, 0, true, true,
                                 false, "create sub CL1 begin" );
      var subCL2 = commCreateCL( db, subCS2, COMMCLNAME, 0, true, true,
                                 false, "create sub CL2 begin" );
      var subTable1 = subCS1 + "." + COMMCLNAME;
      var subTable2 = subCS2 + "." + COMMCLNAME;
      truncateVerify( db, subTable1 );
      truncateVerify( db, subTable2 );

      mainCL.attachCL( subTable1, {"LowBound":{"ID_Default": 0},
                                   "UpBound": {"ID_Default": 3}} );
      mainCL.attachCL( subTable2, {"LowBound":{"ID_Default": 3},
                                   "UpBound": {"ID_Default": 5}} );
      var tableName = mainCS + "." + COMMCLNAME;  // can see main CL ???

      truncateInsertRecord( mainCL, recordNum, recordSize );
      mainCL.truncate();
      truncateVerify( db, subTable1 );
      truncateVerify( db, subTable2 );
   }
   catch( e )
   {
      throw buildException( funcName, e );
   }
   finally
   {
      commDropCS( db, subCS1, false, "drop sub cs1 end" );
      commDropCS( db, subCS2, false, "drop sub cs2 end" );
      commDropCS( db, mainCS, false, "drop main cs end" );
   }
}

/*******************************************************************************
*@Description: 测试混合分区表写入大于多个数据页的普通记录, 再做truncate.
*@Input: collection.truncate()
*@Expectation: truncate清除数据, 并且清除数据页
********************************************************************************/
function testTruncateMixtureCLNormalRecord( db )
{
   var funcName = "testTruncateMixtureCLNormalRecord";
   try
   {
      var mainCS = CHANGEDPREFIX + "_mixtureCL_largeThanPage_mainCL";
      var subCS1 = CHANGEDPREFIX + "_mixtureCL_largeThanPage_subCL1";
      var subCS2 = CHANGEDPREFIX + "_mixtureCL_largeThanPage_subCL2";
      var mainCLOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "range",
                           "IsMainCL": true };
      var subCLOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "hash",
                          "ReplSize": 0 };
      var recordSize = 70000; // large than 65536
      var recordNum = 5;
      var domainName = CHANGEDPREFIX + "_mixture_domain";
      var verJsonObj = { "TotalIndexPages": 4};

      commDropCS( db, subCS1, true, "drop sub cs1 begin" );
      commDropCS( db, subCS2, true, "drop sub cs2 begin" );
      commDropCS( db, mainCS, true, "drop main cs begin" );
      truncateDropDomain( db, domainName, true );
      // create domain
      var domainRGs = new Array();
      var groups = commGetGroups( db );
      for( var i = 0; i < groups.length; ++i )
      {
         domainRGs[i] = groups[i][0]["GroupName"];
      }
      db.createDomain( domainName, domainRGs, {"AutoSplit": true} );

      var mainCL = commCreateCLByOption( db, mainCS, COMMCLNAME, mainCLOption, true,
                                         true, false, "create collection begin" );
      commCreateCS( db, subCS1, false, "create sub cs1", {"Domain": domainName } );
      var subCL1 = commCreateCLByOption( db, subCS1, COMMCLNAME, subCLOption, true,
                                         true, false, "create collection begin" );
      commCreateCS( db, subCS2, false, "create sub cs2", {"Domain": domainName } );
      var subCL2 = commCreateCLByOption( db, subCS2, COMMCLNAME, subCLOption, true,
                                         true, false, "create collection begin" );
      var subTable1 = subCS1 + "." + COMMCLNAME;
      var subTable2 = subCS2 + "." + COMMCLNAME;
      truncateVerify( db, subTable1, verJsonObj );
      truncateVerify( db, subTable2, verJsonObj );

      mainCL.attachCL( subTable1, {"LowBound":{"ID_Default": 0},
                                   "UpBound": {"ID_Default": 3}} );
      mainCL.attachCL( subTable2, {"LowBound":{"ID_Default": 3},
                                   "UpBound": {"ID_Default": 5}} );


      truncateInsertRecord( mainCL, recordNum, recordSize );
      mainCL.truncate();
      truncateVerify( db, subTable1, verJsonObj );
      truncateVerify( db, subTable2, verJsonObj );
   }
   catch( e )
   {
      throw buildException( funcName, e );
   }
   finally
   {
      commDropCS( db, subCS1, false, "drop sub cs1 end" );
      commDropCS( db, subCS2, false, "drop sub cs2 end" );
      commDropCS( db, mainCS, false, "drop main cs end" );
      truncateDropDomain( db, domainName, false);
   }
}

function main()
{
   if(true==commIsStandalone(db))
   {  
      println("Mode is standalone!");
   }
   else if(commGetGroups(db).length < 2)
   {
      println("data groups number is less than 2! You need at least 2 groups to split!");
   }
	else
   {    
      println( "\n---begin to test <testTruncateSplitCLNormalRecord>" );  
      testTruncateSplitCLNormalRecord( db )
      
      println( "\n---begin to test <testTruncateMainSubCLNormalRecord>" );
      testTruncateMainSubCLNormalRecord( db )
      
      println( "\n---begin to test <testTruncateMixtureCLNormalRecord>" );
      testTruncateMixtureCLNormalRecord( db )     
   }
}

main();
