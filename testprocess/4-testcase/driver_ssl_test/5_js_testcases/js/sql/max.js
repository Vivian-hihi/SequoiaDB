//function max
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
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
//create CL.
try
{
  //db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
  var claSize = new RSize( CSPREFIX_CS );
  var varCS = db.getCS(CSPREFIX_CS);
  var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
}
catch( e )
{
	println ("failed to create CL , rc2 = "+e);
	throw e ;
}
//insert records
try 
{
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Tom\",20)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Tomi\",20)");	
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Miko\",30)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Mike\",30)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Tina\",40)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Timiko\",40)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Poo\",40)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Pete\",40)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(name,age) values(\"Jhon\",70)");
}
catch(e)
{
	println("failed to insert 9 records,rc="+e);
	throw e ;
}
//select max(age) from table 
var rc ;
try
{
	rc = db.exec("select max(age) as max_age from "+CSPREFIX_CS+"."+CSPREFIX_CL);	
}
catch(e)
{
	println("err happend when use max,rc="+e);
	throw e ;	
}

var max_age = 0 ;
var obj =eval('('+rc[0]+')') ; 
max_age = obj["max_age"] ; 
if(70!= max_age)
{
	throw -1 ;
	}
//clear enviroment

try
{
	db.execUpdate("drop collectionspace "+CSPREFIX_CS);	
}
catch(e)
{
	println("err happend when clear cs,rc="+e);
	throw e ;	
}
