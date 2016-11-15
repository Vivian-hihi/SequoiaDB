/* *****************************************************************************
@Description: subCL common functions
@modify list:
   2014-07-30 pusheng Ding  Init
   2015-12-19 Ting YU modify
***************************************************************************** */

//2015-12-19 Ting YU modify
function getSourceGroupName_alone( csName , clName )
{
   var clFullName = csName + "." + clName;
   var clInfo = db.snapshot( 8, {Name: clFullName} );
   while( clInfo.next() )
   {
      var clInfoObj = clInfo.current().toObj();
      var GroupName = clInfoObj.CataInfo[0].GroupName;
   }
   return GroupName;
}
function getOtherDataGroups( SourceGroupName )
{
	var allGroups =db.listReplicaGroups().toArray() ;
	var RoleGroupNumbers = 0 ;
	var Groups = [];
	for ( var i = 0; i<allGroups.length; i++ )
	{
		var eval_node=eval("("+allGroups[i]+")");
		if(eval_node["Role"]==0 && eval_node["GroupID"] >= 1000 )
		{
			if( eval_node["GroupName"] != SourceGroupName )
			{
				Groups.push( eval_node["GroupName"] );
			}
		}
	}
	return Groups;
}

//2015-12-19 Ting YU modify
function getPartition( csName ,clName )
{
   var clFullName = csName + "." + clName;
   var clInfo = db.snapshot( 8, {Name: clFullName} );
   while( clInfo.next() )
   {
      var clInfoObj = clInfo.current().toObj();
      var Partition = clInfoObj.Partition;
   }
	return Partition ;
}
function subCL_split_hash( subcl, SourceGroupName, OtherDataGroups, Partition)
{
	var Partition_PerGroup = Partition / ( OtherDataGroups.length + 1 ) ;
	for( var i = 0; i < OtherDataGroups.length; ++i )
	{
		var start_Partition = Math.round( Partition_PerGroup * i ) ;
		var end_Partition = Math.round( Partition_PerGroup * ( i + 1 ) ) ;
		println( start_Partition + '~~~~~~~~~~~~~~~~' + end_Partition ) ;
		try
		{
			subcl.split( SourceGroupName, OtherDataGroups[i], {Partition:start_Partition}, {Partition:end_Partition} ) ;
		}
		catch( e )
		{
			println( "can't split : " + e ) ;
			return -1
		}
	}
	return 0 ;
}
