/************************************
*@Description: 修改固定集合为普通集合，修改参数为开启压缩/AutoSplit/AutoIndexId  
*@author:      liuxiaoxuan
*@createdate:  2017.9.30
*@testlinkCase:seqDB-12803/seqDB-12804
**************************************/

main();

function main()
{
	if(commIsStandalone(db))
	{
		println('skip standlone');
		return;
	}
	
   var csName = COMMCSNAME + "12803_12804";
   commDropCS( db, csName, true, "drop CS in the beginning" );
   
	//create cappedCS and cappedCL
   var csOption = {Capped:true};
   commCreateCS( db, csName, false, "", csOption );
   
   var clName = COMMCLNAME + "12803_12804";
   var clOption = {Capped:true, Size:1024, AutoIndexId:false};
   var dbcl = commCreateCLByOption( db, csName, clName, clOption, true, true );
   
   //check alter cappedCL 
   println( "---begin check alter cappedCL---" )
	var expectErrorCode32 = -32;
	var expectErrorCode245 = -245;
   var alterOption1 = {Capped:false};
   checkAlterResult( dbcl, alterOption1, expectErrorCode32 );
   var alterOption2 = {Compressed:true};
   checkAlterResult( dbcl, alterOption2, expectErrorCode32 );
   var alterOption3 = {AutoIndexId:true};
   checkAlterResult( dbcl, alterOption3, expectErrorCode32 );
	var alterOption4 = {AutoSplit:true};
   checkAlterResult( dbcl, alterOption4, expectErrorCode245);
	
	//clean environment
   commDropCS( db, csName, true, "drop CS in the end" );
	
	//create commonCS and commonCL
   commCreateCS( db, csName, false, "" );
   var dbcl = commCreateCL( db, csName, clName );
	
   //check alter commonCL 
   println( "---begin check alter CommonCL---" )
   var alterOption5 = {Capped:true};
   checkAlterResult( dbcl, alterOption5, expectErrorCode32 );
   var alterOption6 = {Size:1024};
   checkAlterResult( dbcl, alterOption6, expectErrorCode32 );
	var alterOption7 = {Max:1024};
   checkAlterResult( dbcl, alterOption7, expectErrorCode32 );
   var alterOption8 = {OverWrite:true};
   checkAlterResult( dbcl, alterOption8, expectErrorCode32 );
	
	println( "---end check---" )
	commDropCS( db, csName, true, "drop CS in the end" );
   
}

function checkAlterResult( dbcl, options, expectErrorCode )
{
   try
   {
      dbcl.alter( options );
      throw "NEED_ALTER_ERROR";
   }
   catch( e )
   {
      if(  expectErrorCode !== e  )
      {
         throw buildException("checkAlterResult()",e,"check alter result", expectErrorCode, e );
      }
   }
}
