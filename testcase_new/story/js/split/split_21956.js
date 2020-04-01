/************************************
*@description：seqDB-21956:分区表，包含非分区键查询
*@author ：2020-3-31 liyuanyue
**************************************/
testConf.skipStandAlone = true;
main( test );

function test ()
{
   var hashCLName = CHANGEDPREFIX + "_split_21956_hash";
   var rangeCLName = CHANGEDPREFIX + "_split_21956_range";
   var mCLName = CHANGEDPREFIX + "_split_21956_main";
   var sCLName = CHANGEDPREFIX + "_split_21956_sub";

   commDropCL( db, COMMCSNAME, hashCLName );
   commDropCL( db, COMMCSNAME, rangeCLName );
   commDropCL( db, COMMCSNAME, mCLName );
   commDropCL( db, COMMCSNAME, sCLName );

   // create cl
   var option = { ShardingKey: { a: 1 }, ShardingType: "hash", Partition: 1024 };
   var hashCL = commCreateCL( db, COMMCSNAME, hashCLName, option, false );
   var option = { ShardingKey: { a: 1 }, ShardingType: "range" };
   var rangeCL = commCreateCL( db, COMMCSNAME, rangeCLName, option, false );
   var option = { ShardingKey: { a: 1 }, ShardingType: "range", IsMainCL: true };
   var mcl = commCreateCL( db, COMMCSNAME, mCLName, option, false );
   var option = { ShardingKey: { a: 1 }, ShardingType: "hash" };
   commCreateCL( db, COMMCSNAME, sCLName, option, false );

   mcl.attachCL( COMMCSNAME + "." + sCLName, { LowBound: { a: 0 }, UpBound: { a: 10 } } );

   // insert
   var recordsNumber = 3;
   for( var i = 1; i <= recordsNumber; i++ )
   {
      hashCL.insert( { a: i, b: 1 } );
      rangeCL.insert( { a: i, b: 1 } );
      mcl.insert( { a: i, b: 1 } );
   }

   // hash cl find
   var hashcur = hashCL.find( { b: 1 } );
   var hashRecords = 0;
   while( hashcur.next() )
   {
      hashRecords++;
   }
   if( hashRecords != recordsNumber )
   {
      throw new Error( "expected result " + recordsNumber + ",but actually result " + hashRecords );
   }
   var hashcur = hashCL.find( { a: { $gt: 1 }, b: 1 } );
   var hashRecords = 0;
   while( hashcur.next() )
   {
      hashRecords++;
   }
   if( hashRecords != ( recordsNumber - 1 ) )
   {
      throw new Error( "expected result " + ( recordsNumber - 1 ) + ",but actually result " + hashRecords );
   }

   // range cl find
   var rangecur = rangeCL.find( { b: 1 } );
   var rangeRecords = 0;
   while( rangecur.next() )
   {
      rangeRecords++;
   }
   if( rangeRecords != recordsNumber )
   {
      throw new Error( "expected result " + recordsNumber + ",but actually result " + rangeRecords );
   }
   var rangecur = rangeCL.find( { a: { $gt: 1 }, b: 1 } );
   var rangeRecords = 0;
   while( rangecur.next() )
   {
      rangeRecords++;
   }
   if( rangeRecords != ( recordsNumber - 1 ) )
   {
      throw new Error( "expected result " + ( recordsNumber - 1 ) + ",but actually result " + rangeRecords );
   }

   // main_sub cl find
   var mcur = mcl.find( { b: 1 } );
   var mRecords = 0;
   while( mcur.next() )
   {
      mRecords++;
   }
   if( mRecords != recordsNumber )
   {
      throw new Error( "expected result " + recordsNumber + ",but actually result " + mRecords );
   }
   var mcur = mcl.find( { a: { $gt: 1 }, b: 1 } );
   var mRecords = 0;
   while( mcur.next() )
   {
      mRecords++;
   }
   if( mRecords != ( recordsNumber - 1 ) )
   {
      throw new Error( "expected result " + ( recordsNumber - 1 ) + ",but actually result " + mRecords );
   }

   // hash cl explain
   var expRec = recordsNumber;  // 3
   var hashExplainResult = hashCL.find( { b: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = hashExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== expRec )
   {
      throw new Error( "expected " + expRec + ",but actually " + actRec );
   }
   var hashExplainResult = hashCL.find( { a: { $gt: 1 }, b: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = hashExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== ( expRec - 1 ) )
   {
      throw new Error( "expected " + ( expRec - 1 ) + ",but actually " + actRec );
   }

   // range cl explain
   var rangeExplainResult = rangeCL.find( { b: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = rangeExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== expRec )
   {
      throw new Error( "expected " + expRec + ",but actually " + actRec );
   }
   var rangeExplainResult = rangeCL.find( { a: { $gt: 1 }, b: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = rangeExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== ( expRec - 1 ) )
   {
      throw new Error( "expected " + ( expRec - 1 ) + ",but actually " + actRec );
   }

   // main_sub cl explain
   var mExplainResult = mcl.find( { b: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = mExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== expRec )
   {
      throw new Error( "expected " + expRec + ",but actually " + actRec );
   }
   var mExplainResult = mcl.find( { a: { $gt: 1 }, b: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = mExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== ( expRec - 1 ) )
   {
      throw new Error( "expected " + ( expRec - 1 ) + ",but actually " + actRec );
   }

   commDropCL( db, COMMCSNAME, hashCLName );
   commDropCL( db, COMMCSNAME, rangeCLName );
   commDropCL( db, COMMCSNAME, mCLName );
   commDropCL( db, COMMCSNAME, sCLName );
}
