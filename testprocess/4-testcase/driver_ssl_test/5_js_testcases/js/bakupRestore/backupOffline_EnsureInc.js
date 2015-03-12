/*******************************************************************************
@Description : 1.Test backupOffline, specify [EnsureInc] to backup.
@Modify list :
               2014-3-16  Jianhui Xu  Init
               2014-6-20  xiaojun Hu  Changed
*******************************************************************************/

//globle variable
var backupName = CSPREFIX + "_bk" ;

// main entry
function main()
{
   var alreadStart = false ;
   // clean
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "drop cl in begin" ) ;
   bakRemoveBackups( db, CSPREFIX, true ) ;
   // create cl
   var varCL = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, false, "create cl in begin" ) ;
   // insert data
   bakInsertData( varCL ) ;
   // define backup object array
   var grpArray = commGetCSGroups( db, COMMCSNAME ) ;
   var bkObjArray = new Array() ;
   bkObjArray.push( {Name:backupName, Description:"default path full backup", MaxDataFileSize:32, GroupName:grpArray } ) ;
   bkObjArray.push( {Name:backupName, Description:"default path full backup", OverWrite:true, MaxDataFileSize:32, GroupName:grpArray } ) ;
   bkObjArray.push( {Name:backupName, Description:"default path inc backup", EnsureInc:true, MaxDataFileSize:32, GroupName:grpArray } ) ;

   // Run Mode
   var runMode = commIsStandalone( db ) ;
   println( "The runmode : [" + runMode + "]" ) ;
   // backup
   for ( var i = 0 ; i < bkObjArray.length; ++i )
   {
      try
      {
         bakBackup( db, bkObjArray[i] ) ;
         varCL.insert({times:i}) ;
      }
      catch ( e )
      {
         if ( e == -67 ) // already started
         {
            println( "Backup[" + i + "] already started, break " ) ;
            alreadStart = true ;
            break ;
         }
         println( "Backup[" + i + "] failed: " + e ) ;
         throw e ;
      }
   }
   if ( alreadStart == false )
   {
      // check backup
      var backups = commGetBackups( db, backupName, "", false, {EnsureInc:true} ) ;
      println( "Backups = " + backups ) ;
      if ( backups.length != 1 )
      {
         println( "Check ensure inc backup failed" ) ;
         comPrint( backups ) ;
         throw "check ensure inc backup failed" ;
      }
   }

   // remove backup
   bakRemoveBackups( db, backupName, alreadStart ) ;
   // clean - drop cl
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false, "drop cl in clean" ) ;
}

try
{
   main( db ) ;
   db.close() ;
}
catch( e )
{
   throw e ;
}
