/******************************************************************************
@Description: seqDB-10442:数据落在相同组上的相同子表中，批量插入数据
@modify list:
   2016.11.24  zengxianquan  Init
   2019-4-15   xiaoni huang  modify
*******************************************************************************/

main();
function main()
{
   if( true == commIsStandalone( db ) )
   {
      println( "---Is standalone." );
      return;
   } 
   if ( commGetGroupsNum( db ) < 1 )
   {
      println("---Least two groups");
      return ;
   }
   db.setSessionAttr( { PreferedInstance: "M" } );
   
   var mclName  = "mcl_10442" ;
   var sclName1 = "scl_10442_1" ;
   var sclName2 = "scl_10442_2" ;
   var groups = commGetGroups(db, false, "", false, true, true );
   var rgName = groups[1][0].GroupName;

   commDropCL( db, COMMCSNAME, mclName, true, true, "clean main cl" );
   commDropCL( db, COMMCSNAME, sclName1, true, true, "clean sub cl1" );
   commDropCL( db, COMMCSNAME, sclName2, true, true, "clean sub cl2" );
   
   // create main cl
   println("\n---Begin to create cl.");
   var mOpt = { ShardingKey:{a:1}, IsMainCL:true };
   var mainCL = commCreateCLByOption( db, COMMCSNAME, mclName, mOpt, true, true );
   // create sub cl
   var subCLs = [];
   var sOpt = { ShardingKey:{ a:1 }, ShardingType: "hash", ReplSize:0, Compressed:true, Group: rgName };
   var subCL1 = commCreateCLByOption( db, COMMCSNAME, sclName1, { Group: rgName }, true, true );
   var sOpt = { ShardingKey:{ a:1 }, ShardingType: "range", ReplSize:0, Compressed:true, Group: rgName };
   var subCL2 = commCreateCLByOption( db, COMMCSNAME, sclName2, sOpt, true, true ); 
   subCLs.push( subCL1 ) ;
   subCLs.push( subCL2 ) ;   
   // attach cl
   println("\n---Begin to attach cl.");
   mainCL.attachCL( COMMCSNAME + "." + sclName1, { LowBound:{a:0},   UpBound:{a:1000} } ) ;
   mainCL.attachCL( COMMCSNAME + "." + sclName2, { LowBound:{a:1000},UpBound:{a:2000} } ) ;
      
   // insert   
   println("\n---Begin to insert.");
   var recordsNum = 2000;
   var docs = [];
   for(var i = 0; i < recordsNum ; ++i )
   {
      docs.push( {a: i} );
   }
   mainCL.insert( docs );

   // check results
   println("\n---Begin to check total count.");
   var totalCnt = mainCL.count();
   if( Number( totalCnt ) !== recordsNum ) 
   {
      throw buildException( "main", null, "check total count", recordsNum, totalCnt );
   }
   
   // check rg count
   println("\n---Begin to check subCL1 in rg.");
   var sclFullName = COMMCSNAME + "." + sclName1;
   var clRGs = db.snapshot(8, { Name: sclFullName }).current().toObj().CataInfo;
   if( clRGs.length !== 1 ) 
   {
      throw buildException( "main", null, "check subCL1 rgNum", 1, clRGs.length );
   }   
   var rg = db.getRG( rgName ).getMaster().connect();
   var cnt = rg.getCS( COMMCSNAME ).getCL( sclName1 ).count();
   var expCnt = recordsNum / 2;
   if( Number( cnt ) !== expCnt ) 
   {
      throw buildException( "main()", null, "check subCL1 count in rg", expCnt, cnt );
   }
   
   println("\n---Begin to check subCL2 in rg.");
   var sclFullName = COMMCSNAME + "." + sclName2;
   var clRGs = db.snapshot(8,{ Name: sclFullName }).current().toObj().CataInfo;
   if( clRGs.length !== 1 ) 
   {
      throw buildException( "main", null, "check subCL2 rgNum", 1, clRGs.length );
   }   
   var rg = db.getRG( rgName ).getMaster().connect();
   var cnt = rg.getCS( COMMCSNAME ).getCL( sclName2 ).count();
   if( Number( cnt ) !== expCnt ) 
   {
      throw buildException( "main()", null, "check subCL1 count in rg", expCnt, cnt );
   }
   
   
   commDropCL( db, COMMCSNAME, mclName, true, true, "clean main cl in the end" );
}