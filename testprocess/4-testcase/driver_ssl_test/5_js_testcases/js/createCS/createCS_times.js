// create cs.
// normal case.

CSPREFIX_CS = CSPREFIX+"foo" ;

CSPREFIX_CL = CSPREFIX+"bar" ;
function main( db ){


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
   db.createCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to create cs, rc= " + e );
   throw e ;
}

try
{
   var rc = db.getCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to get cs, rc= " + e );
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

}

for(var i = 0; i != 200 ; ++i){
   var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME);
   main( db );	
}