/******************************************************************************
@Description : 1. explain verify
@Modify list :
               2014-11-11 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
CSPREFIX_IDX = CSPREFIX+"idx" ;

if( false == commIsStandalone( db ) )
{
   // set session get data from master
   db.setSessionAttr( {"PreferedInstance":"M"} ) ;
}

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

//create CL
try{
	var varCL = varCS.createCL(CSPREFIX_CL, { ReplSize:0});
}catch(e)
{
	println("can't create normal-CL:" + CSPREFIX_CL + " rc="+e);
	throw e;
}
println("create " + CSPREFIX_CL + " finished");

//insert data
try{
	for(var i=0;i<10000;i++){varCL.insert({a:i-10000,b:i,c:"abcdefghijkl"+i});}
}catch(e)
{
	println("insert-data to " + CSPREFIX_CL + " fail! rc="+e);
	throw e;
}
println("insert data finished");


//find().sort({a:1}).explain()
println("\n***********************************************************");
println(CSPREFIX_CL + ".find().sort({a:1}).explain() begin...");
try{
	var expl = varCL.find().sort({a:1}).explain().toArray();
	var obj = eval('(' + expl + ')');
	if(obj['ReturnNum'] != 0){
		println("expect:");
		println("\t'ReturnNum':0");
		println("return:");
		println("\t'ReturnNum':" + obj['ReturnNum']);
		throw "explain1-error";
	}
	if(obj['ScanType'] != 'tbscan'){
		println("expect:");
		println("\t'ScanType':'tbscan'");
		println("return:");
		println("\t'ScanType':'" + obj['ScanType'] + "'");
		throw "explain1-error";
	}
	if(obj['UseExtSort'] != true){
		println("expect:");
		println("\t'UseExtSort':true");
		println("return:");
		println("\t'UseExtSort':" + obj['UseExtSort']);
		throw "explain1-error";
	}
	if(obj['Name'] != (CSPREFIX_CS+'.'+CSPREFIX_CL)){
		println("expect:");
		println("\t'Name':'" + (CSPREFIX_CS+'.'+CSPREFIX_CL) +"'");
		println("return:");
		println("\t'Name':'" + obj['Name'] +"'");
		throw "explain1-error";
	}
	if(obj['IndexName'] != ''){
		println("expect:");
		println("\t'IndexName':''");
		println("return:");
		println("\t'IndexName':'" + obj['IndexName'] +"'");
		throw "explain1-error";
	}
	if(obj['IndexRead'] != 0 || obj['DataRead'] != 0 ){
		println("explain's statistics are error!");
		println("expect:");
		println("\t'IndexRead':0");
		println("\t'DataRead':0");
		println("return:");
		println("\t'IndexRead':" + obj['IndexRead']);
		println("\t'DataRead':" + obj['DataRead']);
		throw "explain1-error";
	}
}catch(e)
{
	if(e!="explain1-error")
	{
		println(CSPREFIX_CL + ".find().sort({a:1}).explain() fail! rc=" + e);
	}
	else
	{
		println("explain is not expected.Returned explain:");
		println(expl);
	}
	
	throw e;
}
println(CSPREFIX_CL + ".find().sort({a:1}).explain() finished");

//find().sort({a:1}).skip(5000).explain({Run:true})
println("\n***********************************************************");
println(CSPREFIX_CL + ".find().sort({a:1}).skip(5000).explain({Run:true}) begin...");
try{
	var expl = varCL.find().sort({a:1}).skip(5000).explain({Run:true}).toArray();
	var obj = eval('(' + expl + ')');
	if(obj['ReturnNum'] != 5000 && commIsStandalone( db ) ){
		println("standalone mode:");
		println("expect:");
		println("\t'ReturnNum':5000");
		println("return:");
		println("\t'ReturnNum':" + obj['ReturnNum']);
		throw "explain2-error";
	}
	if(obj['ReturnNum'] != 10000 && false == commIsStandalone( db ) ){
		println("cluster mode:");
		println("expect:");
		println("\t'ReturnNum':10000");
		println("return:");
		println("\t'ReturnNum':" + obj['ReturnNum']);
		throw "explain2-error";
	}
	if(obj['ScanType'] != 'tbscan'){
		println("expect:");
		println("\t'ScanType':'tbscan'");
		println("return:");
		println("\t'ScanType':'" + obj['ScanType'] + "'");
		throw "explain2-error";
	}
	if(obj['UseExtSort'] != true){
		println("expect:");
		println("\t'UseExtSort':true");
		println("return:");
		println("\t'UseExtSort':" + obj['UseExtSort']);
		throw "explain2-error";
	}
	if(obj['Name'] != (CSPREFIX_CS+'.'+CSPREFIX_CL)){
		println("expect:");
		println("\t'Name':'" + (CSPREFIX_CS+'.'+CSPREFIX_CL) +"'");
		println("return:");
		println("\t'Name':'" + obj['Name'] +"'");
		throw "explain2-error";
	}
	if(obj['IndexName'] != ''){
		println("expect:");
		println("\t'IndexName':''");
		println("return:");
		println("\t'IndexName':'" + obj['IndexName'] +"'");
		throw "explain2-error";
	}
	if(obj['IndexRead'] != 0 || obj['DataRead'] == 0){
		println("explain's statistics are error!");
		println("expect:");
		println("\t'IndexRead':0");
		println("\t'DataRead':0");
		println("return:");
		println("\t'IndexRead':" + obj['IndexRead']);
		println("\t'DataRead':" + obj['DataRead']);
		throw "explain2-error";
	}
}catch(e)
{
	if(e!="explain2-error")
	{
		println(CSPREFIX_CL + ".find().sort({a:1}).skip(5000).explain({Run:true}) fail! rc=" + e);
	}
	else
	{
		println("explain is not expected.Returned explain:");
		println(expl);
	}
	
	throw e;
}
println(CSPREFIX_CL + ".find().sort({a:1}).skip(5000).explain({Run:true}) finished");

//create index
try{
	varCL.createIndex(CSPREFIX_IDX,{a:1});
}catch(e)
{
	println("cat't create index " + CSPREFIX_IDX + " rc=" + e);
	throw e;
}
println("create index finished!");

//sleep(5);

//find.hint
println("\n***********************************************************");
println(CSPREFIX_CL + ".find().sort({a:1}).hint(index).explain() begin...");
try{
	var expl = varCL.find().sort({a:1}).hint({"":CSPREFIX_IDX}).explain().toArray();
	var obj = eval('(' + expl + ')');
	if(obj['ReturnNum'] != 0){
		println("expect:");
		println("\t'ReturnNum':0");
		println("return:");
		println("\t'ReturnNum':" + obj['ReturnNum']);
		throw "explain3-error";
	}
	if(obj['ScanType'] != 'ixscan'){
		println("expect:");
		println("\t'ScanType':'ixscan'");
		println("return:");
		println("\t'ScanType':'" + obj['ScanType'] + "'");
		throw "explain3-error";
	}
	if(obj['UseExtSort'] != false){
		pprintln("expect:");
		println("\t'UseExtSort':false");
		println("return:");
		println("\t'UseExtSort':" + obj['UseExtSort']);
		throw "explain3-error";
	}
	if(obj['Name'] != (CSPREFIX_CS+'.'+CSPREFIX_CL)){
		println("expect:");
		println("\t'Name':'" + (CSPREFIX_CS+'.'+CSPREFIX_CL) +"'");
		println("return:");
		println("\t'Name':'" + obj['Name'] +"'");
		throw "explain3-error";
	}
	if(obj['IndexName'] != CSPREFIX_IDX){
		println("expect:");
		println("\t'IndexName':'" + (CSPREFIX_IDX) +"'");
		println("return:");
		println("\t'IndexName':'" + obj['Name'] +"'");
		throw "explain3-error";
	}
	if(obj['IndexRead'] != 0 || obj['DataRead'] != 0){
		println("explain's statistics are error!");
		throw "explain3-error";
	}
}catch(e)
{
	if(e!="explain3-error")
	{
		println(CSPREFIX_CL + ".find().sort({a:1}).hint(index).explain({Run:true}) fail! rc=" + e);
	}
	else
	{
		println("explain is not expected.Returned explain:");
		println(expl);
	}
	
	throw e;
}
println(CSPREFIX_CL + ".find().sort({a:1}).hint(index).explain() finished");

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
