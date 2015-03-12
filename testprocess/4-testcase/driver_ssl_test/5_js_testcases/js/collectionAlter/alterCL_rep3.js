/******************************************************************************
@Description : 1. main collection alters replsize
@Modify list :
               2014-07-04  pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"main" ;
SUBCL1NAME = CSPREFIX_CL+"_sub1";
SUBCL2NAME = CSPREFIX_CL+"_sub2";
SUBCL3NAME = CSPREFIX_CL+"_sub3";

try{
	var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME) ;
	var isStandalone = commIsStandalone( db ) ;
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

//mainCL-noSubCL alters replsize
try{
	mainCL.alter({ReplSize:2});
	var sn1 = db.snapshot(8,{Name:CSPREFIX_CS + "." + CSPREFIX_CL});
	var replsize = sn1.current().toObj()['ReplSize'];
	if(replsize == 2)
	{
		println("mainCL-noSubCL alters replsize succ!");
	}
	else
	{
		println("mainCL-noSubCL alters replsize fail! ReplSize=" + replsize);
		throw 1;
	}	
}catch(e)
{
	if(!isStandalone)
	{
		if(e != -1)
		{
			println("mainCL-noSubCL alters replsize fail! rc="+e);
		}
		throw e;
	}
	else
	{
		println("mainCL-noSubCL alters replsize fail when standalone");
	}
}
println("mainCL-noSubCL alters replsize finish!");

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
	if(!isStandalone)
	{
		println("attach sub-CL fail!");
		throw e;
	}
}
println("attach sub-CL finish!");

//mainCL-SubCL alters replsize
try{
	mainCL.alter({ReplSize:3});
	var sn1 = db.snapshot(8,{Name:CSPREFIX_CS + "." + CSPREFIX_CL});
	var replsize = sn1.current().toObj()['ReplSize'];
	if(replsize == 3)
	{
		println("mainCL-SubCL alters replsize succ!");
	}
	else
	{
		println("mainCL-SubCL alters replsize fail! ReplSize=" + replsize);
		throw 1;
	}	
}catch(e)
{
	if(!isStandalone)
	{
		if(e != -1)
		{
			println("mainCL-SubCL alters replsize fail! rc="+e);
		}
		throw e;
	}
	else
	{
		println("mainCL-noSubCL alters replsize fail when standalone");
	}
}
println("mainCL-SubCL alters replsize finish!");

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
	println("detach subCL succ!");
}catch(e)
{
	if(!isStandalone)
	{
		println("detachCL fail! rc="+e);
		throw e;
	}
}

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