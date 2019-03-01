/*******************************************************************************
*@Description:   seqDB-13643:查询不使用排序，指定selector为$include和flags=FLG_QUERY_STRINGOUT
*@Author:        2019-2-25  wangkexin
********************************************************************************/

main();
function main()
{  
   try
   {
      var csName = COMMCSNAME;
      var clName = COMMCLNAME+"_13643" ;
      var cl = commCreateCL(db, csName, clName );
      
      readyData( cl );
	  
	  //使用$include选择符，不使用排序 指定flags=FLG_QUERY_STRINGOUT
	  var rc = cl.find({},{Key:{$include:1}}).flags(1);
	  var expRecs = '[{"":"123"},{"":"3000000000"},{"":"123.456"},{"":"{ \\\"$decimal\\\": \\\"123.456\\\" }"},{"":"value"},{"":"123abcd00ef12358902300ef"},{"":"true"},{"":"2012-01-01"},{"":"2012-01-01-13.14.26.124233"},{"":""},{"":""},{"":""},{"":""},{"":""},{"":""},{"":""}]';
      checkCLData( rc, expRecs );
	  commDropCL( db, csName, clName );
   }
      catch(e)
   {
   	throw e;
   }
}