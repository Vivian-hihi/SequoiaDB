/* *****************************************************************************
@discretion: step one :insert data and commit transaction . step two: delete data and commit transaction 
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

function insertCommit_deleteCommitTest( db )
{
   if( !commIsTransEnabled( db ) )
   {
      println( "transaction is disabled" ) ;
      return 0 ;
   }
   var cs = commCreateCS( db, COMMCSNAME, true ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, false, true ) ;
   
   // insert and then commit
   println( "insert and then commit" ) ;
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
            cl.insert( { transaction: i } ) ;
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
         db.transCommit() ;
      }
      catch( e )
      {
         println( "failed to transCommit") ;
         throw e ;
      }
      i = i + 1 ;
   }
   
   println( "delete and then commit" ) ;
   // delete and then commit
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
            cl.remove() ;
         }
         catch( e )
         {
            println( " failed to remove() " ) ;
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

}
function main( db )
{
   insertCommit_deleteCommitTest( db ) ;
}

try
{
   main( db ) ;
}
catch( e )
{
   println( "transaction insertCommit_deleteCommitTest test failed: " + e ) ;
   throw e ;
}

