/******************************************************************************
@Description : 1. Test db.createDomain(<name>,[option]), have name and Groups.
               2. Test create CS on the domain.
               3. Test insert/update/find/remove operation.
@Modify list :
               2014-6-18  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   // Inspect the run mode
   runMode = inspectRunMode( db ) ;
   if( "standalone" == runMode )
      throw "RunMode_StandAlone" ;

   var domName = csName + "_DomNameAndGroup"

   // Drop CS
   commDropCS( db, csName, clName, true,
               "clear collection space in the beginning" ) ;

   // Drop domain
   clearDomain( db, domName ) ;

   // Get one group and create domain [Testing Point]
   try
   {
      var group = new Array() ;
      group = getGroup( db ) ;
      //println( "Groups" + group[1] ) ;
      db.createDomain( domName, [ group[0] ] ) ;
   }
   catch ( e )
   {
      if( -159 != e )
      {
         println( "Failed to create domain, rc = " + e ) ;
         throw e ;
      }
      else
         println( "The mode is standalong, not group" ) ;
   }

   // Inspect domain that was created
   inspectDomain( db, domName ) ;

   // Create CS in domain and create collection [Testing Point]
   try
   {
      commCreateCS( db, csName, false, "create CS specify domain",
                    { "Domain" : domName } ) ;
      commCreateCL( db, csName, clName, -1, true, false, false,
                    "create collection in domain" ) ;
   }
   catch ( e )
   {
         println( "Failed to create CS by specify domain, rc = " + e ) ;
         throw e ;
   }

   // Insert data
   insertData( db, csName, clName, 1000 )
   println( "Success to insert 2000 records in database" ) ;

   // Query data
   queryData( db, csName, clName )

   // Update data
   updateData( db, csName, clName )

   // Remove data
   removeData( db, csName, clName )

   // Drop domain int the end
   clearDomain( db, domName ) ;
   println( "Success to clean domain : [" + domName + "] in the end" ) ;
}

try
{
   main( db ) ;
   db.close() ;
}
catch ( e )
{
   if( "RunMode_StandAlone" != e )
      throw e ;
   else
      println( "WARNNING! Run Mode is : [ standalone ]" ) ;
}
