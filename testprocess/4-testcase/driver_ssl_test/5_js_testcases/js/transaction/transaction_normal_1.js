/* *****************************************************************************
@discretion: sdb transaction normal operation test
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
function test_normal_one( db )
{
   if( !commIsTransEnabled( db ) )
   {
      println( "transaction is disabled" ) ;
      return 0 ;
   }
   try
   {
      println( "transBegin") ;
      db.transBegin() ;
   }
   catch( e )
   {
      println( "failed to transBegin") ;
      throw e ;
   }
   try
   {
      println( "transCommit" ) ;
      db.transCommit() ;
   }
   catch( e )
   {
      println( "failed to transCommit") ;
      throw e ;
   }

   try
   {
      println( "transBegin") ;
      db.transBegin() ;
   }
   catch( e )
   {
      println( "failed to transBegin") ;
      throw e ;
   }
   try
   {
      println( "transRollback" ) ;
      db.transRollback() ;
   }
   catch( e )
   {
      println( "failed to transRollback") ;
      throw e ;
   }
}
function main( db )
{
   dbNew( db ) ;
   test_normal_one( db ) ;
   dbClose( db ) ;
}

try
{
   main( db ) ;
}
catch( e )
{
   println( "transaction normal operation test failed: " + e ) ;
   throw e ;
}

