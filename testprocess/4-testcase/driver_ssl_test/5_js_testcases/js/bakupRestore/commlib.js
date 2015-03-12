/*******************************************************************************
*@Description : Backup and restore common functions
*@Modify list :
*               2014-3-16  Jianhui Xu  Init
*               2014-6-20  xiaojun Hu  Change
*******************************************************************************/

var hostName = COORDHOSTNAME ;
var coordPort = COORDSVCNAME ;
var csName = COMMCSNAME ;
var clName = COMMCLNAME ;

var db = new SecureSdb( hostName, coordPort ) ;

/* *****************************************************************************
@Description : insert data
@author: Jianhui Xu
@parameter:
   cl : collection object
@change :
          2014-6-20  xiaojun Hu
***************************************************************************** */
function bakInsertData( cl )
{
   try
   {
      cl.insert({no:1002,score:85,interest:["movie","photo"],major:"计算机软件与理论",dep:"计算机学院",info:{name:"Holiday",age:22,sex:"女"}});
      cl.insert({no:1005,score:70,major:"计算机工程",dep:"计算机学院",info:{name:"Jim",age:24,sex:"女"}});
      cl.insert({no:1000,score:80,interest:["basketball","football"],major:"计算机科学与技术",dep:"计算机学院",info:{name:"Tom",age:25,sex:"男"}});
      cl.insert({no:1001,score:82,major:"计算机科学与技术",dep:"计算机学院",info:{name:"Json",age:20,sex:"男"}});
      cl.insert({no:1003,score:90,major:"计算机软件与理论",dep:"计算机学院",info:{name:"Sam",age:30,sex:"男"}});
      cl.insert({no:1004,score:69,interest:["basketball","football","movie"],major:"计算机工程",dep:"计算机学院",info:{name:"Coll",age:26,sex:"男"}});
      cl.insert({no:1008,score:72,interest:["basketball","football","movie"],major:"物理学",dep:"物电学院",info:{name:"Appie",age:20,sex:"女"}});
      cl.insert({no:1006,score:84,interest:["basketball","football","movie","photo"],major:"物理学",dep:"物电学院",info:{name:"Lily",age:28,sex:"女"}});
      cl.insert({no:1007,score:73,interest:["basketball","football","photo"],major:"物理学",dep:"物电学院",info:{name:"Kiki",age:18,sex:"女"}});
      cl.insert({no:1009,score:80,major:"物理学",dep:"物电学院",info:{name:"Lucy",age:36,sex:"女"}});
      cl.insert({no:1011,score:75,major:"光学",dep:"物电学院",info:{name:"Jack",age:30,sex:"男"}});
      cl.insert({no:1010,score:93,major:"光学",dep:"物电学院",info:{name:"Coco",age:27,sex:"女"}});
      cl.insert({no:1012,score:78,interest:["basketball","movie"],major:"光学",dep:"物电学院",info:{name:"Mike",age:28,sex:"男"}});
      cl.insert({no:1014,score:74,interest:["football","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Iccra",age:19,sex:"男"}});
      cl.insert({no:1013,score:86,interest:["basketball","movie","photo"],major:"电学",dep:"物电学院",info:{name:"Jaden",age:20,sex:"男"}});
      cl.insert({no:1016,score:92,major:"电学",dep:"物电学院",info:{name:"Kate",age:20,sex:"男"}});
      cl.insert({no:1015,score:81,major:"电学",dep:"物电学院",info:{name:"Jay",age:15,sex:"男"}});
      // Inspect the number of insert data
      var cnt = 0 ;
      do
      {
         var count = cl.count() ;
         if( 17 == count )
            break ;
         ++cnt ;
      }while( cnt < 20 ) ;
      if( 17 != count )
      {
         println( "Wrong quantity of insert data, count = " + count ) ;
         throw "ErrInserNum" ;
      }
      println( "Success to insert data to Sdb." ) ;
   }
   catch ( e )
   {
      println("bakInsertData: Failed to insert Date, e=" + e ) ;
      throw e ;
   }
}

/* *****************************************************************************
@discretion: remove backups data
@author: Jianhui Xu
@parameter:
   filter : backup name filter, default is ""
   ignoreNotExist : true/false, default is true
   path : backup path, default is ""
   isSubDir : true/false, default is false
***************************************************************************** */
function bakRemoveBackups( db, filter, ignoreNotExist, path, isSubDir )
{
   if ( filter == undefined ) { filter = "" ; }
   if ( ignoreNotExist == undefined ) { ignoreNotExist = true ; }
   if ( path == undefined ) { path = ""; }
   if ( isSubDir == undefined ) { isSubDir = false ; }

   var backups = commGetBackups( db, filter, path, isSubDir ) ;
   for ( var i = 0 ; i < backups.length; ++i )
   {
      try
      {
         if ( path.length != 0 )
         {
            db.removeBackup( {Name:backups[i], Path:path} ) ;
         }
         else
         {
            db.removeBackup( {Name:backups[i]} ) ;
         }
      }
      catch( e )
      {
         // not exist
         if ( !ignoreNotExist || e != -241 )
         {
            println( "bakRemoveBackups: remove backup[" + backups[i] + "] failed: " + e ) ;
            throw e ;
         }
      }
   }
}

/* *****************************************************************************
@discretion: backup offline
@author: Jianhui Xu
@parameter:
   backupObj : backup object
***************************************************************************** */
function bakBackup( db, backupObj )
{
   if ( backupObj == undefined ) { backupObj = {} ; }

   if ( typeof( backupObj ) != "object" )
   {
      throw "bakBackup: backupObj is not object" ;
   }
   try
   {
      db.backupOffline( backupObj ) ;
   }
   catch( e )
   {
      println( "bakBackup: backup failed: " + e ) ;
      commPrint( backupObj ) ;
      throw e ;
   }
}

