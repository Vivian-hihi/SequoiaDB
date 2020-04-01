/************************************
*@description： seqDB-21951:主子表，主表查询命中多个子表且命中同一个数据组
*@author ：2020-3-31 liyuanyue
**************************************/
testConf.skipStandAlone = true;
main( test );

function test ()
{
   if( commGetGroupsNum( db ) < 2 )
   {
      println( "---Least two groups" );
      return;
   }

   var dataGroupName = commGetDataGroupNames( db );

   var mCLName = CHANGEDPREFIX + "_split_m21951";
   var sCLName1 = CHANGEDPREFIX + "_split_s21951_1";
   var sCLName2 = CHANGEDPREFIX + "_split_s21951_2";
   var sCLName3 = CHANGEDPREFIX + "_split_s21951_3";
   var sCLName4 = CHANGEDPREFIX + "_split_s21951_4";

   commDropCL( db, COMMCSNAME, mCLName );
   commDropCL( db, COMMCSNAME, sCLName1 );
   commDropCL( db, COMMCSNAME, sCLName2 );
   commDropCL( db, COMMCSNAME, sCLName3 );
   commDropCL( db, COMMCSNAME, sCLName4 );

   var options = { ShardingKey: { a: 1 }, ShardingType: "range", IsMainCL: true };
   var mcl = commCreateCL( db, COMMCSNAME, mCLName, options );
   var options = { ShardingKey: { a: 1 }, ShardingType: "hash", Group: dataGroupName[0] };
   commCreateCL( db, COMMCSNAME, sCLName1, options );
   var options = { ShardingKey: { a: 1 }, ShardingType: "hash", Group: dataGroupName[0] };
   commCreateCL( db, COMMCSNAME, sCLName2, options );
   var options = { ShardingKey: { a: 1 }, ShardingType: "hash", Group: dataGroupName[1] };
   commCreateCL( db, COMMCSNAME, sCLName3, options );
   var options = { ShardingKey: { a: 1 }, ShardingType: "hash", Group: dataGroupName[1] };
   commCreateCL( db, COMMCSNAME, sCLName4, options );

   mcl.attachCL( COMMCSNAME + "." + sCLName1, { LowBound: { a: 0 }, UpBound: { a: 10 } } );
   mcl.attachCL( COMMCSNAME + "." + sCLName2, { LowBound: { a: 10 }, UpBound: { a: 20 } } );
   mcl.attachCL( COMMCSNAME + "." + sCLName3, { LowBound: { a: 20 }, UpBound: { a: 30 } } );
   mcl.attachCL( COMMCSNAME + "." + sCLName4, { LowBound: { a: 30 }, UpBound: { a: 40 } } );

   for( var i = 0; i < 40; i++ )
   {
      mcl.insert( { a: i } );
   }

   // find
   var cur = mcl.find( { a: { $lt: 20 } } );
   var expResult = 20;
   var actResult = 0;
   while( cur.next() )
   {
      actResult++;
   }
   if( expResult != actResult )
   {
      throw new Error( "expected " + expResult + ",but actually " + actResult );
   }

   // explain
   var explainResult = mcl.find( { a: { $lt: 20 } } ).explain( { Detail: true, Run: true } );
   var actResult = explainResult.current().toObj().PlanPath.Run.ReturnNum;
   if( expResult != actResult )
   {
      throw new Error( "expected " + expResult + ",but actually " + actResult );
   }
   var childOperators = explainResult.current().toObj().PlanPath.ChildOperators;
   if( ( childOperators.length !== 1 ) || ( childOperators[0].GroupName !== dataGroupName[0] ) )
   {
      throw new Error( "explain PlanPath find GroupName error" );
   }

   commDropCL( db, COMMCSNAME, mCLName );
   commDropCL( db, COMMCSNAME, sCLName1 );
   commDropCL( db, COMMCSNAME, sCLName2 );
   commDropCL( db, COMMCSNAME, sCLName3 );
   commDropCL( db, COMMCSNAME, sCLName4 );
}
