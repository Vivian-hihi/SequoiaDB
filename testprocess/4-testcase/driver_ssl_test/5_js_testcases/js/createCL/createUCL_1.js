// create cl.
// normal case.

CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX ;  
var db = new SecureSdb(	COORDHOSTNAME, COORDSVCNAME  ) ;

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

for(var i = 0 ; i < (127 - CSPREFIX.length) ; ++i ){
	  CSPREFIX_CL = CSPREFIX_CL+"a";
	}

try
{
   var varCL = varCS.createCL( CSPREFIX_CL ) ;
}
catch ( e )
{
   println( "failed to create cl, rc= " + e );
   throw e ;
}

try
{
   var rc = varCS.getCL( CSPREFIX_CL ) ;
}
catch ( e )
{
   println( "failed to get cl, rc= " + e );
   throw e ;
}

try
{
   varCL.insert({a:1}) ;
}
catch ( e )
{
   println( "failed to insert record to cl, rc= " + e );
   throw e ;
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

