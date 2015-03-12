/* *****************************************************************************
@discretion: attach hashCL and insert and remove
@modify list:
   2014-07-30 pusheng Ding  Init
***************************************************************************** */

function test_range_attach_hash_insert_remove_args() // Not Error, test mainCL'ShardingType is range and subCL's ShardingType is hash ,insert large number's result
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
		mainCL.attachCL( CS_Name+".month1", { LowBound:{a:0,b:1000},UpBound:{a:1000,b:0} } ) ;
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
	
	println( " no data start " ) ;
	println( " mainCL.remove({a:1})" ) ;
	try 
	{
		mainCL.remove({a:1}) ;
		
	}
	catch( e )
	{
		println( "i = " + i + ", err is :" + e ) ;
		throw e ;
	}
	
	println( " subCL1.remove({b:1}) " ) ;
	try 
	{
		subCL1.remove({b:1}) ;
		
	}
	catch( e )
	{
		println( "i = " + i + ", err is :" + e ) ;
		throw e ;
	}
	
	println( " subCL2.remove({c:1}) " ) ;
	try 
	{
		subCL2.remove({c:1}) ;
		
	}
	catch( e )
	{
		println( "i = " + i + ", err is :" + e ) ;
		throw e ;
	}
	println( " no data end" ) ;
	
	try 
	{
		println( "first remove test" ) ;
		mainCL.remove() ;
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.insert( {a:i} ) ;
		}
		if( mainCL.find().count() != 2000 )
		{
			println( " number is errno " );
		}
		
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.remove( {a:i} ) ;
		}
		if( ( mainCL.find().count() != 0 ) || ( subCL1.find().count() != 0 ) || ( subCL2.find().count() != 0 ) )
		{
			println( " number is errno " );
		}
	}
	catch( e )
	{
		println( "i = " + i + ", err is :" + e ) ;
		throw e ;
	}
	
	try 
	{
		println( "2st remove test" ) ;
		mainCL.remove() ;
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.insert( {a:i} ) ;
		}
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.insert( {a:i} ) ;
		}
		if( mainCL.find().count() != 4000 )
		{
			println( " number is errno " );
		}
		
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.remove( {a:i} ) ;
		}
		if( ( mainCL.find().count() != 0 ) || ( subCL1.find().count() != 0 ) || ( subCL2.find().count() != 0 ) )
		{
			println( " number is errno " );
		}
	}
	catch( e )
	{
		println( "i = " + i + ", err is :" + e ) ;
		throw e ;
	}
	
	try 
	{
		println( "3st remove test" ) ;
		mainCL.remove() ;
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.insert( {a:i} ) ;
		}
		for(var i = 0; i < 2000 ; ++i )
		{
			mainCL.insert( {a:i} ) ;
		}
		if( mainCL.find().count() != 4000 )
		{
			println( " number is errno " );
		}
		
		for(var i = 0; i < 1000 ; ++i )
		{
			subCL1.remove( {a:i} ) ;
		}
		println( "mainCL.find().count()=" + mainCL.find().count() );
		println( "subCL1.find().count()=" + subCL1.find().count() );
		println( "subCL2.find().count()=" + subCL2.find().count() );
		if( ( mainCL.find().count() != 2000 ) || ( subCL1.find().count() != 0 ) || ( subCL2.find().count() != 2000 ) )
		{
			println( " number is errno " );
		}
		println( "mainCL.find().count()=" + mainCL.find().count() );
		println( "subCL1.find().count()=" + subCL1.find().count() );
		println( "subCL2.find().count()=" + subCL2.find().count() );		
		for(var i = 1000; i < 2000 ; ++i )
		{
			subCL2.remove( {a:i} ) ;
		}
		if( ( mainCL.find().count() != 0 ) || ( subCL1.find().count() != 0 ) || ( subCL2.find().count() != 0 ) )
		{
			println( " number is errno " );
		}
	}
	catch( e )
	{
		println( "i = " + i + ", err is :" + e ) ;
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
   test_range_attach_hash_insert_remove_args();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}
