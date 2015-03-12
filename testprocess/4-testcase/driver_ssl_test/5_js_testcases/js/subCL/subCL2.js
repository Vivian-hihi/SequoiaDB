/* *****************************************************************************
@discretion: attach rangCL
@modify list:
   2014-07-30 pusheng Ding  Init
***************************************************************************** */

function test_range_attach_range() // NOT Error, test mainCL'ShardingType is range and subCL's ShardingType is range , attach's result
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
		println( "can't new SecureSdb or dropCS : " + e );
		if( e != -34 )
		{
			println( "throw without dropCS" ) ;
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
		var mainCL = cs.createCL( MainCL_Name, { ShardingKey:{ a:1 }, ShardingType: "range", ReplSize:0, Compressed:true, IsMainCL:true } ) ;
		println( "mainCL" );
		var subCL1 = cs.createCL( subCl_Name + "1", { ShardingKey:{ a:1 }, ShardingType: "range", ReplSize:0, Compressed:true, IsMainCL:false } ) ;
		println( "subCL1" );
		var subCL2 = cs.createCL( subCl_Name + "2", { ShardingKey:{ a:1 }, ShardingType: "range", ReplSize:0, Compressed:true, IsMainCL:false } ) ;
		println( "subCL2" );
		mainCL.attachCL( CS_Name+".month1", { LowBound:{a:0},UpBound:{a:10} } );
		println( "attach subCL1" );
		mainCL.attachCL( CS_Name+".month2", { LowBound:{a:10},UpBound:{a:20} } );
		println( "attach subCL2" );
	}
	catch( e )
	{
		println( " Error: " + e );
		throw e ;
	}
	
}

// Add inspect standalone run mode
try
{
   var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
   // Inspect the run mode is standalone or not
   if( true == commIsStandalone( db ) )
      throw "ModeStandAlone" ;
   test_range_attach_range();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}
