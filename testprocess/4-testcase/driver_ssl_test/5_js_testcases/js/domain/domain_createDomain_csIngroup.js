/******************************************************************************
@Description : 1. Test db.createDomain(<name>,[option]).
               2. Test db.createCS() specify domain, CS locate in domain
                  group is correct or not .
@Modify list :
               2014-6-17  xiaojun Hu  Init
******************************************************************************/
function main( db )
{
   var CSNAME = COMMCLNAME + "_csInGroup" ;
   var domName = COMMCLNAME + "_createCSInGroup"
   var groups = commGetGroups( db ) ;
   for( var i = 0 ; i < groups.length ; ++i )
   {
      var csRg = groups[i][0].GroupName ;
      commDropCS( db, CSNAME, true, "clear environmen in the beginning" ) ;
      try
      {
         db.dropDomain( domName ) ;
      }
      catch( e )
      {
         if( -214 != e && -34 != e )
         {
            println( "failed to drop domain in beginnig, rc = " + e ) ;
            throw e ;
         }
      }
      try
      {
         db.createDomain( domName, [ csRg ] ) ;
         var varCS = db.createCS( CSNAME, { "Domain": domName } ) ;
         // need to create cl, because when cs has no cl, the cs don't create in data group
         varCS.createCL( COMMCLNAME ) ;
      }
      catch( e )
      {
         println( "failed to create domain and create CS, rc = " + e ) ;
         throw e ;
      }
      // Inspect the CS is located in
      var csGroups = commGetCSGroups( db, CSNAME ) ;
      if( csGroups.length > 1 || csGroups != csRg )
      {
         println( db.listDomains({ "Name" : domName }) ) ;
         println( db.snapshot( 5, { "Name" : CSNAME } )) ;
         println( "expect cs group: " + csRg +
                  ", actural cs group: " + csGroups ) ;
         throw "error, create CS located in wrong group" ;
      }
   }
   // Clear the envioronment
   commDropCS( db, CSNAME, false, "clear environmen in the end" ) ;
   try
   {
      db.dropDomain( domName ) ;
   }
   catch( e )
   {
      println( "failed to drop domain in the end, rc = " + e ) ;
      throw e ;
   }
}
// Run Main
try
{
   if( false == commIsStandalone( db ) )
      main( db ) ;
   else
      println( "run mode is : standalone" ) ;
}
catch( e )
{
   throw e ;
}
