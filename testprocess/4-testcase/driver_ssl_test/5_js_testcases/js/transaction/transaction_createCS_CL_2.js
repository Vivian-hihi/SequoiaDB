/* *****************************************************************************
@discretion: when transaction is on, create cs cl index, and the rollback 
@modify list:
   2014-4-3 YiBang Ruan  Init
***************************************************************************** */


var CONNECTNUM = 2 ;
try
{
   db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
}
catch( e )
{
   println( " new  Sdb failed : " + e ) ;
   throw e ;
}

println( "CONNECTNUM:" + CONNECTNUM ) ;

function createcs_cl__index_test( db )
{
   if( !commIsTransEnabled( db ) )
   {
      println( "transaction is disabled" ) ;
      return 0 ;
   }
   // create cs, cl and index
   try
   {
      db.transBegin() ;
   }
   catch( e )
   {
      println( " failed to begin transaction: " + e ) ;
      throw e ;
   }
   var cs = commCreateCS( db, COMMCSNAME, true ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, false, true ) ;
   commCreateIndex( cl, "ageIndex", {age:1}, true, true ) ;
   try
   {
      db.transRollback() ;
   }
   catch( e )
   {
      println( " failed to commit transaction: " + e ) ;
      throw e ;
   }
   
   // drop index
   try
   {
      db.transBegin() ;
   }
   catch( e )
   {
      println( " failed to begin transaction: " + e ) ;
      throw e ;
   }
   commDropIndex( cl, "ageIndex", false ) ;
   try
   {
      db.transRollback() ;
   }
   catch( e )
   {
      println( " failed to commit transaction: " + e ) ;
      throw e ;
   }
   
   
   // drop cl
   try
   {
      db.transBegin() ;
   }
   catch( e )
   {
      println( " failed to begin transaction: " + e ) ;
      throw e ;
   }
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false ) ;
   try
   {
      db.transRollback() ;
   }
   catch( e )
   {
      println( " failed to commit transaction: " + e ) ;
      throw e ;
   }
   
   // drop cs
   try
   {
      db.transBegin() ;
   }
   catch( e )
   {
      println( " failed to begin transaction: " + e ) ;
      throw e ;
   }
   commDropCS( db, COMMCSNAME, false ) ;
   try
   {
      db.transRollback() ;
   }
   catch( e )
   {
      println( " failed to commit transaction: " + e ) ;
      throw e ;
   }
}

function main()
{
   dbNew( db ) ;
   createcs_cl__index_test( db ) ;
   dbClose( db ) ;
}

try
{
   main() ;
}
catch( e )
{
   println( "transaction createCS and createCL test failed: " + e ) ;
   throw e ;
}

