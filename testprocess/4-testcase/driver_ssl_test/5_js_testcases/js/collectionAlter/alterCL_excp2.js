/******************************************************************************
@Description : 1. collection altered Compress, fail
@Modify list :
               2014-07-10 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL1 = CSPREFIX+"bar1" ;
CSPREFIX_CL2 = CSPREFIX+"bar2" ;
CSPREFIX_CL3 = CSPREFIX+"bar3" ;
CSPREFIX_CL4 = CSPREFIX+"bar4" ;

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

//create normalCL
try{
	var normalCL = varCS.createCL(CSPREFIX_CL1,{ReplSize:0,Compressed:true});
}catch(e)
{
	println("can't create normal-CL:" + CSPREFIX_CL1 + " rc="+e);
	throw e;
}
println("create normalCL finished");

//Compressed True to False,expect fail
try{
	normalCL.alter({ShardingKey:{id:1},ShardingType:'hash',Compressed:false});
	throw 1;
}catch(e)
{
	if(e == 1)
	{
		println("normalCL alters Compressed succ,but expect fail!");
		throw e;
	}
}
println("normalCL test finish!");

//create rangeCL
try{
	var rangeCL = varCS.createCL(CSPREFIX_CL2,{ShardingKey:{a:1,b:1},ShardingType:'range',ReplSize:1});
}catch(e)
{
	println("can't create rangeCL:" + CSPREFIX_CL2 + " rc="+e);
	throw e;
}
println("create rangeCL finished");

try{
	rangeCL.alter({Compressed:true});
	throw 1;
}catch(e)
{
	if(e == 1)
	{
		println("rangeCL alters Compressed succ,but expect fail!");
		throw e;
	}
}
println("rangeCL test finish!");

//create hashCL
try{
	var hashCL = varCS.createCL(CSPREFIX_CL3,{ShardingKey:{b:1},ShardingType:'hash',Compressed:true});
}catch(e)
{
	println("can't create hashCL:" + CSPREFIX_CL3 + " rc="+e);
	throw e;
}
println("create hashCL finished");

try{
	hashCL.alter({Compressed:true});
	throw 1;
}catch(e)
{
	if(e == 1)
	{
		println("hashCL alters Compressed succ,but expect fail!");
		throw e;
	}
}
println("hashCL test finish!");

//create mainCL
try{
	var mainCL = varCS.createCL(CSPREFIX_CL4,{ShardingKey:{b:1},IsMainCL:true});
}catch(e)
{
	println("can't create mainCL:" + CSPREFIX_CL4 + " rc="+e);
	throw e;
}
println("create mainCL finished");

try{
	mainCL.alter({Compressed:true});
	throw 1;
}catch(e)
{
	if(e == 1)
	{
		println("mainCL alters Compressed succ,but expect fail!");
		throw e;
	}
}
println("mainCL test finish!");

//clean test-env
try{
	varCS.dropCL(CSPREFIX_CL1);
	varCS.dropCL(CSPREFIX_CL2);
	varCS.dropCL(CSPREFIX_CL3);
	varCS.dropCL(CSPREFIX_CL4);
	db.dropCS(CSPREFIX_CS);
}catch(e)
{
	println("clean test-evn fail! rc="+e);
	throw e;
}
println("clean test-evn succ!");

