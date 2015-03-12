/******************************************************************************
@Description : 1. Test db.createDomain(<name>,[option]), specify Autosplit.
               3. Test hash split and the split data correct.
               2. Test insert/update/find/remove operation.
@Modify list :
               2014-6-18  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   // Inspect the run mode
   runMode = inspectRunMode( db ) ;
   if( "standalone" == runMode )
      throw "RunMode_StandAlone" ;

   var domName = csName + "_DomAutoSplitBase" ;
   var replSize = new RSize( csName ).ReplSize( db ) ;

   // Drop CS in the begnning
   commDropCS( db, csName, clName, true,
               "clear collection space in the beginning" ) ;

   // Clear domain in the begnning
   clearDomain( db, domName ) ;

   // Get all data groups and create domain by specify AutoSplit
   try
   {
      println( "Begin to create domain" ) ;
      var group = new Array() ;
      group = getGroup( db ) ;
      //println( "Get Groups = " + group ) ;
      db.createDomain( domName, group, { "AutoSplit":true } ) ;
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

   // Inspect domain
   inspectDomain( db, domName ) ;

   // Create CS in domain and create collection
   try
   {
      commCreateCS( db, csName, false, "create CS specify domain",
                    { "Domain" : domName } ) ;
      commCreateCLByOption( db, csName, clName, { ShardingKey:{"No":-1},
                            ShardingType:"hash", Partition:1024 , ReplSize:replSize },
                            false, false, "create collection in domain" ) ;
   }
   catch ( e )
   {
         println( "Failed to create CS by specify domain, rc = " + e ) ;
         throw e ;
   }

   // Insert data
   insertData( db, csName, clName, 1000 ) ;
   println( "Success to insert 2000 records" ) ;

   // inspect the AutoSplit is take effect or not
   inspectAutoSplit( db, csName, clName, domName )
   println( "Success to inspect the autosplit parameter" ) ;

   // Query data
   queryData( db, csName, clName ) ;

   // Update data
   updateData( db, csName, clName ) ;

   // Remove data
   removeData( db, csName, clName ) ;

   // Drop domain in the end
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
