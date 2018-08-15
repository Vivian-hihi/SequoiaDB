/******************************************************************************
@Description :  seqDB-15546:指定AutoIndexId:false创建hash分区表，alter为自动切分
@Modify list :  2018-8-9  xiaoni Zhao  Init
******************************************************************************/
function main()
{
	if( true === commIsStandalone( db ) )
	{
	   println( "Standalone environment!" );
      return;	
	}
	
	//get groups from sdb
   var groupNames = getGroupNames();
   if( ( 2 > groupNames.length ) )
   {
      println( "Only one group or standalone environment!" );
      return;
   }
   
   //define the name of CL and domain
   var csName = COMMCSNAME + "_15546";
	var clName = COMMCLNAME + "_15546";	
	var domName = "mydomain" + "_15546"
	var srcGroup = groupNames[0];
   var desGroup = groupNames[1];
	
	//drop domain before
	dropDomain( domName, csName );
	
	//create domain
	db.createDomain( domName, [srcGroup, desGroup] );
	
	//create CL
	var varCL = db.createCS( csName,{Domain:domName} ).createCL( clName,{ShardingKey:{a:1}, ShardingType:"hash",AutoIndexId:false} )
	
	//check id index not existed;
   checkIdIndex( clName, "NoIDIndex", csName ); 
   
   //set CL attributes
   try
   {
      varCL.setAttributes( {AutoSplit:true} );
      throw "NEED_ERROR";	
   }catch(e)
   {
   	if( e!=-279 )
   	{
   	   throw e;	
   	}
   }
   
}

main();