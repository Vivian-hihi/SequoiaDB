/* *****************************************************************************
@discretion: subCL common functions
@modify list:
   2014-07-30 pusheng Ding  Init
***************************************************************************** */

function getSourceGroupName_alone(CS_Name,CL_Name)
{
	var cata=new SecureSdb(COORDHOSTNAME,CATASVCNAME);
	var allCollections=cata.SYSCAT.SYSCOLLECTIONS.find().toArray() ;
	var CS_CL=CS_Name+"."+CL_Name;
	var GroupName = "";
	for( var i = 0 ; i<allCollections.length ; i++ )
	{
		var eval_CL=eval("("+allCollections[i]+")");
		if(eval_CL["Name"]==CS_CL)
		{
			println( eval_CL["Name"] ) ;
			/*for(var j=0;j<eval_CL["CataInfo"].length;j++)
			{
				GroupName = eval_CL["CataInfo"][j]["GroupName"] ;
			}*/
			GroupName = eval_CL["CataInfo"][0]["GroupName"] ;
			break ;
		}
	}
	return GroupName;
}
function getOtherDataGroups( SourceGroupName )
{
	var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME) ;
	var allGroups =db.listReplicaGroups().toArray() ;
	var RoleGroupNumbers = 0 ;
	var Groups = [];
	for ( var i = 0; i<allGroups.length; i++ )
	{
		var eval_node=eval("("+allGroups[i]+")");
		if(eval_node["Role"]==0)
		{
			if( eval_node["GroupName"] != SourceGroupName )
			{
				Groups.push( eval_node["GroupName"] );
			}
		}
	}
	return Groups;
}

function getPartition(CS_Name,CL_Name)
{
	var cata=new SecureSdb(COORDHOSTNAME,CATASVCNAME);
	var allCollections=cata.SYSCAT.SYSCOLLECTIONS.find().toArray() ;
	var CS_CL=CS_Name+"."+CL_Name;
	var Partition = "";
	for( var i = 0 ; i<allCollections.length ; i++ )
	{
		var eval_CL = eval( "(" + allCollections[i] + ")" );
		if(eval_CL["Name"]==CS_CL)
		{
			Partition = eval_CL[ "Partition" ] ;
			break ;
		}
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