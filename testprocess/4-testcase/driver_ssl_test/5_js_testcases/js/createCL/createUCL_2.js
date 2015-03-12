// create cl.
// normal case.

CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;  


var db = new SecureSdb(COORDHOSTNAME, COORDSVCNAME ) ;

try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch (e)
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

try
{
   var varCS = db.createCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to create cs, rc= " + e );
   throw e ;
}

for(var i = 0 ; i < (128 - CSPREFIX.length) ; ++i ){
	  CSPREFIX_CL = CSPREFIX_CL+"a";
	}
var j = true ; 
try
{
   var varCL = varCS.createCL( CSPREFIX_CL ) ;
}
catch ( e )
{
   j = false ; 
}
if( j ){
   throw -1 ; 
}


try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch (e)
{
   println( "unexpected err happened when clear cs:" + e ) ;
   throw e ;
}

