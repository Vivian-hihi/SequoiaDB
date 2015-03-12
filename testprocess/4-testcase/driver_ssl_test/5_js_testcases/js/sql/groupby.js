//group by
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
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(price,country) values(1000,\"China\")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(price,country) values(1600,\"USA\")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(price,country) values(700,\"China\")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(price,country) values(300,\"China\")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(price,country) values(2000,\"English\")");
	db.execUpdate("insert into "+CSPREFIX_CS+"."+CSPREFIX_CL+"(price,country) values(100,\"USA\")");
}
catch(e)
{
	println("failed to insert 6 records,rc="+e);
	throw e ;
}
//select country,sum(price) from table group by country
var rc ;
try
{
	rc = db.exec("select country,sum(price) as sum_price from "+CSPREFIX_CS+"."+CSPREFIX_CL+" group by country");	
}
catch(e)
{
	println("err happend when use group by,rc="+e);
	throw e ;	
}
var size = 0 ;
while ( true )
{
   var i = rc.next() ;
   if ( !i )
      break ;
   else
      size++ ;
}

if ( 3 != size )
{
   println( " get records happend err" ) ;
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
