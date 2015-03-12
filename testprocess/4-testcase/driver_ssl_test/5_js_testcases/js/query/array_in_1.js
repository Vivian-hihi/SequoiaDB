/* *****************************************************************************
@discretion: array: $in without index
@modify list:
   						2014-02-02 Pusheng Ding  Init
***************************************************************************** */

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;

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
	varCL.insert({a:1,b:[],c:"empty array"});
	varCL.insert({a:2,b:[1,2,3],c:"123"});
	varCL.insert({a:3,b:[3,2],c:"32"});
}catch(e)
{
	println("insert data failed! rc="+e);
	throw e;
}
println("insert data finished!");

//find({b:{$in:[1,2,3]}})
//result:
//		{a:2,b:[1,2,3],c:"123"}
//		{a:3,b:[3,2],c:"32"}
println("********************************************");
try{
	var sel1 = varCL.find({b:{$in:[1,2,3]}});
}catch(e){
	println("find({b:{$in:[1,2,3]}}) failed! rc=" + e);
	throw e;
}
//verify result
try{
	if(sel1.size()!=2) {
		throw "result1-error";
	}
	sel1.close();
}catch(e){
	if(e=="result1-error"){
		println("return result:" + sel1);
	}
	throw e;
}
println("find({b:{$in:[1,2,3]}}) succ!");

//find({b:{$in:[1]}})
//result:
//		{a:2,b:[1,2,3],c:"123"}
println("********************************************");
try{
	var sel2 = varCL.find({b:{$in:[1]}});
}catch(e){
	println("find({b:{$in:[1]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	sel2.next();
	if(sel2.current().toObj()['a']!=2){
		throw "result2-error";
	}
	sel2.close();
}catch(e){
	if(e=="result2-error"){
		println("return result:" + sel2);
	}
	throw e;
}
println("find({b:{$in:[1]}}) succ!");

//find({b:{$in:[4,5,6,7,8]}})
//result:
//			empty result set
println("********************************************");
try{
	var sel3 = varCL.find({b:{$in:[4,5,6,7,8]}});
}catch(e){
	println("find({b:{$in:[4,5,6,7,8]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	if(sel3.size()!=0){
		throw "result3-error";
	}
	sel3.close();
}catch(e){
	if(e=="result3-error"){
		println("return result:" + sel3);
	}
	throw e;
}
println("find({b:{$in:[4,5,6,7,8]}}) succ!");

//find({b:{$in:[]}})
//result:
//		{a:1,b:[],c:"empty array"}
println("********************************************");
try{
	var sel4 = varCL.find({b:{$in:[]}});
}catch(e){
	println("find({b:{$in:[]}}) failed! rc=" + e);
	throw e;
}
//verify data
try{
	sel4.next();
	if(sel4.current().toObj()['a']!=1){
		throw "result4-error";
	}
	sel4.close();
}catch(e){
	if(e=="result4-error"){
		println("return result:" + sel4);
	}
}
println("find({b:{$in:[]}}) succ!");
