var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME) ; 
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ; 

function inspecRG()
{
	try
	{
		db.listReplicaGroups() ;	
		return true ; 
	}
	catch ( e )
	{
		if( -159==e )
		{
			println("No groups ") ;
			return false ; 
		}
		else
		{
			println("excute listReplicaGroups failed"+e) ;
			throw e ; 
		}
	} 
}

if( inspecRG()==true )
{
	try
	{
		db.dropCS(CSPREFIX_CS) ;
	}
	catch ( e )
	{
		if( -34!=e )
		{
			println("Failed to drop CS "+e) ;
			throw e ;
		}
	}

	var cl ;
	try
	{
		var claSize = new RSize(CSPREFIX_CS) ; 
		var cs = db.createCS(CSPREFIX_CS) ;
		cl = cs.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()}) ; 	
	}
	catch ( e )
	{
		println("Failed to create CS and CL "+e) ;
		throw e ;
	}

	try
	{
		cl.insert({no:1002,score:85,interest:["movie","photo"],major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:">女"}});
		cl.insert({no:1005,score:70,major:"计算机工程",dep:"计算机学院",info:{name:"Jim",age:24,sex:"女"}});
		cl.insert({no:1000,score:80,interest:["basketball","football"],major:"计算机科学与技术",dep:"计算机学院",info:{name:"Tom",age:25,sex:"男"}});
		cl.insert({no:1001,score:82,major:"计算机科学与技术",dep:"计算机学院",info:{name:"Json",age:20,sex:"男"}});
		cl.insert({no:1003,score:90,major:"计算机软件与理论",dep:"计算机学院",info:{name:"Sam",age:30,sex:"男"}});
		cl.insert({no:1004,score:69,interest:["basketball","football","movie"],major:"计算机工程",dep:"计算机学院",info:{name:"Coll",age:26,sex:"男"}});
		cl.insert({no:1008,score:72,interest:["basketball","football","movie"],major:"物理学",dep:"物电学院",info:{name:"Appie",age:20,sex:"女"}});
		cl.insert({no:1006,score:84,interest:["basketball","football","movie","photo"],major:"物理学",dep:"物电学院",info:{name:"Lily",age:28,sex:"女"}});
		cl.insert({no:1007,score:73,interest:["basketball","football","photo"],major:"物理学",dep:"物电学院",info:{name:"Kiki",age:18,sex:"女"}});
		cl.insert({no:1009,score:80,major:"物理学",dep:"物电学院",info:{name:"Lucy",age:36,sex:"女"}});
		cl.insert({no:1011,score:75,major:"光学",dep:"物电学院",info:{name:"Jack",age:30,sex:"男"}});
		cl.insert({no:1010,score:93,major:"光学",dep:"物电学院",info:{name:"Coco",age:27,sex:"女"}});
		cl.insert({no:1012,score:78,interest:["basketball","movie"],major:"光学",dep:"物电学院",info:{name:"Mike",age:28,sex:"男"}});
		cl.insert({no:1014,score:74,interest:["football","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Iccra",age:19,sex:"男"}});
		cl.insert({no:1013,score:86,interest:["basketball","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Jaden",age:28,sex:"男"}});
		cl.insert({no:1016,score:99,major:"电学",dep:"物电学院",info:{name:"Kate",age:20,sex:"男"}});
		cl.insert({no:1015,score:81,major:"电学",dep:"物电学院",info:{name:"Jay",age:15,sex:"男"}});
	}
	catch ( e )
	{
		println("Failed to insert record to collection "+e) ;
		throw e ;
	}
	println("Success to insert records ") ;
	try
	{
		var count = cl.count() ;
		if( count!=17 )
		{
			println("Error,insert wrong number of records "+count) ;
			throw "Error_Insert" ;
		}
	}
	catch ( e )
	{
		println("Failed inspect the records "+e )
		throw e ;
	}

	try
	{
		cl.remove({$or:[{score:{$gte:85,$lte:96,$ne:90}},{"info.age":{$gt:25,$lt:30}},{"info.name":{$nin:["Kate","Tom"]}},{"info.sex":{$in:["女"]}}]}) ;
		var count = cl.count() ;
	}
	catch ( e )
	{
		println("Failed to remove the records"+e) ;
		throw e ;
	}


	try
	{
		var count = cl.count() ;
		if( count!=2 )
		{
			println("Wrong to remove the records "+count) ;
			throw "Error_Remove" ;
		}	
		println("Success to remove records ($or)") ;
	}
	catch ( e )
	{
		println("Failed to remove the records "+e ) ;
		throw e ;
	}

	try
	{
		db.dropCS(CSPREFIX_CS) ;
	}
	catch ( e )
	{
		println("Failed to drop CS ,end "+e) ;
		throw e ;
	}
}
else
{
	println("Standalone,no Groups") ; 
}
