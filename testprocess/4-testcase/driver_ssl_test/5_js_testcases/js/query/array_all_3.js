/* *****************************************************************************
@discretion: array: $all records are not only array-type
@modify list:
   						2014-02-03 Pusheng Ding  Init
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
	varCL.insert({a:[1,2],b:"b",c:"array"});
	varCL.insert({a:{a1:1,a2:2},b:"c",c:"object"});
}catch(e)
{
	println("insert data failed! rc="+e);
	throw e;
}
println("insert data finished!");

//find({a:{$all:[1,2]}})
//result:
//		{a:[1,2],b:"b",c:"array"}
println("********************************************");
try{
	var sel1 = varCL.find({a:{$all:[1,2]}});
}catch(e){
	println("find({a:{$all:[1,2]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	sel1.next();
	var rec1 = sel1.current();
	if(rec1.toObj()['b']!='b'){
		println("the 1st record is not expected!");
		println("expect:" + "{a:[1,2],b:'b',c:'array'}");
		println("return:" + rec1);
		throw 'result1-error';
	}
	sel1.next();
	if(sel1.size()!=0){
		println("records are more than expected!");
		throw "result1-number-error";
	}
	sel1.close();
}catch(e){
	if(e=="result1-error"){
		println("return result:" + sel1);
	}
	throw e;
}
println("find({a:{$all:[1,2]}}) succ!");

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

//find({a:{$all:[1,2]}}).hit
//result:
//		{a:[1,2],b:"b",c:"array"}
println("********************************************");
try{
	var sel2 = varCL.find({a:{$all:[1,2]}}).hint({"":CSPREFIX_IDX});
}catch(e){
	println("find({a:{$all:[1,2]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	sel2.next();
	var rec1 = sel2.current();
	if(rec1.toObj()['b']!='b'){
		println("the 1st record is not expected!");
		println("expect:" + "{a:[1,2],b:'b',c:'array'}");
		println("return:" + rec1);
		throw 'result2-error';
	}
	sel2.next();
	if(sel2.size()!=0){
		println("records are more than expected!");
		throw "result2-number-error";
	}
	sel2.close();
}catch(e){
	if(e=="result2-error"){
		println("return result:" + sel2);
	}
	throw e;
}
println("find({a:{$all:[1,2]}}) succ!");

//find({a:{$all:[1]}}).hit
//result:
//		{a:[1,2],b:"b",c:"array"}
//		{a:1,b:"a",c:"not a array"}
println("********************************************");
try{
	var sel3 = varCL.find({a:{$all:[1]}}).sort({b:-1}).hint({"":CSPREFIX_IDX});
}catch(e){
	println("find({a:{$all:[1,2]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	sel3.next();
	var rec1 = sel3.current();
	if(rec1.toObj()['b']!='b'){
		println("the 1st record is not expected!");
		println("expect:" + "{a:[1,2],b:'b',c:'array'}");
		println("return:" + rec1);
		throw 'result3-error';
	}
	sel3.next();
	var rec2 = sel3.current();
	if(rec2.toObj()['b']!='a'){
		println("the 2nd record is not expected!");
		println("expect:" + "{a:1,b:'a',c:'not a array'}");
		println("return:" + rec2);
		throw 'result3-error';
	}
	println("verify the 2nd record correct!");
	sel3.next();
	if(sel2.size()!=0){
		println("records are more than expected!");
		throw "result3-number-error";
	}
	sel3.close();
}catch(e){
	if(e=="result3-error"){
		println("return result:" + sel3);
	}
	throw e;
}
println("find({a:{$all:[1]}}) succ!");

//find({a:{$all:[]}})
//result:
//		{a:1,b:"a",c:"not a array"}
//		{a:[1,2],b:"b",c:"array"}
//		{a:{a1:1,a2:2},b:"c",c:"object"}
println("********************************************");
try{
	var sel4 = varCL.find({a:{$all:[]}}).hint({"":CSPREFIX_IDX});
}catch(e){
	println("find({a:{$all:[]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	if(sel4.size()!=1){
		println("records are not expected!");
		throw "result4-error";
	}
	sel4.close();
}catch(e){
	if(e=="result4-error"){
		println("return result:" + sel4);
	}
	throw e;
}
println("find({a:{$all:[]}}) succ!");

//clean env
try{
	varCL.dropIndex(CSPREFIX_IDX);
}catch(e){
	println("drop index failed!rc="+e);
	throw e;
}
