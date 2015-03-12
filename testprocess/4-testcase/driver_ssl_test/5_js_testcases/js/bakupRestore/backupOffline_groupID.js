/*******************************************************************************
@Description : Test db.backupOffline().Specify [GroupID].{GroupID:[1000,1001]}
@Modify list :
               2014-6-20  xiaojun Hu Init
*******************************************************************************/
function main( db )
{
   var alreadStart = false ;
   var path = "" ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true, "Drop CL in the beginning" ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, false,
                          "Create collection in the beginning" ) ;
   bakInsertData( cl ) ;
   bakRemoveBackups( db, CSPREFIX, true ) ;
   println( "Clear the backup in the beginning" ) ;
   // Backup Offline specify the [GroupID]
   var groupID = new Array() ;
   groupID = commGetGroups( db ) ;
   for( var i = 0 ; i < groupID.length ; ++i )
   {
      var bakName = COMMCLNAME + "_bak_" + i ;
      var bakString = "{\"GroupID\":[" + groupID[i][0].GroupID + "],\"Name\":\"" +
                      bakName + "\"}" ;
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
      // Inspect the backup Group,Rg = Group
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
      bakRemoveBackups( db, bakName, alreadStart ) ;
   }
   println( "Clear backup Over in the end" ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, false, "Drop CL in the end" ) ;
}

try
{
   if( false == commIsStandalone( db ) )
   {
      main( db ) ;
      db.close() ;
   }
   else
      println( "This is standalone mode" ) ;
}
catch( e )
{
   throw e ;
}
