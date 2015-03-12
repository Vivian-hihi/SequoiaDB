/* *****************************************************************************
@discretion: sdb transaction normal operation loop test
@modify list:
   2014-4-1 YiBang Ruan  Init
***************************************************************************** */

// LOOPTIME is input parameter
if( typeof( LOOPTIME ) == "undefined" )
{
   LOOPTIME = 10000 ;
}
// judge LOOPTIME whether is a numric type
if( !isNaN( LOOPTIME ) )
{
   if( 0 >= LOOPTIME )
   {
      LOOPTIME = 10000 ;
   }
}
else
{
   LOOPTIME = 10000 ;
}

// CONNECTNUM is input parameter
if( typeof( CONNECTNUM ) == "undefined" )
{
   CONNECTNUM = 1 ;
}
// judge CONNECTNUM whether is a numric type
if( !isNaN( CONNECTNUM ) )
{
   if( 0 >= CONNECTNUM )
   {
      CONNECTNUM = 1 ;
   }
}
else
{
   CONNECTNUM = 1 ;
}

if( typeof( opAfterTransBegin ) == "undefined" )
{
   opAfterTransBegin = 'none' ;
}
opAfterTransBegin.toLowerCase() ;
if( 'none' != opAfterTransBegin && 'commit' != opAfterTransBegin && 'rollback' != opAfterTransBegin )
{
   opAfterTransBegin = 'none' ;
}

println( "LOOPTIME:" + LOOPTIME ) ;
println( "CONNECTNUM:" + CONNECTNUM ) ;
println( "opAfterTransBegin:" + opAfterTransBegin ) ;

var db = new Array() ;

function test_loop( db )
{
   if( !commIsTransEnabled( db[0] ) )
   {
      println( "transaction is disabled" ) ;
      return 0 ;
   }
   for( i = 0; i < CONNECTNUM; ++i )
   {
      for( j = 0; j < LOOPTIME; ++j )
      {
         try
         {
            // println( "transBegin") ;
            db[i].transBegin() ;
         }
         catch( e )
         {
            println( "failed to transBegin") ;
            throw e ;
         }
         if( 'commit' == opAfterTransBegin )
         {
            try
            {
               // println( "transCommit" ) ;
               db[i].transCommit() ;
            }
            catch( e )
            {
               println( "failed to transCommit") ;
               throw e ;
            }
         }
         else if( 'rollback' == opAfterTransBegin )
         {
            try
            {
               // println( "transRollback" ) ;
               db[i].transRollback() ;
            }
            catch( e )
            {
               println( "failed to transRollback") ;
               throw e ;
            }
         }
      }
      println( 'i=' + i + ' , ' + 'j=' + j ) ;
   }
}

function main()
{
   dbArrayNew( db ) ;
   test_loop( db ) ;
   dbArrayClose( db ) ;
}

try
{
   main() ;
}
catch( e )
{
   println( "transaction performance test failed: " + e ) ;
   throw e ;
}

