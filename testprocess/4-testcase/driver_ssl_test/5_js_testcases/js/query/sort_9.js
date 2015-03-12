/******************************************************************************
@Description : 1. sub-CL sort
@Modify list :
               2015-01-16 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"mainsort1" ;
SUBCL1NAME = CSPREFIX_CL+"_sub1";
SUBCL2NAME = CSPREFIX_CL+"_sub2";
SUBCL3NAME = CSPREFIX_CL+"_sub3";
CLINDEX1 = CSPREFIX + "IND1" ;

rownums = 10000;

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

//create main-cl
try{
	var mainCL = varCS.createCL(CSPREFIX_CL,{ShardingKey:{a:1} , ReplSize:0,IsMainCL:true});
}catch(e)
{
	println("can't create main-CL:" + CSPREFIX_CL + " rc="+e);
	throw e;
}
println("createCL " + CSPREFIX_CL + " finished");

//create sub-cl
try{
	var subCL1 = varCS.createCL(SUBCL1NAME,{ShardingKey:{b:1},ShardingType:"hash",Partition:4096});
	var subCL2 = varCS.createCL(SUBCL2NAME,{ReplSize:2});
	var subCL3 = varCS.createCL(SUBCL3NAME,{Compressed:true});
}catch(e)
{
	println("can't create sub-CL");
	throw e;
}

//attach sub-cl
try{
	mainCL.attachCL(CSPREFIX_CS + "." + SUBCL1NAME,{LowBound:{a:-10000},UpBound:{a:0}});
	mainCL.attachCL(CSPREFIX_CS + "." + SUBCL2NAME,{LowBound:{a:0},UpBound:{a:6000}});
	mainCL.attachCL(CSPREFIX_CS + "." + SUBCL3NAME,{LowBound:{a:6000},UpBound:{a:20000}});
}catch(e)
{
	if(!isStandalone)
	{
		println("attach sub-CL fail!");
		throw e;
	}
}
println("attach sub-CL finish!");

//insert data
try{
	for(var i=0;i<rownums;i++){mainCL.insert({a:rownums-i,b:i,c:"abcdefghijkl"+i});}
}catch(e)
{
	println("insert-data into mainCL fail! rc="+e);
}
println("insert-data into mainCL succ!");

//query1
//select a,b,c from foo.bar order by a desc
try{
	var sel = mainCL.find(null,{a:0,b:0,c:'c'}).sort({a:-1});
	var flag=true;
	//expected result {a:rownums,...} {a:rownums-1,...} ... {a:1,...}
	var i = rownums;
	while(sel.next()){
		var ret = sel.current();
		if(ret.toObj()['a']!=i){
			flag = false;
			throw "query1-result-uncorrect";
		}
		i--;
		if(i<0){
			break;
		}
	}
	sel.close();
	if(flag && i!=0){
		flag = false;
		throw "query1-result-uncorrect";
	}
}catch(e){
	if(e!="query1-result-uncorrect"){
		println("'select a,b,c from foo.bar order by a desc' fail! rc="+e);
		throw e;
	}else{
		println("'select a,b,c from foo.bar order by a desc' verify record fail!");
		throw e;
	}
}
println("'select a,b,c from foo.bar order by a desc' finished!");

//create index
try{
	mainCL.createIndex(CLINDEX1,{b:1});
}catch( e ){
   println("create indexes fail");
   throw e ;	
}
println("create indexes finished!");

//query2
//select b from foo.bar order by b
try{
	var sel = mainCL.find(null,{b:0}).sort({b:1}).hint({"":CLINDEX1});
	var flag=true;
	//expected result {b:0} {b:1} ... {b:rownums-1}
	var i = 0;
	while(sel.next()){
		var ret = sel.current();
		if(ret.toObj()['b']!=i){
			flag = false;
			throw "query2-result-uncorrect";
		}
		i++;
		if(i>rownums){
			break;
		}
	}
	sel.close();
	if(flag && i!=rownums){
		flag = false;
		throw "query2-result-uncorrect";
	}
}catch(e){
	if(e!="query2-result-uncorrect"){
		println("'select b from foo.bar order by b' fail! rc="+e);
		throw e;
	}else{
		println("'select b from foo.bar order by b' verify record fail!");
		throw e;
	}
}
println("'select b from foo.bar order by b' finished!");

//attach sub-cl
try{
	 mainCL.detachCL(CSPREFIX_CS + "." + SUBCL1NAME);
	 mainCL.detachCL(CSPREFIX_CS + "." + SUBCL2NAME);
	 mainCL.detachCL(CSPREFIX_CS + "." + SUBCL3NAME);
}catch(e)
{
	if(!isStandalone)
	{
		println("detach sub-CL fail!");
		throw e;
	}
}
println("detach sub-CL finish!");

try
{
	 varCS.dropCL(SUBCL1NAME);
	 varCS.dropCL(SUBCL2NAME);
	 varCS.dropCL(SUBCL3NAME);
	 varCS.dropCL(CSPREFIX_CL);
   db.dropCS(CSPREFIX_CS) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}
