/******************************************************************************
@Description : 1. range collection's query explain
@Modify list :
               2014-07-15 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
CSPREFIX_IDX = CSPREFIX+"idx" ;

if( false == commIsStandalone( db ) )
{
   // set session get data from master
   db.setSessionAttr( {"PreferedInstance":"M"} ) ;
}

function main()
{
	try{
		var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
	}catch(e)
	{
		println("can't connect to db");
		throw e;
	}
	
	//get ReplicaGroups
	try{
		var grouplist = Array();
		var cur = db.listReplicaGroups();
		while(cur.next()){
			if(cur.current().toObj()['GroupID'] >= DATA_GROUP_ID_BEGIN ){
				grouplist.push(cur.current().toObj()['GroupName']);
			}
		}
		var groups_num = grouplist.length;
	}catch(e)
	{
		println("get ReplicaGroups info fail! rc="+e);
		throw e;
	}
	println("ReplicaGroups: " + grouplist);
	
	try{
		db.dropCS( CSPREFIX_CS );
	}catch( e ){}
	
	//create CS
	try{
		var varCS = db.createCS(CSPREFIX_CS);
	}catch(e)
	{
		println("can't create CS:" + CSPREFIX_CS + " rc="+e);
		throw e;
	}
	println("createCS " + CSPREFIX_CS + " finished");
	
	//create rangeCL
	try{
		var varCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{b:1},ShardingType:'range',ReplSize:0});
		var sn1 = db.snapshot(8,{Name:CSPREFIX_CS+"."+CSPREFIX_CL});
		var sourceGroup = sn1.current().toObj()['CataInfo'][0]['GroupName'];
	}catch(e)
	{
		println("can't create range-CL:" + CSPREFIX_CL + " rc="+e);
		throw e;
	}
	println("createCL " + CSPREFIX_CL + " at ReplicaGroup:" + sourceGroup + " finished!");

	//get explain for collection without split
	try{
		varCL.find().explain();
		varCL.find({b:{$gt:3000}}).sort({a:-1}).explain();
		varCL.find({a:{$lt:-1000}}).sort({b:1}).limit(1000).explain();
	}catch(e)
	{
		println("get explain for collection without split fail! rc=" + e);
		throw e;
	}
	println("get explain for collection without split finish");
	
	//split
	try{
		if(groups_num>1){
			var tarGroupIndex=-1;
			var stepId = 3000;
			var partId = 3;
			var lowId = 0;
			var highId = 0;
			for(var i=0;i<partId;i++){
				tarGroupIndex++;
				if(tarGroupIndex == groups_num)
					tarGroupIndex=0;
				if(grouplist[tarGroupIndex]==sourceGroup)
				{
					i--;
					continue;
				}
				lowId = i*stepId;
				highId = (i+1)*stepId;
				varCL.split(sourceGroup, grouplist[tarGroupIndex],{b:lowId},{b:highId});
				println(CSPREFIX_CL+" split from "+sourceGroup+" to "+ grouplist[tarGroupIndex]+" {b:"+lowId+"} {b:"+highId+"}");
			}
			println("split succ!");
		}
		else{
			println("can't split to groups!groupsNum is "+groups_num);
		}
	}catch(e)
	{
		println("split fail! rc="+e);
		throw e;
	}
	
	//find().explain()
	try{
		varCL.find().explain();
	}catch(e)
	{
		println(CSPREFIX_CL + ".find().explain() fail! rc=" + e);
		throw e;
	}
	println(CSPREFIX_CL + ".find().explain() finished");
	
	//find(cond).explain()
	try{
		varCL.find({a:{$gt:-1000}}).explain();
		varCL.find({d:{$gt:-1000}}).explain();
		varCL.find({b:{$et:1000},a:{$in:[-9000,-9001,-9002]}}).explain();
	}catch(e)
	{
		println(CSPREFIX_CL + ".find(cond).explain() fail! rc="+e);
		throw e;
	}
	println(CSPREFIX_CL + ".find(cond).explain() finished");
	
	//find(null,sel).explain()
	try{
		varCL.find(null,{a:1,b:1}).explain();
		varCL.find(null,{c:'abcd'}).explain();
		varCL.find(null,{d:'undefined',a:'N/A'}).explain();
	}catch(e)
	{
		println(CSPREFIX_CL + ".find(null,sel).explain() fail! rc=" + e);
		throw e;
	}
	println(CSPREFIX_CL + ".find(null,sel).explain() finished");
	
	//find(cond,sel).explain()
	try{
		varCL.find({a:{$lt:-1000}},{b:0}).explain();
		varCL.find({$and:[{b:{$lte:1000},a:{$gte:-100000}}]},{b:0,c:'c',a:-1}).explain();
		varCL.find({$not:[{c:{$ne:'abcdefghijkl2001'}}]},{c:'abcd',id:'undefined'}).explain();
	}catch(e)
	{
		println(CSPREFIX_CL + ".find(cond,sel).explain() fail! rc=" + e);
		throw e;
	}
	println(CSPREFIX_CL + ".find(cond,sel).explain() finished");
	
	//find.limit.explain()
	try{
		varCL.find({a:{$mod:[1000,99]}},{a:1,b:1}).limit(10).explain();
		varCL.find().limit(-10).explain();
		varCL.find({d:{$exists:1}}).limit(1).explain();
	}catch(e)
	{
		println(CSPREFIX_CL + ".find.limit.explain fail! rc="+e);
		throw e;
	}
	println(CSPREFIX_CL + ".find.limit.explain finished");
	
	//find.sort.explain()
	try{
		varCL.find().sort({b:-1}).explain();
		varCL.find({a:{$nin:[1,100,1000,10000,100000]}},{a:0,b:0,c:0,d:0}).sort({a:1}).explain();
		varCL.find({b:{$lte:10000}}).sort({c:1,b:-1}).limit(100).explain();
	}catch(e)
	{
		println(CSPREFIX_CL + ".find.sort.explain fail! rc="+e);
		throw e;
	}
	println(CSPREFIX_CL + ".find.sort.explain finished");
	
	//find.skip.explain()
	try{
		varCL.find().skip(9990).explain();
		varCL.find({$or:[{a:{$lte:-9000}},{a:{$gte:-1000}}, {b:2000}]},{c:'a',a:0}).sort({a:1}).skip(1000).explain();
		varCL.find({c:{$elemMatch:{c:'abcdefghijkl2000'}}}).limit(1).skip(0).explain();
	}catch(e)
	{
		println(CSPREFIX_CL + ".find.skip.explain fail! rc=" + e);
	}
	println(CSPREFIX_CL + ".find.skip.explain finished");
	
	//create index
	try{
		varCL.createIndex(CSPREFIX_IDX,{a:-1},false,false);
	}catch(e)
	{
		println("cat't create index " + CSPREFIX_IDX + " rc=" + e);
		throw e;
	}
	println("create index finished!");
	
	//insert data
	try{
		for(var i=0;i<10000;i++){varCL.insert({a:i-10000,b:i,c:"abcdefghijkl"+i});}
	}catch(e)
	{
		println("insert-data fail! rc="+e);
	}
	println("insert-data succ!");
	
	//find.hint
	try{
		varCL.find().hint({"":CSPREFIX_IDX}).explain();
		varCL.find().hint({"":CSPREFIX_IDX+"123"}).explain();
		
	}catch(e)
	{
		println(CSPREFIX_CL + ".find.hint fail! rc=" + e);
		throw e;
	}
	println(CSPREFIX_CL + ".find.hint finished");
	
	//get explain for collection with index
	try{
		varCL.find({a:{$gt:-1000}}).sort({a:-1}).limit(100).explain();
		varCL.find({b:{$lt:1000}}).explain();
		varCL.find({c:{$in:['abcdefghijkl1','abcdefghijkl2','abcdefghijkl3','abcdefghijkl4']},a:{$lt:0}}).explain();
	}catch(e)
	{
		println("get explain for collection with index fail! rc=" + e);
		throw e;
	}
	println("get explain for collection with index finished!");
	
	//clean test-env
	try{
		varCS.dropCL(CSPREFIX_CL);
		db.dropCS(CSPREFIX_CS);
	}catch(e)
	{
		println("clean test-evn fail! rc="+e);
		throw e;
	}
	println("clean test-evn succ!");
}


// Add inspect standalone run mode
try
{
   var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
   // Inspect the run mode is standalone or not
   if( true == commIsStandalone( db ) )
      throw "ModeStandAlone" ;
   main();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}
