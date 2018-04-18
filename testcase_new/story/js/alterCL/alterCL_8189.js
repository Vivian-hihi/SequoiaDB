/******************************************************************************
*@Description : 1.hash-collection altered to partition-collection
*@Modify list :
*               2014-07-09  pusheng Ding  Init
*               2015-03-28  xiaojun Hu    Changed
******************************************************************************/

function main()
{
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
		if(groups_num < 2) {
		   println("groupsNum < 2, there is no enough data group");
		   return;
		}
	}catch(e)
	{
		println("get ReplicaGroups info fail! rc="+e);
		throw e;
	}
	println("ReplicaGroups: " + grouplist);

try
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop colleciton in the beginning" );
}
catch( e )
{
   println( "failed to clean in the beginning" + e ) ;
   throw e ;
}
	//create hash-CL
	try{
      var optionObj = {ShardingKey:{id:1},ShardingType:'hash',
                       ReplSize:0,Partition:4096,Compressed:true};
      var hashCL = commCreateCLByOption( db, COMMCSNAME, COMMCLNAME, optionObj, true,
                                          false, "create collecton failed" );
		var sn1 = db.snapshot(8,{Name:COMMCSNAME+"."+COMMCLNAME});
		var sourceGroup = sn1.current().toObj()['CataInfo'][0]['GroupName'];
	}catch(e)
	{
		println("can't create hash-CL:" + COMMCLNAME + " rc="+e);
		throw e;
	}
	println("createCL " + COMMCLNAME + " at ReplicaGroup:" + sourceGroup + " finished");
	
	//hashCL-noSplit altered to hash-collection, expect fail
	try{
		hashCL.alter({ShardingKey:{id:1},ShardingType:'hash',Partition:1024});
	}catch(e)
	{
	   println("1 hashCL altered to hash-collection fail,but expect succ!");
	   throw e;
	}
	try{
		hashCL.alter({ShardingKey:{id:1,b:-1},ShardingType:'hash'});
	}catch(e)
	{
      println("2 hashCL altered to hash-collection fail,but expect succ!");
	   throw e;
	}
	try{
		hashCL.alter({ShardingKey:{b:1,c:-1},ShardingType:'hash',Partition:4096});
	}catch(e)
	{
	   println("3 hashCL altered to range-collection fail,but expect succ!");
	   throw e;
	}
	println("hashCL-noSplit altered to hash-collection finish!");
	
	//hashCL-noSplit altered to range-collection, expect fail
	try{
		hashCL.alter({ShardingKey:{id:1},ShardingType:'range'});
	}catch(e)
	{
	   println("1 hashCL altered to range-collection fail, but expect succ!");
	   throw e;
	}
	
	try{
		hashCL.alter({ShardingKey:{id:1,b:-1},ShardingType:'range'});
	}catch(e)
	{
	   println("2 hashCL altered to range-collection fail,but expect succ!");
	   throw e;
	}
	
	try{
		hashCL.alter({ShardingKey:{b:1,c:-1},ShardingType:'range'});
	}catch(e)
	{
	   println("3 hashCL altered to range-collection fail,but expect succ!");
	   throw e;
	}
	println("hashCL-noSplit altered to range-collection finish!");
	
	//split	
	try{
		if(groups_num>1){
			var tarGroupIndex=-1;
			var stepId = 1000;
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
				lowId = (i-1)*stepId;
				highId = i*stepId;
				/*println(sourceGroup);
				println(grouplist[tarGroupIndex]);
				println(lowId);
				println(highId);*/
				hashCL.split(sourceGroup, grouplist[tarGroupIndex],{b:lowId},{b:highId});
				println(COMMCLNAME+" split from "+sourceGroup+" to "+ grouplist[tarGroupIndex]+" {id:"+lowId+"} {id:"+highId+"}");
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
	
	//insert data
	try{
		for(var i=0;i<3000;i++){hashCL.insert({id:i,b:i-1000,c:"abcdefghijkl"+i});}
	}catch(e)
	{
		println("insert-data fail! rc="+e);
		throw e;
	}
	println("insert-data succ!");
	
	//hashCL-splited altered to range-collection, expect fail
	try{
		hashCL.alter({ShardingKey:{id:1},ShardingType:'range'});
		throw 1;
	}catch(e)
	{
		if(e == 1)
		{
			println("1 hashCL altered to range-collection succ,but expect fail!");
			throw e;
		}
	}
	
	try{
		hashCL.alter({ShardingKey:{id:1,b:-1},ShardingType:'range'});
		throw 1;
	}catch(e)
	{
		if(e == 1)
		{
			println("2 hashCL altered to range-collection succ,but expect fail!");
			throw e;
   	}
	}
	
	try{
		hashCL.alter({ShardingKey:{b:-1,c:1},ShardingType:'range'});
		throw 1;
	}catch(e)
	{
		if(e == 1)
		{
			println("3 hashCL altered to range-collection succ,but expect fail!");
			throw e;
   	}
	}
	println("hashCL-splited altered to range-collection finish!");
	
	//hashCL-splited data altered to hash-collection, expect fail
	try{
		hashCL.alter({ShardingKey:{id:1},ShardingType:'hash',Partition:4096});
		throw 1;
	}catch(e)
	{
		if(e == 1)
		{
			println("4 hashCL altered to hash-collection succ,but expect fail!");
			throw e;
   	}
	}
	
	try{
		hashCL.alter({ShardingKey:{id:1,b:-1},ShardingType:'hash'});
		throw 1;
	}catch(e)
	{
		if(e == 1)
		{
			println("5 hashCL altered to hash-collection succ,but expect fail!");
			throw e;
   	}
	}
	
	try{
		hashCL.alter({ShardingKey:{b:-1,c:1},ShardingType:'hash',Partition:1024});
		throw 1;
	}catch(e)
	{
		if(e == 1)
		{
			println("6 hashCL altered to hash-collection succ,but expect fail!");
			throw e;
   	}
	}
	println("hashCL-splited altered to hash-collection finish!");
	
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
			if(ret.toObj()['id']==1 && ret.toObj()['b']==-999 && ret.toObj()['c']=='abcdefghijkl1')
				flag = true;
		}
		if(size!=1)
		{
			throw -1;
		}
		if(!flag)
		{
			throw -2;
		}	
	}catch(e)
	{
		if(e==-1)
			println("result-records count not expected. expect:1 return:"+size);
		else if(e==-2)
		{	
			println("record not expected!");
			println("expected:{id:1,b:-999,c:'abcdefghijkl11'}");
			println("returned:"+ret);
		}
		else
			println("select " + COMMCLNAME + " fail! rc="+e);
		throw e;
	}
	println("data-verify succ!");
	
	//clean test-env
	try{
      commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
                  "drop colleciton in the end" );
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

