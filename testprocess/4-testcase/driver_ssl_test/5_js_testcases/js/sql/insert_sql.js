//sql insert normal case
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ; 
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch ( e )
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

try
{
	db.execUpdate("create collectionspace "+CSPREFIX_CS);
}
catch(e)
{
  println("failed to create CS , rc =" +e);
  throw e ;	
}
try 
{
	//db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);
	var claSize = new RSize( CSPREFIX_CS );
  var varCS = db.getCS(CSPREFIX_CS);
  var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
}
catch(e)
{
	println("failed to create collection CL , rc = "+e);
	throw e ;
}
//insert {name:"Mike",age:20}
try
{
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Mike\",20)");
}
catch(e)
{
	println("failed to insert record,rc="+e);
	throw e ;
}

var rc ;
try
{
   rc = db.exec("select name,age from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where name=\"Mike\" and age=20");
}
catch ( e )
{
   println( "failed to read record name-mike,age=20, rc= " + e ) ;
   throw e ;
}

if ( 1 != rc.size() )
{
   println( " get more than one record.." ) ;
   throw -1 ;
}

//clear all the records
try
{
  db.execUpdate("delete from "+CSPREFIX_CS+"."+CSPREFIX_CL);	
}
catch(e)
{
	println("failed to delete records , rc ="+e);
	throw e ;
}

//insert record {_id:"123",a:10}
try
{
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (_id,a) values (\"123\",10)");
}
catch(e)
{
	println("failed to insert record _id=123,a=10,rc1="+e);
	throw e ;
}

var rc ;
try
{
   rc = db.exec("select _id,a from "+CSPREFIX_CS+"."+CSPREFIX_CL+" where _id=\"123\" and a=10");
}
catch ( e )
{
   println( "failed to read record _id=123,a=10, rc= " + e ) ;
   throw e ;
}
if ( 1 != rc.size() )
{
   println( " get more than one record...." ) ;
   throw -1 ;
}

//clear all the records
try
{
  db.execUpdate("delete from "+CSPREFIX_CS+"."+CSPREFIX_CL);	
}
catch(e)
{
	println("failed to delete records , rc ="+e);
	throw e ;
}

//insert record ,the field contain &,%,$.etc

var res = false ;
var aa = Array("=",">","<","*",";",",","\"");

for(var i = 0 ; i < aa.length ; ++i ){
   	try{
   	    var age = aa[i]+"age";	
   	    db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"("+age+")"+" values(25)");
   	}catch( e ){
   		if ( e == -195 )
   			res = true ;
   	}

if(!res)
{
	println("over...") ;
	throw -1 ;
	
}
res = false ; 
}
//the field values is contain %,*,$.etc
var res = false ;
var aa = Array(";",":","\'","\"","{","}","[","]",",","<",">","?","/","|","\\","+","=","-","_","~","`","!","@","#","$","%","^","&","*");
for(var i = 0 ; i < aa.length ; ++i ){
   	try{
   	    var value = aa[i]+"chen";	
   	    db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name) values"+"("+value+")");
   	}catch( e ){
   		if ( e == -195 )
   			res = true ;
   	}
}
if(!res)
{
	throw -1 ;
}

//clear environment
try
{
   db.execUpdate( "drop collectionspace "+CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}
