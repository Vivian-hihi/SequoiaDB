//as  
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
//create CL as .another name
try
{
  //db.execUpdate("create collection "+CSPREFIX_CS+"."+CSPREFIX_CL);	
  var claSize = new RSize( CSPREFIX_CS );
  var varCS = db.getCS(CSPREFIX_CS);
  var varCL = varCS.createCL(CSPREFIX_CL,{ReplSize:claSize.ReplSize()});
}
catch( e )
{
	println ("failed to create CL when use as , rc = "+e);
	throw e ;
}
//insert records
try 
{
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Tomi\",10)");	
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Miko\",20)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Mike\",20)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Tina\",30)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Poo\",40)");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+" (name,age) values (\"Pete\",40)");
}
catch(e)
{
	println("failed to insert 6 records,rc="+e);
	throw e ;
}
//select * from table where name is null
var rc ;
try
{
	rc = db.exec("select bar.name as nn from "+CSPREFIX_CS+"."+CSPREFIX_CL+" as bar where bar.age=10");	
}
catch(e)
{
	println("err happend when use as,rc="+e);
	throw e ;	
}
var bool = false ;
var o = eval('('+rc[0]+')');
var name=o["nn"];
if(name == "Tomi"){
	bool = true ;
	}

if(!bool)
{
	println("failed to execte as");
	throw -2;
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
