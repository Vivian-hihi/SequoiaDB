/******************************************************************************
@Description : 1. Test db.createDomain(<name>,[option]), only have domain name.
               2. Test "NULL domain" cannot be created collection space.
@Modify list :
               2014-6-17  xiaojun Hu  Init
******************************************************************************/

function main( db )
{
   runMode = inspectRunMode( db ) ;
   var domName = csName + "_DomNameOnly"
   if( "standalone" == runMode )
   {
      // Create Domain in standalone
      println( "RunMode_StandAlone" ) ;
      try
      {
         db.createDomain( domName ) ;
         throw "ErrRun" ;
      }
      catch( e )
      {
         if( -159 != e )
         {
            println( "Error to run createDomain in Standalone, rc = " + e ) ;
            throw e ;
         }
      }

      // drop CS
      try
      {
         db.dropCS('standaloneCS') ;
      }
      catch( e )
      {
         if ( -34 != e )
         {
            println ( "Failed to drop CS, rc = " + e ) ;
            throw e ;
         }
      }

      // Create CS attach Domain in standalone
      try
      {
         db.createCS('standaloneCS',{'Domain': domName }) ;
         //throw "ErrRun" ;
      }
      catch( e )
      {
         if( -159 != e )
         {
            println( "Error to run createCS in Standalone, rc = " + e ) ;
            throw e ;
         }
      }
      throw "RunMode_StandAlone" ;
   }

   // Drop CS in the beginning
   commDropCS( db, csName, clName, true,
               "clear collection space in the beginning" ) ;

   // Drop domain in the beginning
   clearDomain( db, domName ) ;

   // Create domain only have domName [Testing Point]
   try
   {
      db.createDomain( domName ) ;
      println( "create domain = " + domName ) ;
   }
   catch( e )
   {
      println( "Failed to create domain, rc = " + e ) ;
      throw e ;
   }

   // Inspect domain
   inspectDomain( db, domName ) ;

   // Create collection space in this domain,
   // "NULL domain" cannot be created CS [Testing Point]
   try
   {
      commCreateCS( db, csName, false, "create CS specify domain",
                   { "Domain" : domName } ) ;
   }
   catch ( e )
   {
      if( -262 != e )  // domain does not have any groups at all
      {
         println( "Failed to create CS by specify domain, rc = " + e ) ;
         throw e ;
      }
   }

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
   switch ( e )
   {
      case "ErrRun" :
         println( "Shouldn't create CS or Domain in standalone" ) ;
         throw e ;
         break ;
      case "RunMode_StandAlone" :
         println( "Run Mode is : [ Standalone ] " ) ;
         break ;
      default :
         throw e
   }
}
