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
   commDropCS( db, csName, true, "drop CS in the beginning" );

   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   //check cappedCL sharding
   println( "---check sharding---" )
   var hashOptions = { Capped:true, Size:1024, Max:10000000, AutoIndexId:false, ShardingKey:{ "age":1 },ShardingType:"hash", Partition: 1024 };
   checkCreateCLOptions( csName, clName1, hashOptions, true);
	
   var rangeOptions = { Capped:true, Size:1024, Max:10000000, AutoIndexId:false, ShardingKey:{ "age":1 },ShardingType:"hash", Partition: 1024 };
   checkCreateCLOptions( csName, clName2, rangeOptions, true );
   
   //check cappedCL alter
   println( "---check cappedCL alter---" )
   var options = { Capped:true, Size:1024, Max:10000000, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName3, options, false, false, "create capped cl" );
   var alterOption1 = { ShardingKey:{a:1}, ShardingType:"hash" };
   checkCappedAlter( dbcl, alterOption1 );
   var alterOption2 = { ShardingKey:{a:1}, ShardingType:"range" };
   checkCappedAlter( dbcl, alterOption2 );
	checkSnapshot(csName, clName3);
   
   //clean environment after test  
   println( "---end the test---" );
   commDropCS( db, csName, true, "drop CS in the end" );
}

function checkCreateCLOptions( csName, clName, options, isValid)
{
    try
    {
		 db.getCS(csName).createCL(clName,options);
	    if ( isValid == undefined ) 
		 { 
	       throw "NEED_CREATE_FAIL_ERROR";
	    } 
	    println("Create CL with option: " + JSON.stringify(options) + " success!");
    }
   catch(e)
   {
      if( e !== -6)
      {
          throw buildException("Invalid parameter is not -6,error msg is: " + e);
      }
      else
      {
          println("check result success!");
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

function checkSnapshot(csName , clName)
{
	try
   {
		var cl_full_name = csName + '.' + clName;
		var options = {Name : cl_full_name};
      var rec = db.snapshot( 8, options );
      var shardingType = rec.current().toObj().ShardingType;
		var shardingKey = rec.current().toObj().ShardingKey;
		if(shardingType != undefined || shardingKey != undefined)
		{
			throw "CHECK SNAPSHOT FAILED";
		}
   }
   catch( e )
   {
      throw buildException("checkSnapshot()",e,"check snapshot", e, e );
   }
}