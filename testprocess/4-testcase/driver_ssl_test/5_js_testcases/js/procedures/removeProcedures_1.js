//removeProcedure() normal test case1
function main( db )
{
   var procName = new Array( CSPREFIX + "_sum", CSPREFIX + "_mult" ) ;
   //create procedures function sun();
   var index = 0 ;
   try
   {
      eval( "db.createProcedure( function " + procName[index] +
                                 "(x,y){return x+y;} )" ) ;
   }
   catch (e)
   {
      println( "failed to create procedure sum, rc = " + e ) ;
      throw e ;
   }
   ++index ;
   try
   {
      eval( "db.createProcedure(function " + procName[index] +
                                "(m,n){return m*n;})" ) ;
   }
   catch(e)
   {
      println( "failed to create procedure multi, rc = " + e ) ;
      throw e ;
   }
   println( "success to create procedure" ) ;

   //remove procedures function sum(),para is "sum" [TEST POINT]
   index = 0 ;
   try
   {
      db.removeProcedure( procName[index] ) ;
   }
   catch(e)
   {
      println( "failed to remove, rc = " + e ) ;
      throw e ;
   }
   ++index ;
   try
   {
      db.removeProcedure( procName[index] ) ;
   }
   catch(e)
   {
      println("failed to remove (), rc = " + e ) ;
      throw e ;
   }
   println( "success to remove procedure" ) ;
}

try
{
   if( false == commIsStandalone( db ) )
      main( db ) ;
   else
      println( "run mode : standalone" ) ;
}
catch( e )
{
   throw e ;
}

