/************************************
*@Description: 创建固定集合，并创建索引
*@author:      luweikang
*@createdate:  2017.7.7
*@testlinkCase:seqDB-11830
**************************************/

main();

function main()
{
   var csName = CHANGEDPREFIX + "_11830_CS";
   var clName = CHANGEDPREFIX + "_11830_CL";
   
   //drop CS and create cappedCS
   println("---begin test---")
   initCappedCS( csName );
   
   //create cappedCL
   var optionObj = {Capped:true, Size:1024, Max:10000000, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, optionObj, false, false, "create cappedCL" );
   
   //createIndex
   println("---createIndex---")
   try
   {
      dbcl.createIndex( "ageIndex", { age: 1 }, true );
   }
   catch( e )
   {
      if( e !== -32 )
      {
         throw buildException( "cappedCL createIndex", e, "cappedCL createIndex", "-32", e );
      }
   }
   
   //clean environment after test
   println( "---end the test---" );
   commDropCS( db, csName, true, "drop CS in the end" );
}