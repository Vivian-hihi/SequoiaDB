/******************************************************************************
@Description : 1.jira510: $all array index
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
}

//insert data
try{
	varCL.insert({a:[1,2]});
	varCL.insert({a:[3,1]});
	varCL.insert({a:[1,2]});
	varCL.insert({a:[1,4]});
}catch(e)
{
	println("insert data fail! rc="+e);
	throw e;
}
println("insert data finished");

//query1 without index
try{
	var sel = varCL.find({a:{$all:[1,2]}});
	var size=0;
	var exprows = 2;
	var flag=true;
	while(sel.next()){
		size++;
  	if(size>exprows)
  	{
  		flag = false;
  		throw "query1-without-index-resultnumber-error";
  	}
  	var ret = sel.current();
  	if(ret.toObj()['a']!='1,2' && ret.toObj()['a']!='2,1'){
  		flag = false;
  		throw "query1-without-index-result-error";
  	}
	}
	sel.close();
	if(flag && size!=exprows)
  {
  	flag = true;
  	throw "query1-without-index-resultnumber-error";
  }
}catch(e)
{
	if(e=="query1-without-index-resultnumber-error")
	{
		println("return rows not expected! expected:" + exprows + " return:"+ size +(size>exprows?" or more":""));
		throw e;
	}
	else if(e=="query1-without-index-result-error")
	{
		println("return incorrect record! expected:{a:[1,2]} returned:"+ret);
		throw e;
	}else{
		println("select data fail! rc="+e);
		throw e;
	}
}
println("query without index finished");

//create index
try{
	varCL.createIndex(CSPREFIX_IDX,{a:1});
}catch(e)
{
	println("can't create index:" + CSPREFIX_IDX + " rc="+e);
	throw e;
}
println("create index finished");

//query2 with index
try{
	var sel = varCL.find({a:{$all:[1,2]}});
	var size=0;
	var exprows = 2;
	var flag=true;
	while(sel.next()){
		size++;
  	if(size>exprows)
  	{
  		flag = false;
  		throw "query2-with-index-resultnumber-error";
  	}
  	var ret = sel.current();
  	if(ret.toObj()['a']!='1,2' && ret.toObj()['a']!='2,1'){
  		flag = false;
  		throw "query2-with-index-result-error";
  	}
	}
	sel.close();
	if(flag && size!=exprows)
  {
  	flag = true;
  	throw "query2-with-index-resultnumber-error";
  }
}catch(e)
{
	if(e=="query2-with-index-resultnumber-error")
	{
		println("return rows not expected! expected:" + exprows + " return:"+ size +(size>exprows?" or more":""));
		throw e;
	}
	else if(e=="query2-with-index-result-error")
	{
		println("return incorrect record! expected:{a:[1,2]} returned:"+ret);
		throw e;
	}else{
		println("select data fail! rc="+e);
		throw e;
	}
}
println("query with index finished");

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