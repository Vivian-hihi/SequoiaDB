/************************************
*@description：seqDB-21952:主子表，主表查询的数据未命中子表
*@author ：2020-3-16 liyuanyue
**************************************/
testConf.skipStandAlone = true;
main( test );

function test ()
{
   var mCLName = CHANGEDPREFIX + "_split_m21952";
   var sCLName = CHANGEDPREFIX + "_split_s21952";

   commDropCL( db, COMMCSNAME, mCLName );
   commDropCL( db, COMMCSNAME, sCLName );

   var options = { ShardingKey: { a: 1 }, ShardingType: "range", IsMainCL: true };
   var mcl = commCreateCL( db, COMMCSNAME, mCLName, options );
   var options = { ShardingKey: { a: 1 }, ShardingType: "hash" };
   commCreateCL( db, COMMCSNAME, sCLName, options );

   mcl.attachCL( COMMCSNAME + "." + sCLName, { LowBound: { a: 0 }, UpBound: { a: 200 } } );

   // insert
   mcl.insert( { a: 1 } );

   var cur = mcl.find( { a: 20 } );
   if( cur.next() )
   {
      throw new Error( "find error" );
   }

   // expalin
   var explainResult = mcl.find( { a: 20 } ).explain( { Detail: true, Run: true } );
   var actRec = explainResult.current().toObj().PlanPath.Run.ReturnNum;
   var expRec = 0;
   if( actRec !== expRec )
   {
      throw new Error( "expected 0,but actually " + actRec );
   }

   commDropCL( db, COMMCSNAME, mCLName );
   commDropCL( db, COMMCSNAME, sCLName );
}
