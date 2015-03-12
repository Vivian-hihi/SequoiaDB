/******************************************************************************
@Description : 1.query use Operator $nin with index
@Modify list :
               2014-08-08 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
CSPREFIX_IDX = CSPREFIX_CL+"_idx" ;

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
	println("createCS " + CSPREFIX_CS + " finished");
}catch(e)
{
	//collection space already exist,use it
	if(e != -33)
	{
		println("can't create CS:" + CSPREFIX_CS + " rc="+e);
		throw e;
	}
	else
	{
		varCS = db.getCS(CSPREFIX_CS);
		println("use CS:" + CSPREFIX_CS);
	}	
}

//create CL
try{
	var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:0,Compressed:true});
	println("create CL finished");
}catch(e)
{
	//collection already exist,use it
	if(e != -22)
	{
		println("can't create CL:" + CSPREFIX_CL + " rc="+e);
		throw e;
	}
	else
	{
		varCL = varCS.getCL(CSPREFIX_CL);
		varCL.remove();
		println("use CL:" + CSPREFIX_CL);
	}
}

//insert data
try{
	for(i=0;i<10;i++)
	{
		varCL.insert({a:1*i, b:10*i});
	}
}catch(e)
{
	println("insert data fail! rc="+e);
	throw e;
}
println("insert data finished");

//no index
//select * from ... where a not in(9,8,7,6,5,4,2,1,0) and b=30
try{
	var sel = varCL.find({a:{$nin:[9,8,7,6,5,4,2,1,0]},b:30});
	var size=0;
	var exprows = 1;
  var flag=true;
  while(sel.next())
  {
  	size++;
  	if(size>exprows)
  	{
  		flag = false;
  		throw 1;
  	}
  	var ret = sel.current();
  	//expected result:{a:3,b:30}
  	if(ret.toObj()['a']!=3 || ret.toObj()['b']!=30)
  	{
  		flag = false;
  		throw 2;
  	}
  }
  sel.close();
  if(flag && size!=exprows)
  {
  	flag = false;
  	throw 1;
  }
}catch(e)
{
	if(e!=1 && e!=2)
	{
		println("select data fail! rc="+e);
		throw e;
	}
	else if(e==1)
	{
		println("return rows not expected! expected:" + exprows + " return:"+ size +(size>exprows?" or more":""));
		throw e;
	}
	else if(e==2)
	{
		println("return incorrect record! expected:{a:3,b:30} returned:"+ret);
		throw e;
	}
}
println("select data without index finished!");

//create index
try{
	varCL.createIndex(CSPREFIX_IDX,{a:1});
	println("create index finished");
}catch(e)
{
	//when redefine index, ignore the exception
	if(e != -247)
	{
		println("can't create index:" + CSPREFIX_IDX + " rc="+e);
		throw e;
	}
	println("already exist index:" + CSPREFIX_IDX);
}
//select * from ... where a not in(10,8,6,7,5,3,2,1,9,0) and b in(50,40,30); 
try{
	var sel = varCL.find({a:{$nin:[10,8,6,7,5,3,2,1,9,0]},b:{$in:[50,40,30]}}).sort({a:1}).hint({"":CSPREFIX_IDX});
	var size=0;
	var exprows = 1;
  var flag=true;
  while(sel.next())
  {
  	size++;
  	if(size>exprows)
  	{
  		flag = false;
  		throw 1;
  	}
  	var ret = sel.current();
  	//expected result:{a:4,b:40}
  	if(ret.toObj()['a']!=4 || ret.toObj()['b']!=40)
  	{
  		flag = false;
  		throw 2;
  	}
  }
  sel.close();
  if(flag && size!=exprows)
  {
  	flag = false;
  	throw 1;
  }
}catch(e)
{
	if(e!=1 && e!=2)
	{
		println("select data fail! rc="+e);
		throw e;
	}
	else if(e==1)
	{
		println("return rows not expected! expected:" + exprows + " return:"+ size +(size>exprows?" or more":""));
		throw e;
	}
	else if(e==2)
	{
		println("return incorrect record!");
		println("the incorrect record:" + ret);
		throw e;
	}
}
println("select data with index finished!");

try{
	varCL.dropIndex(CSPREFIX_IDX);
}catch(e)
{
}

//combined index
try{
	varCL.createIndex(CSPREFIX_IDX,{a:1,b:1});
	println("create index finished");
}catch(e)
{
	//when redefine index, ignore the exception
	if(e != -247)
	{
		println("can't create index:" + CSPREFIX_IDX + " rc="+e);
		throw e;
	}
	println("already exist index:" + CSPREFIX_IDX);
}

//select * from ... where a not in(0,1,3,2,4) and b<40;
try{
	var sel = varCL.find({a:{$nin:[0,1,3,2,4]},b:{$lt:40}}).hint({"":CSPREFIX_IDX});
	var size=0;
	var exprows = 0;
  var flag=true;
  while(sel.next())
  {
  	size++;
  	if(size>exprows)
  	{
  		flag = false;
  		throw 1;
  	}
  	var ret = sel.current();
  }
  sel.close();
  if(flag && size!=exprows)
  {
  	flag = false;
  	throw 1;
  }
}catch(e)
{
	if(e!=1 && e!=2)
	{
		println("select data fail! rc="+e);
		throw e;
	}
	else if(e==1)
	{
		println("return rows not expected! expected:" + exprows +  " return:"+ size +(size>exprows?" or more":""));
		throw e;
	}
}
println("select data with combined index finished!");

//clean test-env
try{
	varCL.dropIndex(CSPREFIX_IDX);
	varCS.dropCL(CSPREFIX_CL);
	db.dropCS(CSPREFIX_CS);
}catch(e)
{
	println("clean test-evn fail! rc="+e);
	throw e;
}
println("clean test-evn succ!");
