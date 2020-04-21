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

   var mcl = commCreateCL( db, COMMCSNAME, mCLName, { ShardingKey: { a: 1 }, IsMainCL: true }, false );
   commCreateCL( db, COMMCSNAME, sCLName, { ShardingKey: { a: 1 } }, false );

   mcl.attachCL( COMMCSNAME + "." + sCLName, { LowBound: { a: 0 }, UpBound: { a: 20 } } );

   mcl.insert( { a: 5 } );

   var findCond = { "a": 200 };
   commCompareResults( mcl.find( findCond ), [] );
   checkHitDataGroups( mcl.find( findCond ).explain( { "Run": true } ), ['SYSCoord'] );

   commDropCL( db, COMMCSNAME, mCLName );
}