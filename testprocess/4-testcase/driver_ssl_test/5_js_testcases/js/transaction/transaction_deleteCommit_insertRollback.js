/* *****************************************************************************
@discretion: step one :delete data and commit transaction . step two: insert data and rollback transaction 
@modify list:
   2014-4-1 YiBang Ruan  Init
***************************************************************************** */


// loop is input parameter
if( typeof( loop ) == "undefined" )
{
   loop = 100 ;
}

// judge amount whether is a data type
if( !isNaN( loop ) )
{
   if( 0 >= loop )
   {
      loop = 100 ;
   }
}
else
{
   loop = 100 ;
}

// amount is input parameter
if( typeof( amount ) == "undefined" )
{
   amount = 10 ;
}

// judge amount whether is a numric type
if( !isNaN( amount ) )
{
   if( 0 >= amount )
   {
      amount = 10 ;
   }
}
else
{
   amount = 10 ;
}

println( "loop:" + loop ) ;
println( "amount:" + amount ) ;


try
{
   var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;
}
catch( e )
{
   println( " new SecureSdb failed. " ) ;
   throw e ;
}

function deleteCommit_insertRollbackTest( db )
{
   if( !commIsTransEnabled( db ) )
   {
      println( "transaction is disabled" ) ;
      return 0 ;
   }
   var cs = commCreateCS( db, COMMCSNAME, true ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, false, true ) ;
   
   println( "insert 1000 record " ) ;
   for( j = 0 ; j < 1000 ; ++j )
   {
      try
      {
         cl.insert( { transaction: j } ) ;
      }
      catch( e )
      {
         println( " failed to insert data : { transaction: " + j + " }" ) ;
         throw e ;
      }
   }
   
   // delete and then commit
   println( "delete and then commit" ) ;
   i = 0;
   while( i < loop )
   {
      try
      {
         db.transBegin() ;
      }
      catch( e )
      {
         println( "failed to transBegin") ;
         throw e ;
      }
      for( j = i*amount ; j < ( i + 1 ) * amount ; ++j )
      {
         try
         {
            cl.remove( { transaction: j } ) ;
         }
         catch( e )
         {
            println( " failed to remove data : { transaction: " + j + " }" ) ;
            println( " loop : " + i ) ;
            throw e ;
         }
      }
      try
      {
         db.transCommit() ;
      }
      catch( e )
      {
         println( "failed to transCommit") ;
         throw e ;
      }
      i = i + 1 ;
   }
   
   println( "insert and then rollback" ) ;
   // insert and then rollback
   i = 0;
   while( i < loop )
   {
      try
      {
         db.transBegin() ;
      }
      catch( e )
      {
         println( "failed to transBegin") ;
         throw e ;
      }
      for( j = 0 ; j < amount ; ++j )
      {
         try
         {
            cl.insert( { transaction: j } ) ;
         }
         catch( e )
         {
            println( " failed to insert data : { transaction: " + j + " }" ) ;
            println( " loop : " + i ) ;
            throw e ;
         }
      }
      try
      {
         db.transRollback() ;
      }
      catch( e )
      {
         println( "failed to transRollback") ;
         throw e ;
      }
      i = i + 1 ;
   }

}
function main( db )
{
   deleteCommit_insertRollbackTest( db ) ;
}

try
{
   main( db ) ;
}
catch( e )
{
   println( "transaction deleteCommit_insertRollbackTest test failed: " + e ) ;
   throw e ;
}

