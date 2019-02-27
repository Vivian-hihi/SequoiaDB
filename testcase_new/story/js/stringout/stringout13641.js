/*******************************************************************************
*@Description:   seqDB-13641: 查询不使用selector，指定flags=FLG_QUERY_STRINGOUT 
*@Author:        2019-2-25  wangkexin
********************************************************************************/

main();
function main()
{  
   try
   {
      var csName = COMMCSNAME;
      var clName = COMMCLNAME+"_13641" ;
      var cl = readyCL( csName, clName );
      
      readyData( cl );
	  
	  //不使用选择符，排序字段：查询字段  排序顺序：顺序排序  指定flags=FLG_QUERY_STRINGOUT
	  var rc1 = cl.find().sort({a:1}).flags(1);
	  var expRecs1 = '[{"_id":1,"a":1,"b":"b1"},{"_id":3,"a":2,"b":"b3"},{"_id":2,"a":3,"b":"b2"},{"_id":4,"a":[10,20,30],"b":"b4"}]';
      checkCLData( rc1, expRecs1 );
	  
	  //不使用选择符，排序字段：非查询字段  排序顺序：逆序排序  指定flags=FLG_QUERY_STRINGOUT
	  var rc2 = cl.find().sort({b:-1}).flags(1);
	  var expRecs2 = '[{"_id":4,"a":[10,20,30],"b":"b4"},{"_id":3,"a":2,"b":"b3"},{"_id":2,"a":3,"b":"b2"},{"_id":1,"a":1,"b":"b1"}]';
	  checkCLData( rc2, expRecs2 );
	  
	  //不使用选择符，不使用排序 指定flags=FLG_QUERY_STRINGOUT
	  var rc3 = cl.find().flags(1);
	  var expRecs3 = '[{"_id":1,"a":1,"b":"b1"},{"_id":2,"a":3,"b":"b2"},{"_id":3,"a":2,"b":"b3"},{"_id":4,"a":[10,20,30],"b":"b4"}]';
	  checkCLData( rc3, expRecs3 );
	  
      cleanCL( csName, clName );
   }
      catch(e)
   {
   	throw e;
   }
}