/* *****************************************************************************
@discretion: attach hashCL and insert-data before split
@modify list:
   2014-07-30 pusheng Ding  Init
***************************************************************************** */

function test_range_attach_hash_insert_large_same_beforesplit() // 
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
		var mainCL = cs.createCL( MainCL_Name, { ShardingKey:{ a:1,b:-1 }, ShardingType: "range", ReplSize:0, Compressed:true, IsMainCL:true } ) ;
		println( "mainCL" );
		var subCL1 = cs.createCL( subCl_Name + "1", { ShardingKey:{ a:1 }, ShardingType: "hash", ReplSize:0, Compressed:true, IsMainCL:false } ) ;
		println( "subCL1" );
		var subCL2 = cs.createCL( subCl_Name + "2", { ShardingKey:{ a:1 }, ShardingType: "hash", ReplSize:0, Compressed:true, IsMainCL:false } ) ;
		println( "subCL2" );
		mainCL.attachCL( CS_Name+".month1", { LowBound:{a:0},UpBound:{a:1000} } ) ;
		println( "attach subCL1" ) ;
		mainCL.attachCL( CS_Name+".month2", { LowBound:{a:1000},UpBound:{a:2000} } ) ;
		println( "attach subCL2" ) ;
	}
	catch( e )
	{
		throw e ;
	}
	
	try 
	{
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.insert( {a:i} ) ;
			mainCL.insert( {a:i+1,b:i+1} ) ;
			//mainCL.insert( {a:0,b:999} ) ;
		}
		println( " The first same data " ) ;
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.insert( {a:i} ) ;
			mainCL.insert( {a:i+1,b:i+1} ) ;
			//mainCL.insert( {a:0,b:999} ) ;
		}
		println( " The second same data " ) ;
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.insert( {a:i} ) ;
			mainCL.insert( {a:i+1,b:i+1} ) ;
			//mainCL.insert( {a:0,b:999} ) ;
		}
	}
	catch( e )
	{
		println( "i = " + i + ", err is :" + e ) ;
		throw e ;
	}
	
	try
	{
		var subCL = [] ;
		subCL.push( subCL1 ) ;
		subCL.push( subCL2 ) ;
		var numberOfsubCl = 2 ;
		for( var i = 0; i < numberOfsubCl; ++i )
		{
			var sourceDataGroupName = getSourceGroupName_alone( CS_Name, subCl_Name + ( i + 1 ) );
			println( "sourceDataGroupName is : " + sourceDataGroupName ) ;
			
			var desDataGroupName = getOtherDataGroups( sourceDataGroupName ) ;
			println("desDataGroupName is "+desDataGroupName);
			
			var Partition = getPartition( CS_Name, subCl_Name + ( i + 1 ) );
			println( "Partition is : " + Partition ) ;
			
			if( !subCL_split_hash( subCL[i], sourceDataGroupName, desDataGroupName, Partition) )
			{
				println( "************SPLIT SUCCED***************" ) ;
			}
		}
	}
	catch( e )
	{
		println( " Error: " + e );
		throw e ;
	}
	
	println( "mainCL max min value:" ) ;
	println( mainCL.find().sort( {a:1} ).limit( 1 ) ) ;
	println( mainCL.find().sort( {a:-1} ).limit( 1 ) ) ;
	
	println( "subCL1 max min value:" ) ;
	println( subCL1.find().sort( {a:1} ).limit( 1 ) ) ;
	println( subCL1.find().sort( {a:-1} ).limit( 1 ) ) ;
	
	println( "subCL2 max min value:" ) ;
	println( subCL2.find().sort( {a:1} ).limit( 1 ) ) ;
	println( subCL2.find().sort( {a:-1} ).limit( 1 ) ) ;
	
}

// Add inspect standalone run mode
try
{
   var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
   // Inspect the run mode is standalone or not
   if( true == commIsStandalone( db ) )
      throw "ModeStandAlone" ;
   test_range_attach_hash_insert_large_same_beforesplit();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}
