/******************************************************************************
@Description : 1. _id sort
@Modify list :
               2015-01-16 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"_idsort1" ;

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
	var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:0});
}catch(e)
{
	println("can't create CS:" + CSPREFIX_CS + " rc="+e);
	throw e;
}
println("createCS " + CSPREFIX_CS + " finished");

//insert data
try{
   for(var i=0; i<rownums; i++){
   	varCL.insert({a:i,b:"abcdefghijklmnopq"+i,c:i+"abcdefghijklmnopqrstuvwxyz"});
  }
}catch(e){
	println("insert data failed!");
  throw e;
}
println("insert data finished!");

//query1
//select a,b,c from foo.bar order by _id
try{
	var sel = varCL.find(null,{a:null,b:'b',c:'c'}).sort({"_id":1});
	var flag=true;
	var i = 0;
	while(sel.next()){
		var ret = sel.current();
		i++;
		if(i>rownums){
			break;
		}
	}
	sel.close();
	if(flag && i!=rownums){
		flag = false;
		throw "query1-result-uncorrect";
	}
}catch(e){
	if(e!="query1-result-uncorrect"){
		println("'select a,b,c from foo.bar order by _id' failed! rc="+e);
		throw e;
	}else{
		println("'select a,b,c from foo.bar order by _id' verify record fail!");
  	throw e;
  }
}
println("'select a,b,c from foo.bar order by _id' finished!");

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
