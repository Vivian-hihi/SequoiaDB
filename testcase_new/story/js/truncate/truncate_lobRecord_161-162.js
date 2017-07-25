/******************************************************************************
*@Description: test large object record put in split table and mixture table(
*              split and main sub cl), then truncate.[truncate_10/truncate_11]
*@Modify list:
*              2015-5-13  xiaojun Hu   Init
******************************************************************************/

/*******************************************************************************
*@Description: 测试垂直分区表写入大于多个数据页的大对象记录，然后再truncate
*@Input: collection.truncate()
*@Expectation: truncate清除大对象数据, 并且清除大对象数据页
********************************************************************************/
function testTruncateSplitDomainLOB( db )
{
   var funcName = "testTruncateSplitDomainLOB";
   try
   {
      var csName = CHANGEDPREFIX + "_Split_Domain_Lob";
      var lobSize = 600000;   // large than 524288
      var lobNumber = 2;
      var verJsonObj = { "TotalIndexPages": 4};
      var clOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "hash",
                          "ReplSize": 0 };
      var domainName = CHANGEDPREFIX + "_split_domain_lob";

      commDropCS( db, csName, true, "drop sub cs1 begin" );
      truncateDropDomain( db, domainName, true );
      // create domain
      var domainRGs = new Array();
      var groups = commGetGroups( db );
      for( var i = 0; i < groups.length; ++i )
      {
         domainRGs[i] = groups[i][0]["GroupName"];
      }
      db.createDomain( domainName, domainRGs, {"AutoSplit": true} );

      commCreateCS( db, csName, false, "create sub cs1", {"Domain": domainName } );
      var cl = commCreateCLByOption( db, csName, COMMCLNAME, clOption, true,
                                         true, false, "create collection begin" );

      var tableName = csName + "." + COMMCLNAME;
      truncateVerify( db, tableName, verJsonObj );

      truncatePutLob( cl, lobSize, lobNumber );
      cl.truncate();
      truncateVerify( db, tableName, verJsonObj );

   }
   catch( e )
   {
      throw buildException( funcName, e );
   }
   finally
   {
      commDropCS( db, csName, false, "drop collection end" );
   }
}

/*******************************************************************************
*@Description: 测试混合分区表写入大于多个数据页的大对象记录，然后再truncate
*              写大对象需要直接从子表写入，truncate则可以从主表做
*@Input: collection.truncate()
*@Expectation: truncate清除大对象数据, 并且清除大对象数据页
********************************************************************************/
function testTruncateMixtureCLLOB( db )
{
   var funcName = "testTruncateMixtureCLLOB";
   try
   {
      var mainCS = CHANGEDPREFIX + "_mixtureCL_largeThanPage_mainCL";
      var subCS1 = CHANGEDPREFIX + "_mixtureCL_largeThanPage_subCL1";
      var subCS2 = CHANGEDPREFIX + "_mixtureCL_largeThanPage_subCL2";
      var mainCLOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "range",
                           "IsMainCL": true };
      var subCLOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "hash",
                          "ReplSize": 0 };
      var lobSize = 600000;   // large than 524288
      var lobNumber = 2;
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

      truncatePutLob( subCL1, lobSize, lobNumber );
      truncatePutLob( subCL2, lobSize, lobNumber );

      mainCL.attachCL( subTable1, {"LowBound":{"ID_Default": 0},
                                   "UpBound": {"ID_Default": 3}} );
      mainCL.attachCL( subTable2, {"LowBound":{"ID_Default": 3},
                                   "UpBound": {"ID_Default": 5}} );

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
      commDropCS( db, subCS1, true, "drop sub cs1 end" );
      commDropCS( db, subCS2, true, "drop sub cs2 end" );
      commDropCS( db, mainCS, true, "drop main cs end" );
      truncateDropDomain( db, domainName, false );
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
      println( "\n---begin to test <testTruncateSplitDomainLOB>" );
      testTruncateSplitDomainLOB( db )
      
//      println( "\n---begin to test <testTruncateMixtureCLLOB>" );
//      testTruncateMixtureCLLOB( db )     
   }
}

main();
