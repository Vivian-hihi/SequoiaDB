/************************************
*@Description: 创建固定集合空间集合，参数Max校验
*@author:      luweikang
*@createdate:  2017.7.11
*@testlinkCase:seqDB-11824
**************************************/

main()

function main()
{
   var csName = CHANGEDPREFIX + "_11824_CS";

   var clName1 = CHANGEDPREFIX + "_11824_CL1";
   var clName2 = CHANGEDPREFIX + "_11824_CL2";
   var clName3 = CHANGEDPREFIX + "_11824_CL3";
   var clName4 = CHANGEDPREFIX + "_11824_CL4";
   var clName5 = CHANGEDPREFIX + "_11824_CL5";
   var clName6 = CHANGEDPREFIX + "_11824_CL6";
   var clName7 = CHANGEDPREFIX + "_11824_CL7";
   
   //clean and createCS CL before test
   println( "---begin test---" );
   initCappedCS( csName );
   
   
   //check cappedCL Capped
   println("---check cl Max---")
   
   //check Max : 10000000
   var options1 = { Capped : true, Size:1024, Max:10000000, AutoIndexId:false };
   checkCreateCLOptions( csName, clName1, options1, true );
   
   //check Max : -1
   var options2 = { Capped : true, Size:1024, Max:-1, AutoIndexId:false };
   checkCreateCLOptions( csName, clName2, options2, false );
   
   //check Max : 0
   var options3 = { Capped : true, Size:1024, Max:0, AutoIndexId:false };
   checkCreateCLOptions( csName, clName3, options3, true );
   
   //check Max : 132.456,单机创建成功
   var options4 = { Capped : true, Size:1024, Max:123.456, AutoIndexId:false };
   checkCreateCLOptions( csName, clName4, options4, true );
   
   //check Max : "abc"
   var options5 = { Capped : true, Size:1024, Max:"abc", AutoIndexId:false };
   checkCreateCLOptions( csName, clName5, options5, false );
     
   //check 不指定Max,单机创建成功
   var options6 = { Capped : true, Size:1024, AutoIndexId:false };
   checkCreateCLOptions( csName, clName6, options6, true );
   
   //check Mx,单机创建成功
   var options7 = { Capped : true, Size:1024, Mx:10000000, AutoIndexId:false };
   checkCreateCLOptions( csName, clName7, options7, true );
   
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