/************************************
*@Description: 创建固定集合空间集合，参数IsMainCL校验
*@author:      csq
*@createdate:  2018.09.11
*@testlinkCase:seqDB-15785
**************************************/

main()

function main()
{
   var csName = CHANGEDPREFIX + "_15785_CS";

   var clName1 = CHANGEDPREFIX + "_15785_CL1";
   
   //clean and createCS CL before test
   println( "---begin test---" );
   initCappedCS( csName );
   
   
   //check cappedCL Capped
   println("---check cl IsMainCL---")
   
   //check IsMainCL : true
   var options1 = { Capped : true, Size:1024, IsMainCL:true, ShardingKey:{a:1}};
   checkCreateCLOptions( csName, clName1, options1, true );
   
   commDropCS( db, csName, true, "drop subcs in the end" );   
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