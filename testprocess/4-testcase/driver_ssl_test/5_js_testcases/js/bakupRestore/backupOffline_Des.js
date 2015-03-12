/*******************************************************************************
@Description : 1.Test backupOffline, specify["GroupName","Path","Description"]
@Modify list :
               2014-6-20  xiaojun Hu  Init
*******************************************************************************/

function main( db )
{
   var alreadStart = false ;
   var path = "" ;
   // Clear the collection space in the beginning
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "clean collection space in the beginning" ) ;
   var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, -1, true, true, false,
                          "create collection in the beginning" ) ;
   // Insert data to SDB
   bakInsertData( cl ) ;
   // Clear backup in the begnning
   bakRemoveBackups( db, CSPREFIX, true ) ;
   println( "Clear the backup in the beginning" ) ;
   // Backup specify the GroupName/Path/Description
   if( false == commIsStandalone( db ) )
   {
      println( "run mode is group" ) ;
      var groups = new Array() ;
      groups = commGetGroups( db ) ;
      for( var i = 0 ; i < groups.length ; ++i )
      {
         for( var j = 0 ; j < groups[i][0].Length ; ++j )
         {
            if( (j+1) == groups[i][0].PrimaryPos )
               path = groups[i][j+1].dbpath ;
         }
         if( undefined == path )
            throw "failed to get primary node path" ;
         var bakName = COMMCSNAME + "bak" + i ;
         var bakString = "{\"GroupName\":\"" + groups[i][0].GroupName +
                         "\",\"Path\":\"" + path +
                         "\",\"Description\":\"backup description\",\"Name\":\""
                         + bakName + "\",\"EnsureInc\":false,\"OverWrite\":false}" ;
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
         bakRemoveBackups( db, bakName, path, alreadStart ) ;
      }
      println( "description backup success" ) ;
   }
   else   // run mode standalone
   {
      println( "run mode is standalone" ) ;
      var aloneBakName = COMMCSNAME + "bakstandalone" ;
      var bakString = "{\"Description\":\"backup description\",\"Name\":\""
                      + aloneBakName +
                      "\",\"EnsureInc\":false,\"OverWrite\":false}" ;
      var backup = eval("(" + bakString + ")") ;
      // Begin to backup
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
         var backups = commGetBackups( db, aloneBakName, path, alreadStart ) ;
         println( "backup file = " + backups ) ;
         if( 1 != backups.length )
         {
            println( "check description backup failed" ) ;
            commPrint( backups ) ;
            throw "check description backup failed" ;
         }
      }
      // Inspect the backup
      println( "description backup success in standalone" ) ;
      bakRemoveBackups( db, aloneBakName, alreadStart ) ;
   }

   // Clear backup in the end
   println( "Clear backup Over in the end" ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, false, "Drop CL in the " ) ;
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
