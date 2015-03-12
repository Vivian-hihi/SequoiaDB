/******************************************************************************
@Description : 1.SEQUOIADBMAINSTREAM-274:
									range-cl is queried error when use operator OR and after split

@Modify list :
               2014-07-17 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
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

   try{
      db.dropCS( CSPREFIX_CS );
   }catch( e ){}
   	
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
      if(groups_num == 1)
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
	
	//insert data
	try{
		for(var i=0;i<10000;i++){varCL.insert({a:i-10000,b:i,c:"abcdefghijkl"+i});}
	}catch(e)
	{
		println("insert-data fail! rc="+e);
	}
	println("insert-data succ!");
	
	//create index
	try{
		varCL.createIndex(CSPREFIX_IDX,{a:1},false,false);
	}catch(e)
	{
		println("cat't create index " + CSPREFIX_IDX + " rc=" + e);
		throw e;
	}
	println("create index finished!");
	
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
	
	//select * from ... where a<=-9000 or a>=-1000 or b=2000
	try{
		var sel = varCL.find({$or:[{a:{$lte:-9000}},{a:{$gte:-1000}}, {b:2000}]}).sort({a:1});
		var size=0;
		var flag=true;
		var expected = 2002;
		while(sel.next())
	  {
	  	size++;
	  	if(size>expected)
	  	{
	  		flag = false;
	  		throw 1;
	  	}
	  	var ret = sel.current();
	  	if(!(ret.toObj()['a']<=-9000 || ret.toObj()['a']>=-1000 || ret.toObj()['b']==2000))
	  	{
	  		flag = false;
	  		throw 2;
	  	}
	  }
	  sel.close();
	  if(flag && size!=expected)
	  {
	  	flag = false;
	  	throw 1;
	  }
	}catch(e){
		if(e!=1 && e!=2)
		{
			println("select data fail! rc="+e);
			throw e;
		}
		else if(e==1)
		{
			println("return rows not expected! expected:"+expected+" return:"+ size +(size>expected?" or more":""));
			throw e;
		}
		else if(e==2)
		{
			println("return incorrect record! " + ret);
			throw e;
		}
	}
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
