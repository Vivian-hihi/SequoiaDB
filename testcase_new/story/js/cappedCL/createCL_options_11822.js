/************************************
*@Description: 创建固定集合空间集合，参数校验
*@author:      luweikang
*@createdate:  2017.7.11
*@testlinkCase:seqDB-11822
**************************************/

main()

function main()
{
   var csName = CHANGEDPREFIX + "_11822_CS";

   var clName1 = CHANGEDPREFIX + "_11822_CL1";
   var clName2 = CHANGEDPREFIX + "_11822_CL2";
   var clName3 = CHANGEDPREFIX + "_11822_CL3";
   var clName4 = CHANGEDPREFIX + "_11822_CL4";
   var clName5 = CHANGEDPREFIX + "_11822_CL5";
   var clName6 = CHANGEDPREFIX + "_11822_CL6";
   var clName7 = CHANGEDPREFIX + "_11822_CL7";
   
   //clean and createCS CL before test
   println( "---begin test---" );
   initCappedCS( csName );
   
   
   //check cappedCL Capped
   println("---check cl Capped---")
   
   //check Capped : true
   var options1 = { Capped : true, Size:1024, Max:10000000, AutoIndexId:false };
   checkCreateCLOptions( csName, clName1, options1, true );
   
   //check Capped : false
   var options2 = { Capped : false, Size:1024, Max:10000000, AutoIndexId:false };
   checkCreateCLOptions( csName, clName2, options2, false );
   
   //check Capped
   var options3 = { Capped : "", Size:1024, Max:10000000, AutoIndexId:false };
   checkCreateCLOptions( csName, clName3, options3, false );
   
   //check Capped : "abc"
   var options4 = { Capped : "abc", Size:1024, Max:10000000, AutoIndexId:false };
   checkCreateCLOptions( csName, clName4, options4, false );
   
   //check options = ""
   var options5 = { Size:1024, Max:10000000, AutoIndexId:false };
   checkCreateCLOptions( csName, clName5, options5, false );
     
   //check Caped
   var options6 = { Caped : true, Size:1024, Max:10000000, AutoIndexId:false };
   checkCreateCLOptions( csName, clName6, options6, false );
   
   //check no Capped
   var options7 = { Size:1024, Max:10000000, AutoIndexId:false };
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
      if( e !== -6 && e !== -315 )
      {
         throw buildException("checkCreateCLOptions()",e,"create cappedCL", "-6",clName+":"+e);
      }
   }
}