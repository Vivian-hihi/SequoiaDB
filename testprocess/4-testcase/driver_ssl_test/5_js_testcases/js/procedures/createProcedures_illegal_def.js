/******************************************************************************
@Description : Test the procedures illegal testing.[Don't have function name]
               [Same function name to create:-38][createProcedure function's
               arguement is wrong]
@Modify list :
               2014-7-30  xiaojun Hu  Init
******************************************************************************/
function main( db )
{
   // Clean procedure in the beginning
   var preProcName = new Array( CSPREFIX + "_sum") ;
   fmpRemoveProcedures( preProcName, true ) ;
   println( "clean procedures in the beginning" ) ;

   // Specified the param named string [T-Point]
   try
   {
      db.createProcedure("function sum(x,y){return x+y ;}");
   }
   catch(e)
   {
      if ( "Error: Sdb.createProcedure(): wrong arguments" != e )
      {
         throw "WrongArguments" ;
      }
   }

   // Overload, duplicate key exist:-38 [T-Point]
   var index = 0 ;
   try
   {
      eval( "db.createProcedure(function " + preProcName[index] +
            "(x,y){return x+y;})" );
   }
   catch(e)
   {
      println("failed to create  function sum(x,y),e = "+e);
      throw e ;
   }
   try
   {
      eval( "db.createProcedure(function " + preProcName[index] +
            "(x,y,z){return x+y+z;})" ) ;
   }
   catch( e )
   {
      if( e != -38 )
      {
         println( "Excute to create function sum(x,y,z) error" );
         throw "" ;
      }
   }

   // Don't have function name in Procedure [T-Point]
   try
   {
      db.createProcedure( function(x,y){ return x+y ; } ) ;
   }
   catch( e )
   {
      if( -6 != e )
      {
         println( "Failed to test no function name in procedure, rc = " + e  ) ;
         throw e ;
      }
   }

   // Clean procedure in the end
   var finalProcName = new Array( CSPREFIX + "_sum" ) ;
   fmpRemoveProcedures( finalProcName, false ) ;
   println( "Clean procedures in the end." ) ;
}

// Running
try
{
   var mode = commIsStandalone( db ) ;
   if( false == mode )
      main( db ) ;
   else
      println( "Run Mode : StandAlone " ) ;
}
catch( e )
{
   throw e ;
}
