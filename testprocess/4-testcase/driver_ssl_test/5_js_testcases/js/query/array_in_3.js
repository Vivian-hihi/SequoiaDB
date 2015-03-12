/* *****************************************************************************
@discretion: array: $in records are not only array-type
@modify list:
   						2014-02-02 Pusheng Ding  Init
***************************************************************************** */

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

//insert data
try{
	varCL.insert({a:1,b:"a",c:"not a array"});
	varCL.insert({a:[1],b:"b",c:"array"});
	varCL.insert({a:{a1:1},b:"c",c:"object"});
}catch(e)
{
	println("insert data failed! rc="+e);
	throw e;
}
println("insert data finished!");

//find({a:{$in:[1]}})
//result:
//		{a:1,b:"a",c:"not a array"}
//		{a:[1],b:"b",c:"array"}
println("********************************************");
try{
	var sel1 = varCL.find({a:{$in:[1]}}).sort({b:1});
}catch(e)
{
	println("find({a:{$in:[1]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	sel1.next();
	var rec1 = sel1.current();
	if(rec1.toObj()['b']!='a'){
		println("the 1st record is not expected!");
		println("expect:" + "{a:1,b:'a',c:'not a array'}");
		println("return:" + rec1);
		throw 'result1-error';
	}
	println("verify the 1st record correct!");
	sel1.next();
	var rec2 = sel1.current();
	if(rec2.toObj()['b']!='b'){
		println("the 2nd record is not expected!");
		println("expect:" + "{a:[1],b:'b',c:'array'}");
		println("return:" + rec2);
		throw 'result1-error';
	}
	println("verify the 2nd record correct!");
	sel1.next();
	if(sel1.size()!=0){
		println("records are more than expected!");
		throw "result1-number-error";
	}
	sel1.close();
}catch(e)
{
	if(e=="result1-error"){
		println("return result:" + sel1);
	}
	throw e;
}
println("find({b:{$in:[1]}}) succ!");

//create index
try{
	varCL.createIndex(CSPREFIX_IDX,{"a":1});
}catch(e){
	if(e!=-247){
		println("create index failed! rc="+e);
		throw e;
	}else{
		println(CSPREFIX_IDX + " index exists, use it");
	}
}
println("create index finished!");

//find({a:{$in:[1]}}).hint
//result:
//		{a:1,b:"a",c:"not a array"}
//		{a:[1],b:"b",c:"array"}
println("********************************************");
try{
	var sel2 = varCL.find({a:{$in:[1]}}).sort({b:1}).hint({"":CSPREFIX_IDX});
}catch(e)
{
	println("find({a:{$in:[1]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	sel2.next();
	var rec1 = sel2.current();
	if(rec1.toObj()['b']!='a'){
		println("the 1st record is not expected!");
		println("expect:" + "{a:1,b:'a',c:'not a array'}");
		println("return:" + rec1);
		throw 'result2-error';
	}
	println("verify the 1st record correct!");
	sel2.next();
	var rec2 = sel2.current();
	if(rec2.toObj()['b']!='b'){
		println("the 2nd record is not expected!");
		println("expect:" + "{a:[1],b:'b',c:'array'}");
		println("return:" + rec2);
		throw 'result2-error';
	}
	println("verify the 2nd record correct!");
	sel2.next();
	if(sel2.size()!=0){
		println("records are more than expected!");
		throw "result2-number-error";
	}
	sel2.close();
}catch(e)
{
	if(e=="result2-error"){
		println("return result:" + sel2);
	}
	throw e;
}
println("find({b:{$in:[1]}}).hint succ!");

//clean env
try{
	varCL.dropIndex(CSPREFIX_IDX);
}catch(e){
	println("drop index failed!rc="+e);
	throw e;
}
