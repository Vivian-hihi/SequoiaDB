/******************************************************************************
@Description : 1.query use Operator $type with index
@Modify list :
               2014-08-11 pusheng Ding  Init
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
	//32-bit integer
	varCL.insert({a:214748364,b:16});
	//64-bit integer
	varCL.insert({a:42474836478,b:18});
	//double
	varCL.insert({a:1.5e+100,b:1});
	//string
	varCL.insert({a:'abcd',b:2});
	//ObjectID
	varCL.insert({a:{$oid:'5156c192f970aed30c020000'},b:7});
	//boolean
	varCL.insert({a:true,b:8});
	//date
	varCL.insert({a:{$date:'2014-08-11'},b:9});
	//timestamp
	varCL.insert({a:{$timestamp:'2014-08-11-15.35.44.123456'},b:17});
	//Binary data
	varCL.insert({a:{$binary:'aGVsbG8gd29ybGQ=',$type:'1'},b:5});
	//Regular expression
	varCL.insert({a:{$regex:'^W',$options:'i'},b:11});
	//Object
	varCL.insert({a:{a1:'object'},b:3});
	//Array
	varCL.insert({a:[1,2,3,4],b:4});
	//null
	varCL.insert({a:null,b:10});
	varCL.insert({b:10});
}catch(e)
{
	println("insert data fail! rc="+e);
	throw e;
}
println("insert data finished");

//no index
//select * from ... where a type is binary and b>0;
try{
	var sel = varCL.find({a:{$type:5},b:{$gt:0}});
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
  	//expected result:{a:{$binary:'aGVsbG8gd29ybGQ=',$type:1},b:5}
  	if( ret.toObj()['b']!=5 )
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
		println("return incorrect record! expected:{a:{$binary:'aGVsbG8gd29ybGQ=',$type:1},b:5} returned:"+ret);
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
//select * from ... where a type is null and b=10;
try{
	var sel = varCL.find({a:{$type:10},b:{$et:10}}).hint({"":CSPREFIX_IDX});
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
  	//expected result:{a:null,b:10}
  	if( ret.toObj()['a']!=null || ret.toObj()['b']!=10)
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
		println('return incorrect record! expected:{a:null,b:10} returned:'+ret);
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

//select * from ... where a type is Array and b%2=0
//SEQUOIADBMAINSTREAM-305, expected result rows:0
try{
	var sel = varCL.find({a:{$type:4},b:{$mod:[2,0]}}).hint({"":CSPREFIX_IDX});
	var size=0;
	var exprows = 0;
  var flag=true;
  while(sel.next())
  {
  	size++;
  	if(size>exprows && size != 1)
  	{
  		flag = false;
  		throw 1;
  	}
  	var ret = sel.current();
 
  }
  sel.close();
  if(flag && size!=exprows && size != 1 )
  {
  	flag = false;
  	throw 1;
  }
}catch(e)
{
	if(e!=1)
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
