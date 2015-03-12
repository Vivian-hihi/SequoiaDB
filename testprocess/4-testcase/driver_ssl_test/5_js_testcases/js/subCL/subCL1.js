/* *****************************************************************************
@discretion: subCL hash-split
@modify list:
   2014-07-30 pusheng Ding  Init
***************************************************************************** */

function test1() // NOT Error, check main CL and sub CL wheter have SourceGroupName , is the base of function split 
{
	CS_Name = "cs_subCL_test" + CSPREFIX ;
	MainCL_Name = "year" ;
	subCl_Name = "month" ;
	try
	{
		var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
		db.dropCS( CS_Name ) ;
	}
	catch( e )
	{
		println( "can't new SecureSdb or dropCS : " );
		if( e != -34 )
		{
			throw e ;
		}
	}
	try{
		var cs = db.createCS( CS_Name ) ;
	}catch(e){
		//getErr(-33);
		//collection space already exist
		if(e != -33)
		{
			throw e ;
		}
		else
		{
			var cs = db.getCS(CS_Name);
		}
	}
	println( CS_Name ) ;
	
	try
	{
		var mainCL = cs.createCL( MainCL_Name, { ShardingKey:{ a:1 },  Partition:4096, ReplSize:0, Compressed:true, IsMainCL:true } ) ;
	}
	catch( e )
	{
		println( "Can't create mainCL: " + e ) ;
		db.dropCS(CS_Name) ;
		throw e ;
	}
	try
	{
		var subCL = cs.createCL( subCl_Name + "1", { ShardingKey:{ a:1 }, ShardingType: "hash", ReplSize:0, Compressed:true, IsMainCL:false } ) ;
	}
	catch( e )
	{
		println( "Can't create subCL: " + e );
		db.dropCS(CS_Name) ;
	}
	
	var sourceDataGroupName = getSourceGroupName_alone( CS_Name, subCl_Name + "1" );
	println( "sourceDataGroupName is : " + sourceDataGroupName ) ;
	var desDataGroupName = getOtherDataGroups( sourceDataGroupName ) ;
	println("desDataGroupName is "+desDataGroupName);
	var Partition = getPartition( CS_Name, subCl_Name + "1" );
	println( "Partition is : " + Partition ) ;
	if( !subCL_split_hash( subCL, sourceDataGroupName, desDataGroupName, Partition) )
	{
		println( "************SPLIT SUCCED***************" ) ;
	}
	else
	{
		throw -1 ;
	}
	return 0 ;
}

// Add inspect standalone run mode
try
{
   var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
   // Inspect the run mode is standalone or not
   if( true == commIsStandalone( db ) )
      throw "ModeStandAlone" ;
   test1();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}
