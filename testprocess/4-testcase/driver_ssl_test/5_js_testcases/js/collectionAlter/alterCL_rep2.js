/******************************************************************************
@Description : 1. hash collection alters replsize
@Modify list :
               2014-07-08  pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"hash1" ;

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
		var group_num = grouplist.length;
		if(group_num == 1)
		{
			println("only one ReplicaGroup:" + grouplist + " Skip the testcase");
			return;
		}
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
	
	//create hash-cl
	try{
		var hashCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{id:1,b:-1},ShardingType:'hash',Partition:4096,ReplSize:2});
		var sn1 = db.snapshot(8,{Name:CSPREFIX_CS+"."+CSPREFIX_CL});
		var sourceGroup = sn1.current().toObj()['CataInfo'][0]['GroupName'];
	}catch(e)
	{
		println("can't create hash-CL:" + CSPREFIX_CL + " rc="+e);
		throw e;
	}
	println("createCL " + CSPREFIX_CL + " at ReplicaGroup:" + sourceGroup + " finished");
	
	//hashCL-noSplit alters replsize
	try{
		hashCL.alter({ReplSize:2});
		var sn1 = db.snapshot(8,{Name:CSPREFIX_CS + "." + CSPREFIX_CL});
		var replsize = sn1.current().toObj()['ReplSize'];
		if(replsize == 2)
		{
			println("hashCL-noSplit alters replsize succ!");
		}
		else
		{
			println("hashCL-noSplit alters replsize fail! ReplSize=" + replsize);
			throw 1;
		}	
	}catch(e)
	{
		if(e != -1)
		{
			println("hashCL-noSplit alters replsize fail! rc="+e);
		}
		throw e;
	}
	println("hashCL-noSplit alters replsize finish!");
	
	//split ({Partition:1024} {Partition:2048}) {Partition:3072})
	try{
		var tarGroupIndex=-1;
		var stepPar = 1024;
		var partId = 3;
		var lowPar = 0;
		var highPar = 0;
		for(var i=0;i<partId;i++){
			tarGroupIndex++;
			if(tarGroupIndex == group_num)
				tarGroupIndex=0;
			if(grouplist[tarGroupIndex]==sourceGroup)
			{
				i--;
				continue;
			}
			lowPar = i*stepPar;
			highPar = (i+1)*stepPar;
			hashCL.split(sourceGroup, grouplist[tarGroupIndex],{Partition:lowPar},{Partition:highPar});
			println(CSPREFIX_CL+" split from "+sourceGroup+" to "+ grouplist[tarGroupIndex]+" {Partition:"+lowPar+"} {Partition:"+highPar+"}");
		}
	}catch(e)
	{
		println("split hashCL fail! rc="+e);
		throw e;
	}
	println("split hashCL succ!");
	
	//hashCL-split alters replsize
	try{
		hashCL.alter({ReplSize:1});
		var sn1 = db.snapshot(8,{Name:CSPREFIX_CS + "." + CSPREFIX_CL});
		var replsize = sn1.current().toObj()['ReplSize'];
		if(replsize == 1)
		{
			println("hashCL-split alters replsize succ!");
		}
		else
		{
			println("hashCL-split alters replsize fail! ReplSize=" + replsize);
			throw 1;
		}	
	}catch(e)
	{
		if(e != -1)
		{
			println("hashCL-split alters replsize fail! rc="+e);
		}
		throw e;
	}
	println("hashCL-split alters replsize finish!");
	
	//insert data
	try{
		for(var i=0;i<30000;i++){hashCL.insert({id:i-10000,b:i,c:"abcdefghijkl"+i});}
	}catch(e)
	{
		println("insert-data into hashCL fail! rc="+e);
	}
	println("insert-data into hashCL succ!");
	
	//select * from bar where id=1
	//expect one record
	try{
		var sel = hashCL.find({id:{$et:1}});
		var size=0;
		var flag=false;
		while(sel.next())
		{
			size++;
			if(size>100)
				break;
			var ret = sel.current();
			if(ret.toObj()['id']==1 && ret.toObj()['b']==10001 && ret.toObj()['c']=='abcdefghijkl10001')
				flag = true;
		}
		if(size!=1)
		{
			throw 1;
		}
		if(!flag)
		{
			throw 2;
		}	
	}catch(e)
	{
		if(e==-1)
			println("result-records count not expected. expect:1 return:"+size);
		else if(e==-2)
		{	
			println("record not expected!");
			println("expected:{id:1,b:10001,c:'abcdefghijkl10001'}");
			println("returned:"+ret);
		}
		else
			println("select " + CSPREFIX_CL + " fail! rc="+e);
		throw e;
	}
	println("data-verify succ!");
	
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

