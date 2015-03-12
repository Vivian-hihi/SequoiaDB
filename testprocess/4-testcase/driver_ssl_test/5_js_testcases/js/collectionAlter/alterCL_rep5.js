/******************************************************************************
@Description : 1. normal collection alters replsize
@Modify list :
               2014-07-08  pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"normal1" ;

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

//create normal-CL
try{
	var normalCL = varCS.createCL(CSPREFIX_CL,{Compressed:true});
}catch(e)
{
	println("can't create normal-CL:" + CSPREFIX_CL + " rc="+e);
	throw e;
}
println("createCL " + CSPREFIX_CL + " finished");

//normalCL alters replsize once
try{
	normalCL.alter({ReplSize:2});
	var sn1 = db.snapshot(8,{Name:CSPREFIX_CS + "." + CSPREFIX_CL});
	var replsize = sn1.current().toObj()['ReplSize'];
	if(replsize == 2)
	{
		println("normalCL alters replsize succ once!");
	}
	else
	{
		println("normalCL alters replsize fail once! ReplSize=" + replsize);
		throw 1;
	}	
}catch(e)
{
	if(!isStandalone)
	{
		if(e != -1)
		{
			println("normalCL alters replsize fail once! rc="+e);
		}
		throw e;
	}
	else
	{
		println("normalCL alters replsize fail once when standalone");
	}
}
println("normalCL alters replsize finish once!");

//insert data
try{
	for(var i=0;i<10000;i++){normalCL.insert({id:i-10000,b:i,c:"abcdefghijkl"+i});}
}catch(e)
{
	println("insert-data into normalCL fail! rc="+e);
}
println("insert-data into normalCL succ!");

//normalCL alters replsize twice
try{
	normalCL.alter({ReplSize:3});
	var sn1 = db.snapshot(8,{Name:CSPREFIX_CS + "." + CSPREFIX_CL});
	var replsize = sn1.current().toObj()['ReplSize'];
	if(replsize == 3)
	{
		println("normalCL alters replsize succ twice!");
	}
	else
	{
		println("normalCL alters replsize fail twice! ReplSize=" + replsize);
		throw 1;
	}	
}catch(e)
{
	if(!isStandalone)
	{
		if(e != -1)
		{
			println("normalCL alters replsize fail twice! rc="+e);
		}
		throw e;
	}
	else
	{
		println("normalCL alters replsize fail twice when standalone");
	}
}
println("normalCL alters replsize finish twice!");

//remove data
try{
	normalCL.remove();
}catch(e)
{
	println("remove-data from normalCL fail! rc="+e);
}
println("remove-data from normalCL succ!");

//insert data
try{
	for(var i=0;i<5000;i++){normalCL.insert({id:i,b:i-2500,c:"abcdefghijkl"+i});}
}catch(e)
{
	println("insert-data into normalCL fail! rc="+e);
}
println("insert-data into normalCL succ!");

//select * from bar where id=1
//expect one record
try{
	var sel = normalCL.find({id:{$et:1}});
	var size=0;
	var flag=false;
	while(sel.next())
	{
		size++;
		if(size>100)
			break;
		var ret = sel.current();
		if(ret.toObj()['id']==1 && ret.toObj()['b']==-2499 && ret.toObj()['c']=='abcdefghijkl1')
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