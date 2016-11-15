/*******************************************************************************
@Description : Test Backup SDB by use default argument.[db.backupOffline()]
@Modify list :
               2014-6-20  xiaojun Hu Init
*******************************************************************************/

function main( db )
{
   var backupName = "" ;   // in default backup, use time as backup name
   var alreadStart = false ;
   commDropCL( db, csName, clName, true, true, "Drop CL in the beginning" ) ;
   var cl = commCreateCL( db, csName, clName, -1, true, true, false,
                          "Create collection in the beginning" ) ;
   bakInsertData( cl ) ;
   bakRemoveBackups( db, CHANGEDPREFIX, true ) ;
   println( "Clear the backup in the beginning" ) ;
   // Backup don't have options [Test]. -67:Backup always begin
   try
   {
      bakBackup( db ) ;
   }
   catch( e )
   {
      if( -67 == e || -240 == e )   // backup already started
      {
         println( "default backup already started or exist, break " ) ;
         alreadStart = true ;
      }
      else
      {
         println( "default backup failed, rc = " + e ) ;
         throw e ;
      }
   }
   println( "default backup success" ) ;
   // check backup operation is success or not
   if( false == alreadStart )
   {
      var backups = commGetBackups( db ) ;
      if( 1 > backups.length )    // default backup will backup all groups
      {
         println( "check default backup failed, length" + backups.length ) ;
         commPrint( backups ) ;
         throw "check default backup failed" ;
      }
   }
   // Drop collection space
   bakRemoveBackups( db, backupName, alreadStart ) ;
   commDropCL( db, csName, clName, true, false, "Drop CL in the end" ) ;
}

try
{
   main( db ) ;
   db.close() ;
}
catch ( e )
{
   throw e ;
}
