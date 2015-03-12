/******************************************************************************
@Description : 1. sort: table sort
@Modify list :
               2015-01-15 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;

rownums = 10000;

var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME );
try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

try{
	var varCS = db.createCS(CSPREFIX_CS);
	var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:0,Compressed:true});
}catch( e ){
   println("createCS or createCL fail");
   throw e ;	
}

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
//select a,b,c from foo.bar order by a
try{
	var sel = varCL.find(null,{a:null,b:'b',c:'c'}).sort({a:1});
	var flag=true;
	//expected result {a:0,...} {a:1,...} ... {a:9999,...}
	var i = 0;
	while(sel.next()){
		var ret = sel.current();
		if(ret.toObj()['a']!=i){
			flag = false;
			throw "query1-result-uncorrect";
		}
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
		println("'select a,b,c from foo.bar order by a' failed! rc="+e);
		throw e;
	}else{
		println("'select a,b,c from foo.bar order by a' verify record fail!");
  	throw e;
  }
}
println("'select a,b,c from foo.bar order by a' finished!");

//query2
//select b,a from foo.bar order by a desc
try{
	var sel = varCL.find(null,{b:'b',a:null}).sort({a:-1});
	var flag=true;
	//expected result {a:9999,...} {a:9998,...} ... {a:0,...}
	var i = rownums;
	while(sel.next()){
		var ret = sel.current();
		if(ret.toObj()['a']!=(i-1)){
			flag = false;
			throw "query2-result-uncorrect";
		}
		i--;
		if(i<0){
			break;
		}
	}
	sel.close();
	if(flag && i!=0){
		flag = false;
		throw "query2-result-uncorrect";
	}
}catch(e){
	if(e!="query2-result-uncorrect"){
		println("'select b,a from foo.bar order by a desc' failed! rc="+e);
		throw e;
	}else{
		println("'select b,a from foo.bar order by a desc' verify record fail!");
		println(sel);
  	throw e;
  }
}
println("'select b,a from foo.bar order by a desc' finished!");

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
