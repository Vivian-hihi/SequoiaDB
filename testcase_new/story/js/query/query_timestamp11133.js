/*******************************************************************************
*@Description:   seqDB-11133:查询时间戳，闰年的2月29号
*@Author:        2019-2-25  wangkexin
********************************************************************************/

main();
function main()
{
	try
   {
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
      
      var collection = new Collection( csName, clName, clOpt );     
      var cl = collection.create();
	  
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