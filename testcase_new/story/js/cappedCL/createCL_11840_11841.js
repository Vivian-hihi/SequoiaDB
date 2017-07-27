/************************************
*@Description: 在普通集合空间上创建与固定集合空间上同名的普通集合
               在固定集合空间上创建与普通集合空间上同名的固定集合
*@author:      luweikang
*@createdate:  2017.7.7
*@testlinkCase:seqDB-11840,seqDB-11841
**************************************/

main();

function main()
{
   var normal_csName = CHANGEDPREFIX + "_11840_CS";
   var capped_csName = CHANGEDPREFIX + "_11841_CS";
   
   var clName1 = CHANGEDPREFIX + "_11840_CL";
   var clName2 = CHANGEDPREFIX + "_11841_CL";
   
   //drop CS and create cappedCS
   println("---begin test---");
   commDropCS( db, normal_csName, true, "drop CS in the beginning" );
   commCreateCS( db, normal_csName, false, "beginning to create CS", null );
   initCappedCS( capped_csName );
   
   //normalCS quick cappedCS
   println("---normalCS quick cappedCS---")
   normalCStoCreateCL( normal_csName, clName1 );
   cappedCStoCreateCL( capped_csName, clName1 );
   
   //cappedCS quick normalCS
   println("---cappedCS quick normalCS---")
   cappedCStoCreateCL( capped_csName, clName2 );
   normalCStoCreateCL( normal_csName, clName2 );
   
   //clean environment after test
   println( "---end the test---" );
   commDropCS( db, normal_csName, true, "drop CS in the end" );
   commDropCS( db, capped_csName, true, "drop CS in the end" );
}

function normalCStoCreateCL( normal_csName, capped_csName )
{
   try
   {
      db.getCS( normal_csName ).createCL( capped_csName );
   }
   catch( e )
   {
      throw e;
   }
}

function cappedCStoCreateCL( capped_csName, normal_clName )
{
   var optionObj = {Capped:true, Size:1024, Max:10000000, AutoIndexId:false};
   try
   {
      db.getCS(capped_csName).createCL( normal_clName, optionObj );
   }
   catch( e )
   {
      throw e;
   }
}