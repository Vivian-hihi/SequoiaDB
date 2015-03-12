/******************************************************************************
@Description : 1.SEQUOIADBMAINSTREAM-293
									index scan hang when there's multiple predicates
								 fixed: SVN 13962 
@Modify list :
               2014-08-07 pusheng Ding  Init
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
	var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:0});
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

//create index
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


//insert data
try{
	varCL.insert({a:1, b:10});
	varCL.insert({a:2, b:20});
}catch(e)
{
	println("insert data fail! rc="+e);
	throw e;
}
println("insert data finished");

//select * from ... where a>=0 and b=10
try{
	var sel = varCL.find({a:{$gte:0},b:10}).hint({"":CSPREFIX_IDX});
	var size=0;
  var flag=true;
  while(sel.next())
  {
  	size++;
  	if(size>1)
  	{
  		flag = false;
  		throw 1;
  	}
  	var ret = sel.current();
  	if(ret.toObj()['a']!=1 || ret.toObj()['b']!=10)
  	{
  		flag = false;
  		throw 2;
  	}
  }
  sel.close();
  if(flag && size!=1)
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
		println("return rows not expected! expected:1 return:"+ size +(size>1?" or more":""));
		throw e;
	}
	else if(e==2)
	{
		println("return incorrect record! expected:{a:1, b:10} returned:" + ret);
		throw e;
	}
}
println("select data finished!");

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

