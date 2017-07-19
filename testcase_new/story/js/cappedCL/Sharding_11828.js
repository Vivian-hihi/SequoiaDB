/************************************
*@Description: 创建固定集合空间集合，find查询数据 
*@author:      luweikang
*@createdate:  2017.7.12
*@testlinkCase:seqDB-11828
**************************************/

main();

function main()
{
   if( true === commIsStandalone( db ) )
   {
      println( "mode is standalone" );
      return;
   }
   
   var csName = CHANGEDPREFIX + "_11828_CS";
   var clName1 = CHANGEDPREFIX + "_11828_CL1";
   var clName2 = CHANGEDPREFIX + "_11828_CL2";
   var clName3 = CHANGEDPREFIX + "_11828_CL3";
   
   //clean and createCS CL before test
   println( "---begin test---" );
   initCappedCS( csName );
   
   //check cappedCL sharding
   println( "---check sharding---" )
   var hashOptions = { Capped:true, Size:1024000000, Max:10000000, AutoIndexId:false, ShardingKey:{ "age":1 },ShardingType:"hash", Partition: 1024 };
   createCappedCL( csName, clName1, hashOptions );
   var rangeOptions = { Capped:true, Size:1024000000, Max:10000000, AutoIndexId:false, ShardingKey:{ "age":1 },ShardingType:"hash", Partition: 1024 };
   createCappedCL( csName, clName2, rangeOptions );
   
   //check cappedCL alter
   println( "---check cappedCL alter---" )
   var options = { Capped:true, Size:1024000000, Max:10000000, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName3, options, false, false, "create capped cl" );
   var alterOption1 = { ShardingKey:{a:1}, ShardingType:"hash" };
   checkCappedAlter( dbcl, alterOption1 );
   var alterOption2 = { ShardingKey:{a:1}, ShardingType:"range" };
   checkCappedAlter( dbcl, alterOption2 );
   
   //clean environment after test  
   println( "---end the test---" );
   commDropCS( db, csName, true, "drop CS in the end" );
}

function createCappedCL( csName, clName, options )
{
   try
   {
      db.getCS( csName ).createCL( clName, options );
      throw "ERR_CREATE_CAPPEDCL";
   }
   catch( e )
   {
      if( e !== -32 )
      {
         throw buildException("createCappedCL()",e,"create cappedCL", "-32", e );
      }
   }
}

function checkCappedAlter( dbcl, options )
{
   try
   {
      dbcl.alter( options );
      throw "ERR_ALTER_CAPPEDCL";
   }
   catch( e )
   {
      if( e !== -32 )
      {
         throw buildException("checkCappedAlter()",e,"check cappedCL alter", "-32", e );
      }
   }
}