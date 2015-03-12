/******************************************************************************
@Description : 1. string sort
@Modify list :
               2015-01-16 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"stringsort1" ;
CLINDEX1 = CSPREFIX + "IND1" ;

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
	var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:0});
}catch(e)
{
	println("can't create CS:" + CSPREFIX_CS + " rc="+e);
	throw e;
}
println("createCS " + CSPREFIX_CS + " finished");

//insert data
try{
	varCL.insert({a:"book",b:2, c:"abcd"});
	varCL.insert({a:"agree",b:1, c:"efghi"});
	varCL.insert({a:"dog",b:4,c:"xyz"});
	varCL.insert({a:"cat",b:3, c:"jklmn"});
}catch(e)
{
	println("insert-data into varCL fail! rc="+e);
}
println("insert-data into varCL succ!");

//query1
//select a,b from foo.bar order by a
try{
	var sel = varCL.find(null,{a:"default",b:0}).sort({a:1});
	var flag=true;
	//expected result {a:"agree",b:1} {a:"book",b:2} {a:"cat",b:3} {a:"dog",b:4}
	var i = 0;
	var rownum = 4;
	while(sel.next()){
		i++;
		var ret = sel.current();
		if(ret.toObj()['b']!=i){
			flag = false;
			throw "query1-result-uncorrect";
		}
		if(i>rownum){
			break;
		}
	}
	sel.close();
	if(flag && i!=rownum){
		flag = false;
		throw "query1-result-uncorrect";
	}
}catch(e){
	if(e!="query1-result-uncorrect"){
		println("'select a,b from foo.bar order by a' fail! rc="+e);
		throw e;
	}else{
		println("'select a,b from foo.bar order by a' verify record fail!");
		throw e;
	}
}
println("'select a,b from foo.bar order by a' finished!");

//create index
try{
	varCL.createIndex(CLINDEX1,{a:1});
}catch( e ){
   println("create indexes fail");
   throw e ;	
}
println("create indexes finished!");

//query2
//select a,b,c from foo.bar order by a
try{
	var sel = varCL.find(null,{a:"default",b:0,c:"default"}).sort({a:1}).hint({"":CLINDEX1});
	var flag=true;
	//expected result {a:"agree",b:1} {a:"book",b:2} {a:"cat",b:3} {a:"dog",b:4}
	var i = 0;
	var rownum = 4;
	while(sel.next()){
		i++;
		var ret = sel.current();
		if(ret.toObj()['b']!=i){
			flag = false;
			throw "query2-result-uncorrect";
		}
		if(i>rownum){
			break;
		}
	}
	sel.close();
	if(flag && i!=rownum){
		flag = false;
		throw "query2-result-uncorrect";
	}
}catch(e){
	if(e!="query2-result-uncorrect"){
		println("'select a,b,c from foo.bar order by a' fail! rc="+e);
		throw e;
	}else{
		println("'select a,b,c from foo.bar order by a' verify record fail!");
		throw e;
	}
}
println("'select a,b,c from foo.bar order by a' finished!");

try
{
	 varCS.dropCL(CSPREFIX_CL);
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}