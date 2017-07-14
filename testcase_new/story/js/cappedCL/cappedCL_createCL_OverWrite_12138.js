/************************************
*@Description: 创建固定集合空间集合，参数OverWrite校验
*@author:      luweikang
*@createdate:  2017.7.14
*@testlinkCase:seqDB-12138
**************************************/

main()

function main()
{
   var csName = CHANGEDPREFIX + "_12138_CS";

   var clName1 = CHANGEDPREFIX + "_12138_CL1";
   var clName2 = CHANGEDPREFIX + "_12138_CL2";
   var clName3 = CHANGEDPREFIX + "_12138_CL3";
   var clName4 = CHANGEDPREFIX + "_12138_CL4";
   var clName5 = CHANGEDPREFIX + "_12138_CL5";
   var clName6 = CHANGEDPREFIX + "_12138_CL6";
   var clName7 = CHANGEDPREFIX + "_12138_CL7";
   
   //clean and createCS CL before test
   println( "---begin test---" );
   initCappedCS( csName );
   
   
   //check cappedCL Capped
   println("---check cl Max---")
   
   //check OverWrite : true
   var options1 = { Capped : true, Size:1024000000, Max:10000000, AutoIndexId:false, OverWrite:true };
   checkCreateCLOptions( csName, clName1, options1, true );
   
   //check OverWrite : 1
   var options2 = { Capped : true, Size:1024000000, Max:10000000, AutoIndexId:false, OverWrite:1 };
   checkCreateCLOptions( csName, clName2, options2, false );
   
   //check OverWrite : 0
   var options3 = { Capped : true, Size:1024000000, Max:10000000, AutoIndexId:false, OverWrite:0 };
   checkCreateCLOptions( csName, clName3, options3, false );
   
   //check OverWrite : false
   var options4 = { Capped : true, Size:1024000000, Max:10000000, AutoIndexId:false, OverWrite:false };
   checkCreateCLOptions( csName, clName4, options4, true );
   
   //check OverWrite : "abc"
   var options5 = { Capped : true, Size:1024000000, Max:10000000, AutoIndexId:false, OverWrite:"abc" };
   checkCreateCLOptions( csName, clName5, options5, false );
     
   //check OverWrite:123.456
   var options6 = { Capped : true, Size:1024000000, Max:10000000, AutoIndexId:false, OverWrite:123.456};
   checkCreateCLOptions( csName, clName6, options6, false );
   
   //check OvWre:true
   var options7 = { Capped : true, Size:1024000000, Max:10000000, AutoIndexId:false, OvWre:true };
   checkCreateCLOptions( csName, clName7, options7, false );
   
   //clean environment after test  
   println( "---end the test---" );
   commDropCS( db, csName, true, "drop CS in the end" );
}

function checkCreateCLOptions( csName, clName, options, result )
{
   try
   {
      db.getCS( csName ).createCL( clName, options );
      if( result !== true ){
         throw "ERR_CREATE_CAPPEDCL"
      }
   }
   catch( e )
   {
      if( e !== -6 )
      {
         throw buildException("checkCreateCLOptions()",e,"create cappedCL", "-6", e);
      }
   }
}