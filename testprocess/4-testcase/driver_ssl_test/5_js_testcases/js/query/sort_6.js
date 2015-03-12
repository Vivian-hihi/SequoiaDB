/******************************************************************************
@Description : 1. select a from foo.bar order by b;
@Modify list :
               2015-01-08 pusheng Ding  Init by jira-506
******************************************************************************/

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;

var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
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
	var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:0});
}catch( e ){
   println("createCS or createCL fail");
   throw e ;	
}

//insert data
try{
   varCL.insert({a:1,b:2});
   varCL.insert({a:2,b:1});
}catch(e){
	println("insert data failed!");
  throw e;
}
println("insert data finished!");

//query1
//select a from foo.bar order by b desc;
try{
	var sel = varCL.find(null,{a:0}).sort({b:-1});
	var flag=true;
	//expected result {a:1} {a:2}
	var i = 0;
	while(sel.next()){
		i++;
		var ret = sel.current();
		println(ret);
		if(ret.toObj()['a']!=i){
			flag = false;
			throw 'query1-result-uncorrect';
		}
		if(i>2){
			break;
		}
	}
	sel.close();
	if(flag && i!=2){
		flag = false;
		throw 'query1-result-uncorrect';
	}
}catch(e){
	if(e!='query1-result-uncorrect'){
		println("select a from foo.bar order by b desc; fail! rc="+e);
		throw e;
	}else{
		println("'select a from foo.bar order by b desc' verify record fail!");
		throw e;
	}
}
println("'select a from foo.bar order by b desc' finished!");

//query2
//select a from foo.bar order by b;
try{
	var sel = varCL.find(null,{a:0}).sort({b:1});
	var flag=true;
	//expected result {a:2} {a:1}
	var i = 2;
	while(sel.next()){
		var ret = sel.current();
		println(ret);
		if(ret.toObj()['a']!=i){
			flag = false;
			throw 'query2-result-uncorrect';
		}
		i--;
		if(i<0){
			break;
		}
	}
	sel.close();
	if(flag && i!=0){
		flag = false;
		throw 'query2-result-uncorrect';
	}
}catch(e){
	if(e!='query2-result-uncorrect'){
		println("select a from foo.bar order by b fail! rc="+e);
		throw e;
	}else{
		println("'select a from foo.bar order by b' verify record fail!\n");
		throw e;
	}
}
println("'select a from foo.bar order by b' finished!");

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
