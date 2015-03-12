/*******************************************************************************
@Discription : After insert 1000 record, and the normal connection read
               the max record
@Modify list :
               2014-4-3 YiBang Ruan  Init
*******************************************************************************/

var CONNECTNUM = 2 ;
var db = new Array() ;

println( "CONNECTNUM:" + CONNECTNUM ) ;

function dbArrayNew( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i] = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
      }
   }
   catch( e )
   {
      println( " new the " + i + "st Sdb failed. " ) ;
      throw e ;
   }
}

function dbArrayClose( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i].close() ;
      }
   }
   catch( e )
   {
      println( " close the" + i + "st Sdb failed. " ) ;
      throw e ;
   }
}

function dbReadDifferent( db )
{
   if( !commIsTransEnabled( db[0] ) )
   {
      println( "transaction is disabled" ) ;
      return 0 ;
   }
   var cs1 = commCreateCS( db[0], COMMCSNAME, true ) ;
   var cs2 = commCreateCS( db[1], COMMCSNAME, true ) ;
   var cl1 = commCreateCL( db[0], COMMCSNAME, COMMCLNAME, 0, true, false, true ) ;
   var cl2 = commCreateCL( db[1], COMMCSNAME, COMMCLNAME, 0, true, false, true ) ;
   try
   {
      db[0].transBegin() ;
   }
   catch( e )
   {
      println( " failed to begin transaction: " + e ) ;
      throw e ;
   }
   for( i = 0; i < 10000; ++i )
   {
      cl1.insert( { "transTest":i } ) ;
   }
   if( cl2.find( {"transTest":9999} ).count() != 1 )
   {
      println( " session which is not transactional can't read the max record " ) ;
      throw "ErrReadRecord" ;
   }
   try
   {
      db[0].transCommit() ;
   }
   catch( e )
   {
      println( " failed to commit transaction: " + e ) ;
      throw e ;
   }
}

function main()
{
   dbArrayNew( db ) ;
   dbReadDifferent( db ) ;
   dbArrayClose( db ) ;
}

try
{
   main() ;
}
catch( e )
{
   println( "transaction read test failed: " + e ) ;
   throw e ;
}

