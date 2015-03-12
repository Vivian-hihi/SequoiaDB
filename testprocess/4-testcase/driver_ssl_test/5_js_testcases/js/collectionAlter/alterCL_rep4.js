/******************************************************************************
@Description : 1. sub collection alters replsize
@Modify COORDHOSTNAME :
               2014-07-08  pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"subm" ;
SUBCL1NAME = CSPREFIX_CL+"_sub1";
SUBCL2NAME = CSPREFIX_CL+"_sub2";
SUBCL3NAME = CSPREFIX_CL+"_sub3";

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
	
	//create CS
	try{
		var varCS = db.createCS(CSPREFIX_CS);
	}catch(e)
	{
		println("can't create CS:" + CSPREFIX_CS + " rc="+e);
		throw e;
	}
	println("createCS " + CSPREFIX_CS + " finished");
	
	//create main-cl
	try{
		var mainCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{id:1} , ReplSize:2,Compressed:true,IsMainCL:true});
	}catch(e)
	{
		println("can't create main-CL:" + CSPREFIX_CL + " rc="+e);
		throw e;
	}
	println("createCL " + CSPREFIX_CL + " finished");
	
	//create sub-cl
	try{
		var subCL1 = varCS.createCL(SUBCL1NAME,{ShardingKey:{b:1},ShardingType:"hash",Partition:4096});
		var subCL2 = varCS.createCL(SUBCL2NAME,{ReplSize:2});
		var subCL3 = varCS.createCL(SUBCL3NAME,{Compressed:false});
	}catch(e)
	{
		println("can't create sub-CL");
		throw e;
	}
	
	//attach sub-cl
	try{
		mainCL.attachCL(CSPREFIX_CS + "." + SUBCL1NAME,{LowBound:{id:-10000},UpBound:{id:0}});
		mainCL.attachCL(CSPREFIX_CS + "." + SUBCL2NAME,{LowBound:{id:0},UpBound:{id:10000}});
		mainCL.attachCL(CSPREFIX_CS + "." + SUBCL3NAME,{LowBound:{id:10000},UpBound:{id:20000}});
	}catch(e)
	{
		println("attach sub-CL fail!");
		throw e;
	}
	println("attach sub-CL finish!");
	
	//insert data
	try{
		for(var i=0;i<30000;i++){mainCL.insert({id:i-10000,b:i,c:"abcdefghijkl"+i});}
	}catch(e)
	{
		println("insert-data into mainCL fail! rc="+e);
	}
	println("insert-data into mainCL succ!");
	
	//SubCL alters replsize
	try{
		subCL1.alter({ReplSize:3});
		var sn1 = db.snapshot(8,{Name:CSPREFIX_CS + "." + SUBCL1NAME});
		var replsize = sn1.current().toObj()['ReplSize'];
		if(replsize == 3)
		{
			println("SubCL1 alters replsize succ!");
		}
		else
		{
			println("SubCL1 alters replsize fail! ReplSize=" + replsize);
			throw 1;
		}
		
		subCL2.alter({ReplSize:2});
		var sn2 = db.snapshot(8,{Name:CSPREFIX_CS + "." + SUBCL2NAME});
		replsize = sn2.current().toObj()['ReplSize'];
		if(replsize == 2)
		{
			println("SubCL2 alters replsize succ!");
		}
		else
		{
			println("SubCL2 alters replsize fail! ReplSize=" + replsize);
			throw 1;
		}
		
		subCL3.alter({ReplSize:1});
		var sn3 = db.snapshot(8,{Name:CSPREFIX_CS + "." + SUBCL3NAME});
		replsize = sn3.current().toObj()['ReplSize'];
		if(replsize == 1)
		{
			println("SubCL3 alters replsize succ!");
		}
		else
		{
			println("SubCL3 alters replsize fail! ReplSize=" + replsize);
			throw 1;
		}
	}catch(e)
	{
		if(e != -1)
		{
			println("SubCL alters replsize fail! rc="+e);
		}
		throw e;
	}
	println("SubCL alters replsize finish!");
	
	//remove data
	try{
		mainCL.remove();
	}catch(e)
	{
		println("remove-data from mainCL fail! rc="+e);
	}
	println("remove-data from mainCL succ!");
	
	//insert data
	try{
		for(var i=0;i<30000;i++){mainCL.insert({id:i-10000,b:i,c:"abcdefghijkl"+i});}
	}catch(e)
	{
		println("insert-data into mainCL fail! rc="+e);
	}
	println("insert-data into mainCL succ!");
	
	//detachCL
	try{
		mainCL.detachCL(CSPREFIX_CS + "." + SUBCL1NAME);
		mainCL.detachCL(CSPREFIX_CS + "." + SUBCL2NAME);
		mainCL.detachCL(CSPREFIX_CS + "." + SUBCL3NAME);
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
   main();
}
catch( e )
{
   if( "ModeStandAlone" == e )
      println( "The run mode is standalone" ) ;
   else
      throw e ;
}
