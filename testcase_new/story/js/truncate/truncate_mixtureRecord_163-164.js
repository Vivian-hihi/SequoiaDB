/******************************************************************************
*@Description: testcases for normal table, and record size
*@Modify list:
*              2015-5-8  xiaojun Hu   Init
******************************************************************************/

/*******************************************************************************
*@Description: 测试普通表写入大于多个数据页的普通记录和大对象记录，然后再truncate
*@Input: collection.truncate()
*@Expectation: truncate清除普通记录和大对象数据, 并且清除普通数据页和大对象数据页
********************************************************************************/
function testTruncateMixtureRecordNormalTbl( db )
{
   var funcName = "testTruncateMixtureRecordNormalTbl";
   try
   {  
      var recordSize = 70000;   // large than 65536
      var recordNum = 2;
      var lobSize = 600000;   // large than 524288
      var lobNumber = 2;
      var tableName = COMMCSNAME + '.' + COMMCLNAME;
      
      commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "clean cl begin")
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, false, "create cl begin" );      
      truncateVerify( db, tableName );
           
      truncatePutLob( cl, lobSize, lobNumber );
      truncateInsertRecord( cl, recordNum, recordSize);
      
      cl.truncate();
      truncateVerify( db, tableName );      
   }
   catch( e )
   {
      throw buildException( funcName, e );
   }
   finally
   {
      commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "clean cl end")
   }
}

/*******************************************************************************
*@Description: 测试混合分区表写入大于多个数据页的普通记录和大对象记录，
*              然后再truncate
*@Input: collection.truncate()
*@Expectation: truncate清除普通记录和大对象数据, 并且清除普通数据页和大对象数据页
********************************************************************************/
function testTruncateMixtureRecordMixtureTbl( db )
{
   var funcName = "testTruncateMixtureRecordMixtureTbl";
   try
   {
      var mainCS = CHANGEDPREFIX + "_mixtureCL_largeThanPage_mainCL";
      var subCS1 = CHANGEDPREFIX + "_mixtureCL_largeThanPage_subCL1";
      var subCS2 = CHANGEDPREFIX + "_mixtureCL_largeThanPage_subCL2";
      var mainCLOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "range",
                           "IsMainCL": true };
      var subCLOption = { "ShardingKey": {"ID_Default": 1}, "ShardingType": "hash",
                          "ReplSize": 0 };
      var domainName = CHANGEDPREFIX + "_mixture_domain";
      var verJsonObj = { "TotalIndexPages": 4};
      var recordSize = 70000;   // large than 65536
      var recordNum = 2;
      var lobSize = 600000;   // large than 524288
      var lobNumber = 2;

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

      // put lob from subCL
      truncatePutLob( subCL1, lobSize, lobNumber );
      truncatePutLob( subCL2, lobSize, lobNumber );
      // insert record from mainCL
      truncateInsertRecord( mainCL, recordNum, recordSize);

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
   println( "\n---begin to test <testTruncateMixtureRecordNormalTbl>" );
   testTruncateMixtureRecordNormalTbl( db );
   
   
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
      println( "\n---begin to test <testTruncateMixtureRecordMixtureTbl>" );
      testTruncateMixtureRecordMixtureTbl( db );      
   }
}

main();
