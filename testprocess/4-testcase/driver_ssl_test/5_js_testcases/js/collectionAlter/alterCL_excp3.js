/******************************************************************************
@Description : 1. collection altered properties which are not supported
@Modify list :
               2014-07-10 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL1 = CSPREFIX+"bar1" ;
CSPREFIX_CL2 = CSPREFIX+"bar2" ;
CSPREFIX_CL3 = CSPREFIX+"bar3" ;

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

try{
	normalCL.alter({Compress:true});
	throw 1;
}catch(e)
{
	if(e == 1)
	{
		println("normalCL alters Compress succ,but expect fail!");
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
	rangeCL.alter({ShardingKey:{id:1},ShardingType:'range'});
	throw 1;
}catch(e)
{
	if(e == 1)
	{
		println("rangeCL alters ShardingKey succ,but expect fail!");
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
	hashCL.alter({Partition:1024});
	throw 1;
}catch(e)
{
	if(e == 1)
	{
		println("hashCL alters Partition-num succ,but expect fail!");
		throw e;
	}
}
println("hashCL test finish!");

//clean test-env
try{
	varCS.dropCL(CSPREFIX_CL1);
	varCS.dropCL(CSPREFIX_CL2);
	varCS.dropCL(CSPREFIX_CL3);
	db.dropCS(CSPREFIX_CS);
}catch(e)
{
	println("clean test-evn fail! rc="+e);
	throw e;
}
println("clean test-evn succ!");

