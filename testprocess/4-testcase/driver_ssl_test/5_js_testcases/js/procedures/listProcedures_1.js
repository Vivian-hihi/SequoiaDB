//listProcedure() normal test case
//Test the cluster .it can't be stone

function main( db )
{
   var procName = new Array( CSPREFIX + "_sum", CSPREFIX + "_mult" ) ;
   fmpRemoveProcedures( procName, true ) ;
   println( "success to remove procedures in the beginning" ) ;
   var index = 0 ;
   try
   {
      eval( "db.createProcedure(function " + procName[index] +
            "(x,y){return x+y;})" ) ;
   }
   catch(e)
   {
      println("Fail to create procedures function sum"+e) ;
      throw e ;
   }
   println("create procedures function sum finish") ;

   ++index ;
   try
   {
      eval( "db.createProcedure(function " + procName[index] +
            "(m,n){return m*n;})" ) ;
   }
   catch(e)
   {
      println("Fail to create procedures function mult"+e) ;
      throw e ;
   }
   println("create procedures function mult finish") ;

   try
   {
     db.listProcedures() ;
   }
   catch(e)
   {
     println("Fail to list the procedures"+e) ;
     throw e ;
   }

   index = 0 ; // reset index
   try
   {
     var lis = db.listProcedures({name:procName[index]}) ;
   }
   catch(e)
   {
     println("Fail to list procedure sum"+e) ;
     throw e ;
   }
   ++index ;
   try
   {
     var list = db.listProcedures( procName[index] ) ;
   }
   catch(e)
   {
     println("Fail to list procedure mult"+e) ;
     throw e ;
   }

   //clean evioronment.
   fmpRemoveProcedures( procName, false ) ;
   println("remove functions finish") ;
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
