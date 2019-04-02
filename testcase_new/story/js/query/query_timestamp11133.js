/*******************************************************************************
*@Description:   seqDB-11133:查询时间戳，闰年的2月29号
*@Author:        2019-2-25  wangkexin
********************************************************************************/

main();
function main()
{
	try
   {//TODO:1、该用例对应的问题单中，描述问题场景和创建domain没有多大关系，这里没有cs限制可以使用公共cs
      var csName = COMMCSNAME+"_11133";
      var clName = COMMCLNAME+"_11133" ;
	  var domainName = "mydomain11133";
	  var clOpt = {ReplSize:0, ShardingKey:{a:1}, ShardingType:"hash"};
	  var dataRGNames = [];
     
	  if( commIsStandalone(db) )
	  {
		  println(" Deploy mode is standalone!");
		  return;
	  }
	  //TODO:2、这里没有必要限制组个数
	  if( commGetGroupsNum(db) < 3 )
	  {
		  println("This testcase needs at least 3 groups to run.");
		  return;
	  }
	  
	  try
	  {
		  dataRGNames = getDataGroupsName();
		  db.createDomain( domainName, dataRGNames ) ;
		  println( "create domain = " + domainName ) ;
	  }
	  catch( e )
	  {
		  println( "Failed to create domain, rc = " + e ) ;
		  throw e ;
	  }
	  
	  var varCS = commCreateCS( db, csName, true, "create CS in the beginning" , { "Domain" : domainName });
	  var varCL = varCS.createCL(clName,clOpt);
      //TODO:3、下面createcl和上面的代码是重复的，之前已经提过一次！
      var collection = new Collection( csName, clName, clOpt );     
      var cl = collection.create();
	  
	  //TODO:4、建议多几条测试数据
	  println( "Begin to insert data.") ;
	  cl.insert({"a":{"$timestamp":"1984-02-29-13.14.26.124233"}});
	  var countNum = cl.count({"a":{"$timestamp":"1984-02-29-13.14.26.124233"}});
	  if(countNum != 1)
	  {
		   throw buildException("query timestamp fail", null, "count", 1, countNum);
	  }
	  
	  db.dropCS(csName);
	  println( "Success to clean cs : [" + csName + "] in the end" ) ;
	  db.dropDomain(domainName);
	  println( "Success to clean domain : [" + domainName + "] in the end" ) ;  
   }
   catch(e)
   {
   	throw e;
   }
}

function getDataGroupsName()
{  
   var tmpArray = commGetGroups( db ); 
   var groupNameArray = new Array;
   for( i = 0 ; i < tmpArray.length; i++ )
   {
      groupNameArray.push( tmpArray[i][0].GroupName );
   } 
   return groupNameArray ;
}