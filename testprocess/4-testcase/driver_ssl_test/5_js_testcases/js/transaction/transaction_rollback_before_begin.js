/* *****************************************************************************
@discretion: sdb transaction commit before begin test
@modify list:
   2014-4-1 YiBang Ruan  Init
***************************************************************************** */
try
{
   db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
}
catch( e )
{
   println( " new  Sdb failed : " + e ) ;
   throw e ;
}
println( "COORDHOSTNAME:" + COORDHOSTNAME ) ;
println( "COORDSVCNAME:" + COORDSVCNAME ) ;
println( "CSPREFIX:" + CSPREFIX ) ;

function test_commit( db )
{
   if( !commIsTransEnabled( db ) )
   {
      println( "transaction is disabled" ) ;
      return 0 ;
   }
   try
   {
      println( "transRollback" ) ;
      db.transRollback() ;
   }
   catch( e )
   {
      if( -196 != e )
      {
         println( "failed to transRollback") ;
         throw e ;
      }
   }
}
function main( db )
{
   dbNew( db ) ;
   test_commit( db ) ;
   dbClose( db ) ;
}

try
{
   main( db ) ;
}
catch( e )
{
   println( "transaction commit before begin test failed: " + e ) ;
   throw e ;
}

