/************************************
*@Description: 创建固定集合，指定AutoIndexId ,压缩的相关参数
*@author:      luweikang
*@createdate:  2017.7.6
*@testlinkCase:seqDB-11769,seqDB-11893
**************************************/

main();

function main()
{
   var csName = CHANGEDPREFIX + "_11769_CS";
   var clName = CHANGEDPREFIX + "_11769_CL";
   
   //clean environment before test
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
   //create cappedCS
   println( "begin to create cappedCS" );
   var options = {Capped:true};
   commCreateCS( db, csName, false, "create cappedCS", options );
   
   //create cappedCL AutoIndexId:true
   var optionObj1 = {Capped:true, Size:1024000000, Max:10000000, AutoIndexId:true};
   createCappedCL( db, csName, clName+1, optionObj1);
   
   //create cappedCL Compressed:true
   var optionObj2 = {Capped:true, Size:1024000000, Max:10000000, AutoIndexId:true, Compressed:true};
   createCappedCL( db, csName, clName+2, optionObj2);
   
   //create cappedCL Compressed:false
   var optionObj3 = {Capped:true, Size:1024000000, Max:10000000, AutoIndexId:true, Compressed:false};
   createCappedCL( db, csName, clName+3, optionObj3);
   
   //create cappedCL CompressionType:"snappy"
   var optionObj4 = {Capped:true, Size:1024000000, Max:10000000, AutoIndexId:true, Compressed:true, CompressionType:"snappy"};
   createCappedCL( db, csName, clName+4, optionObj4);
   
   //create cappedCL CompressionType:"lzw"
   var optionObj5 = {Capped:true, Size:1024000000, Max:10000000, AutoIndexId:true, Compressed:true, CompressionType:"lzw"};
   createCappedCL( db, csName, clName+5, optionObj5);
   
   //clean cappedCS after test
   commDropCS( db, csName, true, "drop CS in the end" );
   println("---end the test---")
}

function createCappedCL( db, csName, clName, optionObj)
{
   try
   {
      db.getCS( csName ).createCL( clName, optionObj );
      if( optionObj.Compressed !== false ){
         throw "ERR_CREATE_CAPPEDCL";
      }
   }
   catch( e )
   {
      if( e !== -6 )
      {
         throw buildException( "create cappedCL AutoIndexId:true", null, "create cappedCL AutoIndexId:true", "-6",  e);
      }
   }
}