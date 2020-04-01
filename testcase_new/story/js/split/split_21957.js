/************************************
*@description：seqDB-21957:分区表，不带匹配符精确匹配 
*@author ：2020-3-31 liyuanyue
**************************************/
testConf.skipStandAlone = true;
main( test );

function test ()
{
   var hashCLName = CHANGEDPREFIX + "_split_21957_hash";
   var rangeCLName = CHANGEDPREFIX + "_split_21957_range";
   var mCLName = CHANGEDPREFIX + "_split_21957_main";
   var sCLName = CHANGEDPREFIX + "_split_21957_sub";

   commDropCL( db, COMMCSNAME, hashCLName );
   commDropCL( db, COMMCSNAME, rangeCLName );
   commDropCL( db, COMMCSNAME, mCLName );
   commDropCL( db, COMMCSNAME, sCLName );

   var option = { ShardingKey: { a: 1 }, ShardingType: "hash", Partition: 1024 };
   var hashCL = commCreateCL( db, COMMCSNAME, hashCLName, option, false );
   var option = { ShardingKey: { a: 1 }, ShardingType: "range" };
   var rangeCL = commCreateCL( db, COMMCSNAME, rangeCLName, option, false );
   var option = { ShardingKey: { a: 1 }, ShardingType: "range", IsMainCL: true };
   var mcl = commCreateCL( db, COMMCSNAME, mCLName, option, false );
   var option = { ShardingKey: { a: 1 }, ShardingType: "hash" };
   commCreateCL( db, COMMCSNAME, sCLName, option, false );

   mcl.attachCL( COMMCSNAME + "." + sCLName, { LowBound: { a: 0 }, UpBound: { a: 10 } } );

   hashCL.insert( { a: 1 } );
   rangeCL.insert( { a: 1 } );
   mcl.insert( { a: 1 } );

   var hashcur = hashCL.find( { a: 1 } );
   if( !( hashcur.next() ) )
   {
      throw new Error( "find error" );
   }

   var rangecur = rangeCL.find( { a: 1 } );
   if( !( rangecur.next() ) )
   {
      throw new Error( "find error" );
   }

   var maincur = mcl.find( { a: 1 } );
   if( !( maincur.next() ) )
   {
      throw new Error( "find error" );
   }

   var expRec = 1;
   var hashExplainResult = hashCL.find( { a: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = hashExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== expRec )
   {
      throw new Error( "expected 1,but actually " + actRec );
   }

   var rangeExplainResult = rangeCL.find( { a: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = rangeExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== expRec )
   {
      throw new Error( "expected 1,but actually " + actRec );
   }

   var mExplainResult = mcl.find( { a: 1 } ).explain( { Detail: true, Run: true } );
   var actRec = mExplainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( actRec !== expRec )
   {
      throw new Error( "expected 1,but actually " + actRec );
   }

   commDropCL( db, COMMCSNAME, hashCLName );
   commDropCL( db, COMMCSNAME, rangeCLName );
   commDropCL( db, COMMCSNAME, mCLName );
   commDropCL( db, COMMCSNAME, sCLName );
}
