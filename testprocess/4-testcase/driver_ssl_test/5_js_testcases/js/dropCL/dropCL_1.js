// drop collection.
// normal case.
CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

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

try
{
   var varCL = varCS.createCL( CSPREFIX_CL,{Compressed:true} ) ;
}
catch ( e )
{
   println( "failed to create cl, rc= " + e );
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
   varCS.dropCL( CSPREFIX_CL ) ;
}
catch ( e )
{
   println( "failed to drop cl, rc= " + e );
   throw e ;
}


var found = true ;
try
{
   varCS.getCL( CSPREFIX_CL  ) ;
}
catch ( e )
{
   if ( -23 != e )
   {
      println( "unexpected err, rc= " + e );
      throw e ;
   }
   else
   {
      found = false ;
   }
}

if ( found )
{
   println( "still can get cl after drop") ;
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


