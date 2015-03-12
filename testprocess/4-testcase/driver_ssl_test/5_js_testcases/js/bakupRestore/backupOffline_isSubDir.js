/*******************************************************************************
@Description : Test db.backupOffline().Specify [IsSubDir].{IsSubDir:false}
@Modify list :
               2014-6-20  xiaojun Hu Init
*******************************************************************************/
function main( db )
{
   var alreadStart = false ;
   var path = "" ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "Drop CL in the beginning" ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, false,
                          "Create collection in the beginning" ) ;
   bakInsertData( cl ) ;
   bakRemoveBackups( db, CSPREFIX, true ) ;
   println( "Clear the backup in the beginning" ) ;
   // Backup Offline specify the [groups]
   var groups = new Array() ;
   if( false == commIsStandalone( db ) )
   {
      groups = commGetGroups( db ) ;
      for( var i = 0 ; i < groups.length ; ++i )
      {
         var bakName = COMMCLNAME + "_bak_" + i ;
         println( "Gruop : " + groups[i][0].GroupName ) ;
         var bakString = "{\"GroupName\":[\"" + groups[i][0].GroupName +
                         "\"],\"Name\":\""+ bakName + "\",\"IsSubDir\":true}" ;
         var backup = eval("(" + bakString + ")") ;
         commPrint( backup ) ;
         try
         {
            bakBackup( db, backup ) ;
         }
         catch( e )
         {
            if( -67 == e || -240 == e )
            {
               println( "backup description is already started" ) ;
               alreadStart = true ;
            }
            else
            {
               println( "failed to backup by specify description, rc = " + e ) ;
               throw e ;
            }
         }
         if( false == alreadStart )
         {
            var backups = commGetBackups( db, bakName, path, alreadStart ) ;
            println( "backup file = " + backups ) ;
            if( 1 != backups.length )
            {
               println( "check description backup failed" ) ;
               commPrint( backups ) ;
               throw "check description backup failed" ;
            }
         }
         // backup and check over, then remove backup
         bakRemoveBackups( db, bakName, alreadStart, path ) ;
      }
      println( "groupID backup success" ) ;
   }
   else
   {
      var bakName = COMMCSNAME + "bakStandalone" ;
      var bakString = "{\"Name\":\"" + bakName + "\"," +
                      "\"IsSubDir\":true}" ;
      var backup = eval("(" + bakString + ")") ;
         try
         {
            bakBackup( db, backup ) ;
         }
         catch( e )
         {
            if( -67 == e || -240 == e )
            {
               println( "backup description is already started" ) ;
               alreadStart = true ;
            }
            else
            {
               println( "failed to backup by specify description, rc = " + e ) ;
               throw e ;
            }
         }
         if( false == alreadStart )
         {
            var backups = commGetBackups( db, bakName, path, alreadStart ) ;
            println( "backup file = " + backups ) ;
            if( 1 != backups.length )
            {
               println( "check description backup failed" ) ;
               commPrint( backups ) ;
               throw "check description backup failed" ;
            }
         }
         // backup and check over, then remove backup
         bakRemoveBackups( db, bakName, alreadStart, path ) ;
   }
   println( "Clear backup Over in the end" ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, false, "Drop CL in the end" ) ;
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
