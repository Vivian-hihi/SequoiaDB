/******************************************************************************
@Description :  seqDB-15545:指定AutoIndexId:false，加入域并使用自动切分
@Modify list :  2018-8-8  xiaoni Zhao  Init
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
   var csName = COMMCSNAME + "_15545";
	var clName = COMMCLNAME + "_15545";	
	var domName = "mydomain" + "_15545"
	var srcGroup = groupNames[0];
   var desGroup = groupNames[1];
	
	//drop domain before
	dropDomain( domName, csName );
	
	//create domain
	db.createDomain( domName, [srcGroup, desGroup] );
	
	//create CL
	var varCL = db.createCS( csName,{Domain:domName} ).createCL( clName,{ShardingKey:{a:1}, ShardingType:"hash",AutoIndexId:false,AutoSplit:true} )

   //check id index not existed;
   checkIdIndex( clName, "NoIDIndex", csName);
   
   //check catalog information
   checkCataInfo( clName, srcGroup, 2, csName );
   
   //insert data
   for( var i=1; i<=50; i++ )
   {
      varCL.insert( {a:i} );
   }
   
   //check data
   checkSplitResult( srcGroup, desGroup, clName, csName )
}

main()