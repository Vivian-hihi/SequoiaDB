/* *****************************************************************************
@discretion: Procedure common functions
@modify list:
   2014-3-14 Jianhui Xu  Init
***************************************************************************** */

var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

/* *****************************************************************************
@discretion: clean procedure
@author: Jianhui Xu
@parameter:
   filter : procedure name filter
***************************************************************************** */
function fmpCleanProcedures( db, filter )
{
   if ( filter == undefined ) { filter = ""; }

   var procedures = commGetProcedures( db, filter ) ;
   for ( var i = 0 ; i < procedures.length; ++i )
   {
      try
      {
         db.removeProcedure( procedures[i] ) ;
      }
      catch( e )
      {
         println( "fmpCleanProcedures remove procedure " + procedures[i] + " failed: " + e ) ;
         if ( e != -233 )
         {
            throw e ;
         }
      }
   }
}

/* *****************************************************************************
@discretion: clean procedure
@author: Jianhui Xu
@parameter:
   nameArray : name array
   ignoreNotExist : true/false, default is false
***************************************************************************** */
function fmpRemoveProcedures( nameArray, ignoreNotExist )
{
   if ( ignoreNotExist == undefined ) { ignoreNotExist = false ; }

   for ( var i = 0 ; i < nameArray.length; ++i )
   {
      try
      {
         db.removeProcedure( nameArray[i] ) ;
      }
      catch(e)
      {
         if ( !ignoreNotExist || e != -233 )
         {
            println("Failed to remove function[" + nameArray[i] + "],e=" + e ) ;
            throw e ;
         }
      }
   }
}
