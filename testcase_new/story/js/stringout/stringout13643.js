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
      var cl = readyCL( csName, clName );
      
      readyData( cl );
	  
	  //使用$include选择符，不使用排序 指定flags=FLG_QUERY_STRINGOUT
	  var rc = cl.find({},{a:{$include:1}}).flags(1);
	  var expRecs = '[{"":"1"},{"":"3"},{"":"2"},{"":""}]';
      checkCLData( rc, expRecs);
	  cleanCL( csName, clName );
   }
      catch(e)
   {
   	throw e;
   }
}