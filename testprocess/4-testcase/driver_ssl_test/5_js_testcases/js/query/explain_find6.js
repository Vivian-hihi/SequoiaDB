/******************************************************************************
@Description : 1. explain:{Run:true} or explain:{Run:false}
@Modify list :
               2014-11-11 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;

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
	var varCL = varCS.createCL(CSPREFIX_CL);
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

//find().explain()
//default is explain({Run:false})
println(CSPREFIX_CL + ".find().explain() begin...");
try{
	var expl = varCL.find().explain().toArray();
	var obj = eval('(' + expl + ')');
	if(obj['ReturnNum'] != 0){
		println("explain's ReturnNum is not 0 while Run is default value!");
		throw -1;
	}
	if(obj['ScanType'] != 'tbscan'){
		println("explain's ScanType is not tbscan while there is not index matched!");
		throw -1;
	}
	if(obj['UseExtSort'] != false){
		println("explain's UseExtSort is not false while there is not a sort sub!");
		throw -1;
	}
	if(obj['Name'] != (CSPREFIX_CS+'.'+CSPREFIX_CL)){
		println("explain's Name is error!");
		throw -1;
	}
	if(obj['IndexName'] != ''){
		println("explain's IndexName is not empty while query wouldn't use any index!");
		throw -1;
	}
	if(obj['IndexRead'] != 0 || obj['DataRead'] != 0 
		 || obj['UserCPU'] != 0 || obj['SysCPU'] != 0){
		println("explain's statistics are error!");
		throw -1;
	}
}catch(e)
{
	if(e!=-1)
	{
		println(CSPREFIX_CL + ".find().explain() fail! rc=" + e);
	}
	else
	{
		println("explain is not expected.Returned explain:");
		println(expl);
	}
	
	throw e;
}
println(CSPREFIX_CL + ".find().explain() finished");

//find().explain({Run:true})
println(CSPREFIX_CL + ".find().explain({Run:true}) begin...");
try{
	var expl = varCL.find().explain({Run:true}).toArray();
	var obj = eval('(' + expl + ')');
	if(obj['ReturnNum'] != 10000){
		println("explain's ReturnNum is not 10000 while Run is default value!");
		throw -1;
	}
	if(obj['ScanType'] != 'tbscan'){
		println("explain's ScanType is not tbscan while there is not index matched!");
		throw -1;
	}
	if(obj['UseExtSort'] != false){
		println("explain's UseExtSort is not false while there is not a sort sub!");
		throw -1;
	}
	if(obj['Name'] != (CSPREFIX_CS+'.'+CSPREFIX_CL)){
		println("explain's Name is error!");
		throw -1;
	}
	if(obj['IndexName'] != ''){
		println("explain's IndexName is not empty while query wouldn't use any index!");
		throw -1;
	}
	if(obj['IndexRead'] != 0 || obj['DataRead'] == 0){
		println("explain's statistics are error!");
		throw -1;
	}
}catch(e){
	if(e!=-1)
	{
		println(CSPREFIX_CL + ".find().explain({Run:true}) fail! rc=" + e);
	}
	else
	{
		println("explain is not expected.Returned explain:");
		println(expl);
	}
	
	throw e;
}
println(CSPREFIX_CL + ".find().explain({Run:true}) finished");

//find().explain({Run:false})
//default is explain({Run:false})
println(CSPREFIX_CL + ".find().explain({Run:false}) begin...");
try{
	var expl = varCL.find().explain({Run:false}).toArray();
	var obj = eval('(' + expl + ')');
	if(obj['ReturnNum'] != 0){
		println("explain's ReturnNum is not 0 while Run is default value!");
		throw -1;
	}
	if(obj['ScanType'] != 'tbscan'){
		println("explain's ScanType is not tbscan while there is not index matched!");
		throw -1;
	}
	if(obj['UseExtSort'] != false){
		println("explain's UseExtSort is not false while there is not a sort sub!");
		throw -1;
	}
	if(obj['Name'] != (CSPREFIX_CS+'.'+CSPREFIX_CL)){
		println("explain's Name is error!");
		throw -1;
	}
	if(obj['IndexName'] != ''){
		println("explain's IndexName is not empty while query wouldn't use any index!");
		throw -1;
	}
	if(obj['IndexRead'] != 0 || obj['DataRead'] != 0 
		 || obj['UserCPU'] != 0 || obj['SysCPU'] != 0){
		println("explain's statistics are error!");
		throw -1;
	}
}catch(e)
{
	if(e!=-1)
	{
		println(CSPREFIX_CL + ".find().explain({Run:false}) fail! rc=" + e);
	}
	else
	{
		println("explain is not expected.Returned explain:");
		println(expl);
	}
	
	throw e;
}
println(CSPREFIX_CL + ".find().explain({Run:false}) finished");

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
