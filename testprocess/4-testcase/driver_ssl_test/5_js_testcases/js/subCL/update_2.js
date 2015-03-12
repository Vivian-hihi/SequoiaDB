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


function test_range_attach_hash_update_2()// NOT Error, test mainCL'ShardingType is range and subCL's ShardingType is hash , insert's result
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
	try
	{
		var cs = db.createCS( CS_Name ) ;
		var mainCL = cs.createCL( MainCL_Name, { ShardingKey:{ a:1 }, ShardingType: "range", Partition:4096, ReplSize:0, Compressed:true, IsMainCL:true } ) ;
		println( "mainCL" );
		var subCL1 = cs.createCL( subCl_Name + "1", { ShardingKey:{ a:1 }, ShardingType: "hash", ReplSize:0, Compressed:true, IsMainCL:false } ) ;
		println( "subCL1" );
		var subCL2 = cs.createCL( subCl_Name + "2", { ShardingKey:{ a:1 }, ShardingType: "hash", ReplSize:0, Compressed:true, IsMainCL:false } ) ;
		println( "subCL2" );
		mainCL.attachCL( CS_Name+".month1", { LowBound:{a:0},UpBound:{a:1} } ) ;
		println( "attach subCL1" ) ;
		mainCL.attachCL( CS_Name+".month2", { LowBound:{a:1},UpBound:{a:2} } ) ;
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
	
	try
	{
		for(var i = 0; i < 2 ; ++i )
		{
			mainCL.insert({a:i,b:{name:"YoYo",age:23,phone:[12,56,"reqnf"]},c:"jkdi"}) ;
		}
	}
	catch( e )
	{
		println( "i = " + i + ", err is :" + e ) ;
	}
	
	try
	{
	   mainCL.update( {$unset:{c:"jkdi"}}) ;
	}
	catch ( e )
	{
	   println( "failed to update record, rc= " + e ) ;
	   throw e ;
	}
	
	var rc ;
	try
	{
	   rc = mainCL.find( {c:"jkdi"} ) ;
	}
	catch ( e )
	{
	   println( "failed to read record, rc= " + e ) ;
	   throw e ;
	}
	
	if ( 0 != mainCL.find( {c:"jkdi"} ).count() )
	{
	   println( " get more than zero record" ) ;
	   throw -1 ;
	}
	
	try
	{
	   mainCL.update( {$inc:{salary:100}}) ;
	}
	catch ( e )
	{
	   println( "failed to update record, rc1= " + e ) ;
	   throw e ;
	}
	var rc1 ;
	try
	{
	 rc1 = mainCL.find({salary:100});
	}
	catch ( e )
	{
	   println( "failed to read record, rc1= " + e ) ;
	   throw e ;
	}
	
	if ( 2 != mainCL.find( {salary:100} ).count() )
	{
	   println( " get more than one record, rc1" ) ;
	   throw -1 ;
	}
	
	try
	{
	   mainCL.update( {$push:{"b.phone":3}}) ;
	}
	catch ( e )
	{
	   println( "failed to update record, rc2= " + e ) ;
	   throw e ;
	}
	var rc2 ;
	try
	{
	 rc2 = mainCL.find({"b.phone.3":3});
	}
	catch ( e )
	{
	   println( "failed to read record, rc2= " + e ) ;
	   throw e ;
	}
	if ( 2 != mainCL.find( {"b.phone.3":3} ).count() )
	{
	   println( " get more than one record ,rc2" ) ;
	   throw -1 ;
	}
	
	
	try
	{
	   mainCL.update( {$pull:{"b.phone":3}}) ;
	}
	catch ( e )
	{
	   println( "failed to update record, rc3= " + e ) ;
	   throw e ;
	}
	var rc3 ;
	try
	{
	 rc3 = mainCL.find({"b.phone.3":3});
	}
	catch ( e )
	{
	   println( "failed to read record, rc3= " + e ) ;
	   throw e ;
	}
	if ( 0 != mainCL.find({"b.phone.3":3}).count() )
	{
	   println( " get more than one record ,rc3" ) ;
	   throw -1 ;
	}
	
	try
	{
	   mainCL.update( {$push_all:{array:[3,4]}}) ;
	}
	catch ( e )
	{
	   println( "failed to update record, rc4= " + e ) ;
	   throw e ;
	}
	var rc4 ;
	try
	{
	 rc4 = mainCL.find({array:[3,4]});
	}
	catch ( e )
	{
	   println( "failed to read record, rc4= " + e ) ;
	   throw e ;
	}
	if ( 2 != mainCL.find({array:[3,4]}).count() )
	{
	   println( " get more than one record ,rc4" ) ;
	   throw -1 ;
	}
	
	try
	{
	   mainCL.update( {$pull_all:{array:[3,4]}}) ;
	}
	catch ( e )
	{
	   println( "failed to update record, rc5= " + e ) ;
	   throw e ;
	}
	var rc5 ;
	try
	{
	 rc5 = mainCL.find({array:[]});
	}
	catch ( e )
	{
	   println( "failed to read record, rc5= " + e ) ;
	   throw e ;
	}
	if ( 2 != mainCL.find({array:[]}).count() )
	{
	   println( " get more than one record ,rc5" ) ;
	   throw -1 ;
	}
	
	try
	{
	   mainCL.update( {$pop:{"b.phone":2}}) ;
	}
	catch ( e )
	{
	   println( "failed to update record, rc6= " + e ) ;
	   throw e ;
	}
	var rc6 ;
	try
	{
	 rc6 = mainCL.find({"b.phone":[12]});
	}
	catch ( e )
	{
	   println( "failed to read record, rc6 = " + e ) ;
	   throw e ;
	}
	if ( 2 != mainCL.find({"b.phone":[12]}).count() )
	{
	   println( " get more than one record ,rc6" ) ;
	   throw -1 ;
	}
	
	try
	{
	   mainCL.update( {$addtoset:{"b.phone":[12]}}) ;
	}
	catch ( e )
	{
	   println( "failed to update record, rc7= " + e ) ;
	   throw e ;
	}
	var rc7 ;
	try
	{
	 rc7 = mainCL.find({"b.phone":[12]});
	}
	catch ( e )
	{
	   println( "failed to read record, rc7 = " + e ) ;
	   throw e ;
	}
	if ( 2 != mainCL.find({"b.phone":[12]}).count() )
	{
	   println( " get more than one record ,rc7" ) ;
	   throw -1 ;
	}
//	println( "mainCL.find({a:0})" ) ;
//	println( mainCL.find({a:0}) ) ;
//	println( "mainCL.find({a:1})" ) ;
//	println( mainCL.find({a:1}) ) ;
//	
//	println( "subCL1.find({a:0})" ) ;
//	println( subCL1.find({a:0}) ) ;
//	println( "subCL1.find({a:1})" ) ;
//	println( subCL1.find({a:1}) ) ;
//	
//	println( "subCL2.find({a:0})" ) ;
//	println( subCL2.find({a:0}) ) ;
//	println( "subCL2.find({a:1})" ) ;
//	println( subCL2.find({a:1}) ) ;
}

function main()
{
	try
	{
		var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
		db.listReplicaGroups();
	}
	catch( e )
	{
		if( e == -159 )
		{
			println( "can't run in standalone" ) ;
			return ;
		}
		println( "fail to check standalone" ) ;
		throw e ;
	}
	println( "test_range_attach_hash_update_2 is start" ) ;
	test_range_attach_hash_update_2();
	println( "test_range_attach_hash_update_2 is end" ) ;
	println() ;
	
}

main() ;







