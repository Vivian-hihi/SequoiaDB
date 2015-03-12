/******************************************************************************
@Description : 1. sort: a.a1 sort
@Modify list :
               2015-01-15 pusheng Ding  Init
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
CLINDEX1 = CSPREFIX + "IND1" ;

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
		varCL.insert({a:{a1:i,a2:rownums-i},b:"abcdefghijklmnopq"+i});
	}
}catch( e ){
	println("insert data failed!");
  throw e;
}
println("insert data finished!");

//query1
//select a,b from foo.bar order by a.a1 desc
try{
	var sel = varCL.find(null,{a:null,b:'b'}).sort({"a.a1":-1});
	var flag=true;
	var i = rownums;
	while(sel.next()){
		i--;
		var ret = sel.current();
		if(ret.toObj()['b']!=("abcdefghijklmnopq"+i)
		   || ret.toObj()['a']['a1']!=i
		   || ret.toObj()['a']['a2']!=(rownums-i)){
			flag = false;
			throw "query1-result-uncorrect";
		}
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
		println("'select a,b from foo.bar order by a.a1 desc' failed! rc="+e);
		throw e;
	}else{
		println("'select a,b from foo.bar order by a.a1 desc' verify record fail!");
  	throw e;
  }
}
println("'select a,b from foo.bar order by a.a1 desc' finished!");

//create index
try{
	varCL.createIndex(CLINDEX1,{a:1});
}catch( e ){
   println("create indexes fail");
   throw e ;	
}
println("create indexes finished!");

//query2
//select a,b from foo.bar order by a.a1
try{
	var sel = varCL.find(null,{a:null,b:'b'}).sort({"a.a1":1}).hint({"":CLINDEX1});
	var flag=true;
	var i = 0;
	while(sel.next()){
		var ret = sel.current();
		if(ret.toObj()['b']!=("abcdefghijklmnopq"+i)
		   || ret.toObj()['a']['a1']!=i
		   || ret.toObj()['a']['a2']!=(rownums-i)){
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
		println("'select a,b from foo.bar order by a.a1' with index failed! rc="+e);
		throw e;
	}else{
		println("'select a,b from foo.bar order by a.a1' with index verify record fail!");
  	throw e;
  }
}
println("'select a,b from foo.bar order by a.a1' with index finished!");

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
