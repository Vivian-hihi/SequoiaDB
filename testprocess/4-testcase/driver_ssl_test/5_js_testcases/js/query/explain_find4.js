/******************************************************************************
@Description : 1. main collection's query explain
@Modify list :
               2014-07-15 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
SUBCL1NAME = CSPREFIX_CL+"_sub1";
SUBCL2NAME = CSPREFIX_CL+"_sub2";
SUBCL3NAME = CSPREFIX_CL+"_sub3";
CSPREFIX_IDX = CSPREFIX+"idx" ;

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
	
	//create varCL
	try{
		var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:0,IsMainCL:true,ShardingKey:{id:1}});
	}catch(e){
		println("can't create main-CL:" + CSPREFIX_CL + " rc="+e);
		throw e;
	}
	println("create varCL finished");
	
	//create sub-CL
	try{
		var subCL1 = varCS.createCL(SUBCL1NAME,{ReplSize:0,ShardingKey:{b:1},ShardingType:"hash",Partition:1024});
		var sn1 = db.snapshot(8,{Name:CSPREFIX_CS+"."+SUBCL1NAME});
		var sourceGroup1 = sn1.current().toObj()['CataInfo'][0]['GroupName'];
		var subCL2 = varCS.createCL(SUBCL2NAME,{ReplSize:0,ShardingKey:{b:1},ShardingType:'range'});
		var sn2 = db.snapshot(8,{Name:CSPREFIX_CS+"."+SUBCL2NAME});
		var sourceGroup2 = sn2.current().toObj()['CataInfo'][0]['GroupName'];
		var subCL3 = varCS.createCL(SUBCL3NAME,{ReplSize:0,Compressed:false});
	}catch(e)
	{
		println("create subCLs fail! rc="+e);
		throw e;
	}
	println("create subCLs finished");
	
	//attach sub-cl
	try{
		varCL.attachCL(CSPREFIX_CS + "." + SUBCL1NAME,{LowBound:{id:-10000},UpBound:{id:0}});
		varCL.attachCL(CSPREFIX_CS + "." + SUBCL2NAME,{LowBound:{id:0},UpBound:{id:10000}});
		varCL.attachCL(CSPREFIX_CS + "." + SUBCL3NAME,{LowBound:{id:10000},UpBound:{id:20000}});
	}catch(e)
	{
		println("attach sub-CL fail!");
		throw e;
	}
	println("attach sub-CL finish!");
	
	//insert data
	try{
		for(var i=0;i<CATASVCNAME;i++){varCL.insert({id:i-10000,b:i,c:"abcdefghijkl"+i});}
		println("insert-data succ");
	}catch(e)
	{
		println("insert-data fail! rc="+e);
		throw e;
	}
	
	//get explain for collection without split
	try{
		varCL.find().explain();
		varCL.find({id:{$et:-9900},b:{$et:100}},{b:1,c:'abcd'}).explain();
		varCL.find({id:{$lt:-5000},b:{$in:[3000,4000,5000,6000,7000]}}).sort({b:1}).explain();
		varCL.find().sort({id:-1}).limit(100).skip(1000).explain();
		varCL.find({c:{$isnull:0}},{}).hint({"":CSPREFIX_IDX}).explain();
	}catch(e)
	{
		println("get explain for collection without split fail! rc=" + e);
		throw e;
	}
	println("get explain for collection without split finished!");
	
	//split subCL2
	try{
		if(groups_num>1)
		{
			var tarGroupIndex=-1;
			var stepId = 2000;
			var partId = 5;
			var lowId = 0;
			var highId = 0;
			for(var i=0;i<partId;i++){
				tarGroupIndex++;
				if(tarGroupIndex == groups_num)
					tarGroupIndex=0;
				if(grouplist[tarGroupIndex]==sourceGroup2)
				{
					i--;
					continue;
				}
				lowId = (i-1)*stepId + 10000;
				highId = i*stepId + 10000;
				subCL2.split(sourceGroup2, grouplist[tarGroupIndex],{b:lowId},{b:highId});
				println(SUBCL2NAME+" split from "+sourceGroup2+" to "+ grouplist[tarGroupIndex]+" {b:"+lowId+"} {b:"+highId+"}");
			}
		}
	}catch(e)
	{
		println(SUBCL2NAME + " split fail! rc="+e);
		throw e;
	}
	println(SUBCL2NAME + " split finish!");
	
	//create index
	try{
		varCL.createIndex(CSPREFIX_IDX,{id:1,b:1},false,false);
	}catch(e)
	{
		println("cat't create index " + CSPREFIX_IDX + " rc=" + e);
		throw e;
	}
	println("create index finished!");
	
	//split subCL1 
	try{
		if(groups_num>1)
		{
			tarGroupIndex=-1;
			var stepPar = 256;
			var partId = 3;
			var lowPar = 0;
			var highPar = 0;
			for(var i=0;i<partId;i++){
				tarGroupIndex++;
				if(tarGroupIndex == groups_num)
					tarGroupIndex=0;
				if(grouplist[tarGroupIndex]==sourceGroup1)
				{
					i--;
					continue;
				}
				lowPar = i*stepPar;
				highPar = (i+1)*stepPar;
				subCL1.split(sourceGroup1, grouplist[tarGroupIndex],{Partition:lowPar},{Partition:highPar});
				println(SUBCL1NAME+" split from "+sourceGroup1+" to "+ grouplist[tarGroupIndex]+" {Partition:"+lowPar+"} {Partition:"+highPar+"}");
			}
		}
	}catch(e)
	{
		println(SUBCL1NAME + " split fail! rc="+e);
		throw e;
	}
	println(SUBCL1NAME + " split finish!");
	
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
		varCL.find({$or:[{a:{$lte:9000}},{a:{$gte:-1000}}, {b:20000}]},{c:'a',a:0}).sort({a:1}).skip(1000).explain();
		varCL.find({c:{$elemMatch:{c:'abcdefghijkl2000'}}}).limit(1).skip(0).explain();
	}catch(e)
	{
		println(CSPREFIX_CL + ".find.skip.explain fail! rc=" + e);
	}
	println(CSPREFIX_CL + ".find.skip.explain finished");
	
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
		varCL.find({a:{$gt:15000}}).sort({a:-1}).limit(100).explain();
		varCL.find({b:{$lt:10000}}).explain();
		varCL.find({c:{$in:['abcdefghijkl1','abcdefghijkl2','abcdefghijkl3','abcdefghijkl4']},a:{$lt:0}}).explain();
	}catch(e)
	{
		println("get explain for collection with index fail! rc=" + e);
		throw e;
	}
	println("get explain for collection with index finished!");
	
	//detachCL
	try{
		varCL.detachCL(CSPREFIX_CS + "." + SUBCL1NAME);
		varCL.detachCL(CSPREFIX_CS + "." + SUBCL2NAME);
		varCL.detachCL(CSPREFIX_CS + "." + SUBCL3NAME);
	}catch(e)
	{
		println("detachCL fail! rc="+e);
		throw e;
	}
	println("detach subCL succ!");
	
	//clean test-env
	try{
		varCS.dropCL(SUBCL1NAME);
		varCS.dropCL(SUBCL2NAME);
		varCS.dropCL(SUBCL3NAME);
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
   // set session get data from master
   db.setSessionAttr( {"PreferedInstance":"M"} ) ;
   main();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}
