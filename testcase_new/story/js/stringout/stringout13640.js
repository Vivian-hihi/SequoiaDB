/*******************************************************************************
*@Description:   seqDB-13640:查询使用排序，指定selector为$include和flags=FLG_QUERY_STRINGOUT
*@Author:        2019-2-25  wangkexin
********************************************************************************/

main();
function main()
{  
   try
   {
      var csName = COMMCSNAME;
      var clName = COMMCLNAME+"_13640" ;
      var cl = readyCL( csName, clName );
      
      readyData( cl );
	  
	  //使用$include选择符，排序字段：查询字段  排序顺序：顺序排序  指定flags=FLG_QUERY_STRINGOUT
	  var rc1 = cl.find({},{a:{$include:1}}).sort({a:1}).flags(1);
	  var expRecs1 = '[{"":"1"},{"":"2"},{"":"3"},{"":""}]';
      checkCLData( rc1, expRecs1 );
	  
	  //使用$include选择符，排序字段：非查询字段  排序顺序：逆序排序  指定flags=FLG_QUERY_STRINGOUT
	  var rc2 = cl.find({},{a:{$include:1}}).sort({b:-1}).flags(1);
	  var expRecs2 = '[{"":""},{"":"2"},{"":"3"},{"":"1"}]';
	  checkCLData( rc2, expRecs2 );
      cleanCL( csName, clName );
   }
      catch(e)
   {
   	throw e;
   }
}