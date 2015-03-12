//eval procedures normal test case

function main( db )
{
   if ( commIsStandalone( db ) )
   {
      println( "In standalone, not run test" ) ;
      return ;
   }

   // clean
   fmpCleanProcedures( db, CSPREFIX ) ;

   // create procedures
   var nameArray = new Array( CSPREFIX+"_sum", CSPREFIX+"_mult" ) ;
   var index = 0 ;

   try
   {
      eval( "db.createProcedure( function " + nameArray[index] + "( x, y ){ return x + y ; } )" ) ;
      ++index ;
      eval( "db.createProcedure( function " + nameArray[index] + "( m, n ){ return m * n ;} )" ) ;
   }
   catch(e)
   {
      println("Failed to create procedure function[" + nameArray[index] + "], e: " +e ) ;
      throw e ;
   }

   // create and eval
   try
   {
      var res = db.eval( 'function sing(x){return x;} sing(5); ' ) ;
      if ( res != 5 )
      {
         println( "eval sing result[" + res + "] is error" ) ;
         throw "eval sing result error" ;
      }
   }
   catch(e)
   {
      println("Failed to create and eval function sing, e=" + e ) ;
      throw e ;
   }

   // eval created funcs
   var resArray = new Array( 7, 10 ) ;
   index = 0 ;
   try
   {
      var res = 0 ;
      while( index < nameArray.length )
      {
         res = db.eval( nameArray[index] + "(2,5)" ) ;
         if ( res != resArray[index] )
         {
            println( "eval function[" + nameArray[index] + "] result[" + res + "] is error" ) ;
            break ;
         }
         ++index ;
      }
   }
   catch(e)
   {
      println( "Failed to eval function[" + nameArray[index] + "] failed, e=" + e ) ;
      throw e ;
   }

   if ( index != nameArray.length )
   {
      throw "eval function result error" ;
   }

   // remove function
   fmpRemoveProcedures( nameArray, false ) ;

}

// main entry
try
{
   main( db ) ;
   db.close() ;
}
catch( e )
{
   throw e ;
}
