//CSPREFIX = "suse";
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;//student table
CSPREFIX_CL1 = CSPREFIX+"bar1" ;  //grade table

var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
//clear environment
try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch (e)
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}
//create CS
try 
{
	db.execUpdate("create collectionspace "+CSPREFIX_CS) ;
}
catch( e )
{
	println ("failed to create CS , rc1 = "+e);
	throw e ;
}
//create student table.
try
{
  //db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
  var claSize = new RSize( CSPREFIX_CS );
  var varCS = db.getCS(CSPREFIX_CS);
  var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
  var varCL1 = varCS.createCL(CSPREFIX_CL1,{ReplSize:claSize.ReplSize()})
}
catch( e )
{
	println ("failed to create cl , rc = "+e);
	throw e ;
}

try{
   for(var i = 1;i<=100;i++){
      db.execUpdate("insert into " +CSPREFIX_CS +"." +CSPREFIX_CL + "(name,id) values(\"A"+i+"\","+i+")" )	;
   }
   for(var i = 1 ; i<= 50;i++){
     db.execUpdate( "insert into " +CSPREFIX_CS +"." +CSPREFIX_CL1 + "(user,id) values(\"B"+i+"\","+i+")" );	
   }
}catch( e ){
   println("insert some date failed!!!")
   throw e;	
}

try{
	 println("select A.name , B.user , A.id from "+CSPREFIX_CS+"."+CSPREFIX_CL+" as A left outer join "+CSPREFIX_CS+"."+CSPREFIX_CL1+ " as B on A.id=B.id order by A.id asc");
   var res = db.exec("select A.name , B.user , A.id from "+CSPREFIX_CS+"."+CSPREFIX_CL+" as A left outer join "+CSPREFIX_CS+"."+CSPREFIX_CL1+ " as B on A.id=B.id order by A.id asc /*+use_hash()*/");	
}catch( e ){
	println("get the result by left outer join failed order by asc!!!");
	throw e ;
}
var number = 0;
while( res.next() ){number++;}
if( 100 != number ){
   println("get the result is error , result number should be 100 , but the number = " + number + " order by asc");
   throw -1;	
}

try{
	 println("select A.name , B.user , B.id from "+CSPREFIX_CS+"."+CSPREFIX_CL+" as A left outer join "+CSPREFIX_CS+"."+CSPREFIX_CL1+ " as B on A.id=B.id order by B.id desc");
   var res = db.exec("select A.name , B.user , B.id from "+CSPREFIX_CS+"."+CSPREFIX_CL+" as A left outer join "+CSPREFIX_CS+"."+CSPREFIX_CL1+ " as B on A.id=B.id order by A.id desc /*+use_hash()*/");	
}catch( e ){
	println("get the result by left outer join failed order by desc!!!");
	throw e ;
}
var number = 0;
while( res.next() ){number++;}
if( 100 != number ){
   println("get the result is error , result number should be 50 , but the number = " + number + " order by desc");
   throw -2;	
}

try{
	 println("select A.name , B.user , B.id from "+CSPREFIX_CS+"."+CSPREFIX_CL+" as A left outer join "+CSPREFIX_CS+"."+CSPREFIX_CL1+ " as B on A.id=B.id order by B.id desc");
   var res = db.exec("select A.name , B.user , B.id from "+CSPREFIX_CS+"."+CSPREFIX_CL+" as A left outer join "+CSPREFIX_CS+"."+CSPREFIX_CL1+ " as B on A.id=B.id order by A.id desc /*+use_hash()*/");	
}catch( e ){
	println("get the result by left outer join failed order by desc!!!");
	throw e ;
}

var value = res.current().toObj();
if( value["name"] != "A100" ){
	 println( "get the result is error by value" );
   throw -3;	
}	
try
{
	db.execUpdate("drop collectionspace "+CSPREFIX_CS);
}
catch(e)
{
  println("failed to drop cs;rc="+e);
  throw e ;	
}
