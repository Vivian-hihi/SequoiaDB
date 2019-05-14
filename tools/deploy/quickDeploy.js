/****************************************************************
@decription:   Deploy SequoiaDB, MySQL, PostgreSQL

@input:        sdb:   Boolean
               mysql: Boolean
               pg:    Boolean
               cm:    Number, default: 11790

               eg: bin/sdb -f quickDeploy.js -e 'var sdb=true; var mysql=true; var cm=11790'

@author:       Ting YU 2019-04-12
****************************************************************/

var USER_SET_DEPLOY = true ;

// check parameter
if ( typeof( sdb ) === "undefined" &&
     typeof( mysql ) === "undefined" &&
     typeof( pg ) === "undefined" )
{
   var sdb = true ;
   var mysql = true ;
   var pg = true ;
   USER_SET_DEPLOY = false ;
}

if ( typeof( sdb ) === "undefined" )
{
   var sdb = false ;
}
else if( sdb.constructor !== Boolean )
{
   throw "Invalid para[sdb], should be Boolean" ;
}

if ( typeof( mysql ) === "undefined" )
{
   var mysql = false ;
}
else if( mysql.constructor !== Boolean )
{
   throw "Invalid para[mysql], should be Boolean" ;
}

if ( typeof( pg ) === "undefined" )
{
   var pg = false ;
}
else if( pg.constructor !== Boolean )
{
   throw "Invalid para[pg], should be Boolean" ;
}

if ( typeof( cm ) === "undefined" )
{
   var cm = 11790 ;
}
else if( cm.constructor !== Number )
{
   throw "Invalid para[cm], should be Number" ;
}

// set global variable
var DEPLOY_SEQUOIADB  = sdb ;
var DEPLOY_MYSQL      = mysql ;
var DEPLOY_POSTGRESQL = pg ;
var LOCAL_CM_PORT     = cm ;
var TMP_COORD_SVC     = 18800 ;
var MY_HOSTNAME       = System.getHostName() ;
var TRANSACTION_CONF  = { transactionon: true, transautocommit: true } ;

// run!
main() ;

function main()
{
   var ignoreNotInstall = true ;
   if ( USER_SET_DEPLOY ) ignoreNotInstall = false ;

   if ( DEPLOY_SEQUOIADB )
   {
      deploySequoiadb() ;
   }
   if ( DEPLOY_MYSQL )
   {
      deployMysql( ignoreNotInstall ) ;
   }
   if ( DEPLOY_POSTGRESQL )
   {
      deployPostgresql( ignoreNotInstall ) ;
   }
}

// return obj:
// {
//   "VERSION": "3.2",
//   "USER": "sdbadmin",
//   "INSTALL_DIR": "/opt/sequoiasql/mysql",
//   "MD5": "818cea64849dff4c1b572a6d6af5d757"
// }
function getSqlInstallInfo( dbType, ignoreNotInstall )
{
   var systemFile = "" ;
   var dbFullType = "" ;
   if ( dbType == "mysql" )
   {
      systemFile = "/etc/default/sequoiasql-mysql" ;
      dbFullType = "SequoiaSQL-MySQL" ;
   }
   else if ( dbType == "postgresql" )
   {
      systemFile = "/etc/default/sequoiasql-postgresql" ;
      dbFullType = "SequoiaSQL-PostgreSQL" ;
   }
   else
   {
      println( "Invalid type[" + dbType + "]!") ;
      throw "ERROR" ;
   }

   try
   {
      var file = new File( systemFile, 0644, SDB_FILE_READONLY ) ;
   }
   catch( e )
   {
      if ( e == -4 )
      {
         if ( ignoreNotInstall ) return ;
         println( "ERROR: This machine has not installed " +
                  dbFullType + "!" ) ;
         throw "ERROR" ;
      }
      else
      {
         throw e ;
      }
   }

   var infoObj = {} ;
   while( true )
   {
      var aLine ;
      try
      {
         aLine = file.readLine() ;
         aLine = aLine.replace( /[\r\n]/g, "" ) ; // delete last line break
      }
      catch( e )
      {
         if( e == -9 ) break ; // -9: Hit end of file
         println( "Unexpected error[" + e + "] when read a line from " +
                  "configure file!" ) ;
         throw e ;
      }

      var conf = aLine.split( "=" ) ;
      infoObj[ conf[0] ] = conf[1] ;
   }

   return infoObj ;
}

// return obj:
// {
//   "NAME": "sdbcm",
//   "SDBADMIN_USER": "sdbadmin",
//   "INSTALL_DIR": "/opt/source/sequoiadb/",
//   "MD5": "818cea64849dff4c1b572a6d6af5d757"
// }
function getSequoiadbInstallInfo( hostName )
{
   try
   {
      var oma = new Oma( MY_HOSTNAME, LOCAL_CM_PORT ) ;
   }
   catch( e )
   {

      println( "Unexpected error[" + e + "] when connecting cm[" + MY_HOSTNAME +
               ":" + LOCAL_CM_PORT + "]!" ) ;
      throw e ;
   }

   try
   {
      var cmPort = oma.getAOmaSvcName( hostName ) ;
      var omaRemote = new Oma( hostName, cmPort ) ;
   }
   catch( e )
   {

      println( "Unexpected error[" + e + "] when connecting cm[" + hostName +
               ":" + cmPort + "]!" ) ;
      throw e ;
   }

   var installInfo = {} ;
   try
   {
      installInfo = omaRemote.getOmaInstallInfo().toObj() ;
   }
   catch( e )
   {
      if ( e == -4 )
      {
         println( "ERROR: This machine has not installed SequoiaDB!" ) ;
         throw "ERROR" ;
      }
      else
      {
         throw e ;
      }
   }

   return installInfo ;
}

function getSqlConf( dbType, installedPath )
{
   var selfPath = getSelfPath() ;

   var confFile = "" ;
   if ( dbType == "mysql" )
   {
      confFile = "mysql.conf" ;
   }
   else if ( dbType == "postgresql" )
   {
      confFile = "postgresql.conf" ;
   }
   else
   {
      println( "Invalid db type!" ) ;
      return ;
   }

   var fileFullPath = selfPath + "/" + confFile ;
   try
   {
      var file = new File( fileFullPath, 0644, SDB_FILE_READONLY ) ;
   }
   catch( e )
   {
      if ( e == -4 )
      {
         println( "File[" + fileFullPath + "] not exist!" ) ;
         return ;
      }
      else
      {
         throw e ;
      }
   }

   // check first line     TODO try catche
   var headLine = file.readLine() ;
   headLine =  headLine.replace( /[\r\n]/g, "" ) ; // delete last line break
   if ( headLine != "instanceName,port,databaseDir,coordAddr" )
   {
      println( "Invalide configure file! first line: " + headLine ) ;
      throw "ERROR" ;
   }

   // loop each line
   var allConf = [] ;
   while( true )
   {
      var aLine ;
      try
      {
         aLine = file.readLine() ;
         aLine = aLine.replace( /[\r\n]/g, "" ) ; // delete last line break
      }
      catch( e )
      {
         if( e == -9 ) break ; // -9: Hit end of file
         println( "Unexpected error[" + e + "] when read a line from " +
                  "configure file!" ) ;
         throw e ;
      }

      // check line
      if ( aLine == "" ) continue ;

      if ( aLine.substr( 0,1 ) == "#" ) continue ;   // this line is a note

      // split line
      var instanceConf = aLine.split( "," ) ;
      var len = instanceConf.length ;
      if ( len < 4 )
      {
         println( "Invalid configure file[" + confFile + "]!" ) ;
         throw "ERROR" ;
      }
      else if ( len == 4 )
      {
         var coordAddr = instanceConf[3] ;
         if ( coordAddr.substr( 0, 1 )  == "[" &&
              coordAddr.substr( -1, 1 ) == "]" )
         {
            // delete the '[' at the beginning of the line
            coordAddr = coordAddr.replace( /(^\[)/, '' ) ;
            // delete the ']' at the end of the line
            coordAddr = coordAddr.replace( /(\]$)/, '' ) ;
            instanceConf[3] = coordAddr ;
         }
         if ( coordAddr == "" )
         {
            println( "Invalid configure file[" + confFile + "]!" ) ;
            throw "ERROR" ;
         }
      }
      else if ( len > 4 )
      {
         var coordAddr = "" ;
         for ( var i = 3; i < len ; i++ )
         {
            coordAddr += instanceConf[i] ;
            if ( i != ( len - 1 ) )
            {
               coordAddr += "," ;
            }
         }
         // check first char is '[', last char is ']'
         if ( coordAddr.substr( 0, 1 )  != "[" ||
              coordAddr.substr( -1, 1 ) != "]" )
         {
            println( "Invalid configure file[" + confFile + "]!" ) ;
            throw "ERROR" ;
         }
         // delete the '[' at the beginning of the line
         coordAddr = coordAddr.replace( /(^\[)/, '' ) ;
         // delete the ']' at the end of the line
         coordAddr = coordAddr.replace( /(\]$)/, '' ) ;
         instanceConf[3] = coordAddr ;
         instanceConf.splice( 4, len - 4 ) ;
      }

      // replace installed path
      instanceConf[2] = instanceConf[2].replace( /\[installPath\]/g,
                                                 installedPath ) ;

      // set coord address
      if ( instanceConf[3] == "-" )
      {
         instanceConf[3] = getACoordAddr() ;
      }

      allConf.push( instanceConf ) ;
   }

   return allConf ;
}

function getACoordAddr()
{
   var coordAddr = "" ;

   var nodesConf = getSequoiadbConf() ;
   for ( var i in nodesConf )
   {
      var aNode = nodesConf[i] ;
      if ( aNode[0] == "coord" )
      {
         coordAddr = aNode[2] + ":" + aNode[3] ;
         break ;
      }
   }

   return coordAddr ;
}

function getSequoiadbConf( replaceInstallPath )
{
   if ( typeof( replaceInstallPath ) === "undefined" )
   {
      replaceInstallPath = false ;
   }

   var selfPath = getSelfPath() ;
   var fileFullPath = selfPath + "/sequoiadb.conf" ;
   var file = new File( fileFullPath, 0644, SDB_FILE_READONLY ) ;

   // check first line     TODO try catche
   var headLine = file.readLine() ;
   headLine =  headLine.replace( /[\r\n]/g, "" ) ; // delete last line break
   if ( headLine != "role,groupName,hostName,serviceName,dbPath" )
   {
      println( "Invalide configure file! first line: " + headLine ) ;
      throw "ERROR" ;
   }

   // loop each line
   var nodesConf = [] ;
   while( true )
   {
      var aLine ;
      try
      {
         aLine = file.readLine() ;
         aLine = aLine.replace( /[\r\n]/g, "" ) ; // delete last line break
      }
      catch( e )
      {
         if( e == -9 ) break ; // -9: Hit end of file
         println( "Unexpected error[" + e + "] when read a line from " +
                  "configure file!" ) ;
         throw e ;
      }

      // check line
      if ( aLine == "" ) continue ;

      if ( aLine.substr( 0,1 ) == "#" ) continue ;   // this line is a note

      var aNode = aLine.split( "," ) ;
      if ( aNode.length != 5 )
      {
         println( "Invalid configure file!" ) ;
         throw "ERROR" ;
      }

      // replace 'localhost' to real hostname
      aNode[2] = aNode[2].replace( /localhost/g, MY_HOSTNAME ) ;

      // replace installed path
      if ( replaceInstallPath )
      {
         var installedPath = getSequoiadbInstallInfo( aNode[2] ).INSTALL_DIR ;
         aNode[4] = aNode[4].replace( /\[installPath\]/g, installedPath ) ;
      }

      nodesConf.push( aNode ) ;
   }

   return nodesConf ;
}

function createTmpCoord()
{
   var oma = new Oma( MY_HOSTNAME, LOCAL_CM_PORT ) ;

   var service = TMP_COORD_SVC ;
   var installedPath = getSequoiadbInstallInfo( MY_HOSTNAME ).INSTALL_DIR ;

   try
   {
      oma.createCoord( service, installedPath + "/database/coord/" + service ) ;
   }
   catch( e )
   {
      if ( e != -145 )  // -145: already exists, ignore error
      {
         println( "Unexpected error[" + e + "] when creating temp coord: " +
                  "localhost:" + service + "!" ) ;
         throw e ;
      }
   }

   oma.startNode( service ) ;
}

function checkCataPrimary( db )
{
   var hasPrimary = false;

   for( var i = 0; i < 10*600; i++ )//wait for cata group to select primary node
   {
      try
      {
         sleep( 100 ) ;
         var cataRG = db.getRG( "SYSCatalogGroup" ) ;
         hasPrimary = true ;
         break ;
      }
      catch(e)
      {
         if( e !== -71 )
         {
            println( "Unexpected error[" + e + "] when " +
                     "db.getRG( \"SYSCatalogGroup\" )!" ) ;
            throw e;
         }
      }
   }

   if( hasPrimary === false )
   {
      println( "Fail to select primary node in group[SYSCatalogGroup] " +
               "after 10 minute" ) ;
      return false ;
   }

   return true ;
}

function checkeDataPrimary( db, groupName )
{
   var hasPrimary = false ;

   for( var i = 0; i < 10*600; i++ )//wait for data group to select primary node
   {
      try
      {
         sleep(100);
         db.getRG( groupName ).getMaster();
         hasPrimary = true;
         break;
      }
      catch(e)
      {
         if( e !== -71 )
         {
            println( "Unexpected error[" + e + "] when getting group[" +
                     groupName + "]!" ) ;
            throw e;
         }
      }
   }

   if( hasPrimary === false )
   {
      println( "Fail to select primary node in group[" + groupName +
               "] after 10 minute" ) ;
      return false ;
   }

   return true ;
}

function addCataAddr2TmpCoord( cataHostName, cataSvc )
{
   var oma = new Oma( MY_HOSTNAME, LOCAL_CM_PORT ) ;

   var cataPort = parseInt( cataSvc ) + 3 ;
   var cataAddrSetting = cataHostName + ":" + cataPort ;
   oma.updateNodeConfigs( TMP_COORD_SVC, { catalogaddr: cataAddrSetting } ) ;

   oma.stopNode( TMP_COORD_SVC ) ;
   oma.startNode( TMP_COORD_SVC ) ;

   var db = new Sdb( MY_HOSTNAME, TMP_COORD_SVC ) ;

   return db ;
}

function createCatalog( nodesConf )
{
   if ( nodesConf.length == 0 ) return ;

   var db = new Sdb( MY_HOSTNAME, TMP_COORD_SVC ) ;

   for ( var i in nodesConf )
   {
      var aNodeConf = nodesConf[i] ;
      var hostName = aNodeConf[2] ;
      var service = aNodeConf[3] ;
      var dbPath = aNodeConf[4] ;

      if ( i == 0 )
      {
         try
         {
            db.createCataRG( hostName, service, dbPath ) ;
         }
         catch( e )
         {
            if ( e == -145 || e == -200 ) // -145: already exists, ignore error
            {
               db = addCataAddr2TmpCoord( hostName, service ) ;
            }
            else
            {
               println( "Unexpected error[" + e + "] when creating catalog " +
                        "node: " + hostName + ":" + service + "!" ) ;
               throw e ;
            }
         }
      }
      else
      {
         try
         {
            var rg = db.getCatalogRG() ;
            var node = rg.createNode( hostName, service, dbPath ) ;
         }
         catch( e )
         {
            if ( e != -145 ) // -145: already exists, ignore error
            {
               println( "Unexpected error[" + e + "] when creating catalog " +
                         "node: " + hostName + ":" + service + "!" ) ;
               throw e ;
            }
         }
         try
         {
            rg.getNode( hostName, service ).start() ;
         }
         catch( e )
         {
            println( "Unexpected error[" + e + "] when starting catalog node: "
                     + hostName + ":" + service + "!" ) ;
            throw e ;
         }
      }

      println( "Create catalog: " + hostName + ":" + service ) ;
   }

   var rc = checkCataPrimary( db ) ;
   if ( !rc )
   {
      println( "Failed to wait catalog group change primary!" ) ;
      throw "ERROR" ;
   }
}

function createCoord( nodesConf )
{
   if ( nodesConf.length == 0 ) return ;

   var db = new Sdb( MY_HOSTNAME, TMP_COORD_SVC ) ;

   try
   {
      db.createCoordRG() ;
   }
   catch( e )
   {
      if ( e != -153 ) // -153: group already exist, ignore error
      {
         println( "Unexpected error[" + e + "] when creating coord group!" ) ;
         throw e ;
      }
   }

   for ( var i in nodesConf )
   {
      var aNodeConf = nodesConf[i] ;
      var hostName = aNodeConf[2] ;
      var service = aNodeConf[3] ;
      var dbPath = aNodeConf[4] ;

      try
      {
         var rg = db.getCoordRG() ;
         rg.createNode( hostName, service, dbPath, TRANSACTION_CONF ) ;
      }
      catch( e )
      {
         if ( e != -145 ) // -145: node already exist, ignore error
         {
            println( "Unexpected error[" + e + "] when creating coord node!" ) ;
            throw e ;
         }
      }

      println( "Create coord:   " + hostName + ":" + service ) ;
   }

   try
   {
      rg.start() ;
   }
   catch( e )
   {
      println( "Unexpected error[" + e + "] when starting coord group!" ) ;
      throw e ;
   }
}

function createData( nodesConf )
{
   if ( nodesConf.length == 0 ) return ;

   var db = new Sdb( MY_HOSTNAME, TMP_COORD_SVC ) ;

   for ( var i in nodesConf )
   {
      var aNodeConf = nodesConf[i] ;
      var groupName = aNodeConf[1] ;
      var hostName = aNodeConf[2] ;
      var service = aNodeConf[3] ;
      var dbPath = aNodeConf[4] ;

      try
      {
         var rg = db.getRG( groupName ) ;
      }
      catch( e )
      {
         if ( e == -154 )
         {
            var rg = db.createRG( groupName ) ;
         }
         else
         {
            println( "Unexpected error[" + e + "] when get data group[" +
                     groupName + "]!" ) ;
            throw e ;
         }
      }

      try
      {
         rg.createNode( hostName, service, dbPath, TRANSACTION_CONF ) ;
      }
      catch( e )
      {
         if ( e != -145 )
         {
            println( "Unexpected error[" + e + "] when creating data node[" +
                     hostName + ":" + service + "]!" ) ;
            throw e ;
         }
      }

      println( "Create data:    " + hostName + ":" + service ) ;
   }

   for ( var i in nodesConf )
   {
      var aNodeConf = nodesConf[i] ;
      var groupName = aNodeConf[1] ;

      try
      {
         var rg = db.getRG( groupName ) ;
         rg.start() ;
      }
      catch( e )
      {
         println( "Unexpected error[" + e + "] when starting data group[" +
                  groupName + "]!" ) ;
         throw e ;
      }
   }
}

function removeTmpCoord()
{
   var oma = new Oma( MY_HOSTNAME, LOCAL_CM_PORT ) ;
   var service = TMP_COORD_SVC ;

   try
   {
      oma.removeCoord( service ) ;
   }
   catch( e )
   {
      println( "Unexpected error[" + e + "] when removing temp coord[localhost:"
               + service + "]!" ) ;
      throw e ;
   }
}

function checkUser( dbType, installInfo )
{
   if ( dbType == "sequoiadb" )
   {
      var expUser = installInfo.SDBADMIN_USER ;
   }
   else
   {
      var expUser = installInfo.USER ;
   }

   var curUser = System.getUserEnv().toObj().USER ;

   if ( expUser != curUser )
   {
      println( "You should execute this script by user[" + expUser + "], " +
               "but current user is [" + curUser + "]!" ) ;
      throw "ERROR" ;
   }
}

function deploySequoiadb()
{
   println( "\n************ Deploy SequoiaDB ************************" ) ;

   // check it has installation or not
   var installInfo = getSequoiadbInstallInfo( MY_HOSTNAME ) ;
   if ( installInfo == undefined )
   {
      throw "ERROR" ;
   }

   // check user
   checkUser( "sequoiadb", installInfo ) ;

   // get node configure
   var catalogConf = [] ;
   var coordConf = [] ;
   var dataConf = [] ;
   var nodesConf = getSequoiadbConf( true ) ;
   for ( var i in nodesConf )
   {
      var aNodeConf = nodesConf[i] ;
      var role = aNodeConf[0] ;
      if ( role == "catalog" )
      {
         catalogConf.push( aNodeConf ) ;
      }
      else if ( role == "coord" )
      {
         coordConf.push( aNodeConf ) ;
      }
      else if ( role == "data" )
      {
         dataConf.push( aNodeConf ) ;
      }
      else
      {
         println( "Unexpect configure: role[" + role + "]" ) ;
         throw "ERROR" ;
      }
   }

   if ( catalogConf.length < 1 )
   {
      println( "SequoiaDB need at least 1 catalog!" ) ;
      throw "ERROR" ;
   }

   // create sequoiadb cluster
   createTmpCoord() ;

   createCatalog( catalogConf ) ;

   createCoord( coordConf ) ;

   createData( dataConf ) ;

   removeTmpCoord() ;
}

function deployMysql( ignoreNotInstall )
{
   if ( !ignoreNotInstall )
   {
      println( "\n************ Deploy SequoiaSQL-MySQL *****************" ) ;
   }

   // check it has installation or not
   var installInfo = getSqlInstallInfo( "mysql", ignoreNotInstall ) ;
   if ( installInfo == undefined && ignoreNotInstall )
   {
      return ;
   }
   var installedPath = installInfo.INSTALL_DIR ;

   if ( ignoreNotInstall )
   {
      println( "\n************ Deploy SequoiaSQL-MySQL *****************" ) ;
   }

   // check user
   checkUser( "mysql", installInfo ) ;

   var sqlCtl = installedPath + "/bin/sdb_sql_ctl" ;
   var cmd = new Cmd() ;

   // get configure
   var allConf = getSqlConf( "mysql", installedPath ) ;

   // create instance
   for ( var i in allConf )
   {
      var instanceConf = allConf[i] ;
      var instanceName = instanceConf[0] ;
      var port = instanceConf[1] ;
      var databaseDir = instanceConf[2] ;
      var coordAddr = instanceConf[3] ;
      var newInst = true ;

      try
      {
         // add instance
         var command = sqlCtl + " addinst "+ instanceName +" -D " +
                       databaseDir + " -p " + port ;
         cmd.run( command ) ;
      }
      catch( e )
      {
         var rc = cmd.getLastRet() ;
         if ( rc == 8 ) // 8: instance exist
         {
            newInst = false ;
         }
         else
         {
            println( cmd.getLastOut() ) ;
            throw e ;
         }
      }

      try
      {
         // set coord address
         var coordSetting = "sequoiadb_conn_addr=\"" + coordAddr + "\"" ;
         var file = new File( databaseDir + "/auto.cnf" ) ;
         var content = file.read() ;
         content = content.replace( /sequoiadb_conn_addr=(.*)/g, coordSetting );
         if ( content.indexOf( "sequoiadb_conn_addr=" ) == -1 )
         {
            content = content.replace( /\[mysqld\]/g,
                                       "[mysqld]\n" + coordSetting ) ;
         }
         file.seek( 0 ) ;
         file.write( content ) ;

         // restart instance to make the configuration take effect
         var command = sqlCtl + " restart " + instanceName ;
         cmd.run( command ) ;
      }
      catch( e )
      {
         if ( newInst )
         {
            println( cmd.getLastOut() ) ;
            throw e ;
         }
      }

      println( "Create instance: [name: " + instanceName + ", port: " + port +
               "]" ) ;
   }
}

function deployPostgresql( ignoreNotInstall )
{
   if ( !ignoreNotInstall )
   {
      println( "\n************ Deploy SequoiaSQL-PostgreSQL ************" ) ;
   }

   // check it has installation or not
   var installInfo = getSqlInstallInfo( "postgresql", ignoreNotInstall ) ;
   if ( installInfo == undefined && ignoreNotInstall )
   {
      return ;
   }
   var installedPath = installInfo.INSTALL_DIR ;

   if ( ignoreNotInstall )
   {
      println( "\n************ Deploy SequoiaSQL-PostgreSQL ************" ) ;
   }

   // check user
   checkUser( "postgresql", installInfo ) ;

   var sqlCtl = installedPath + "/bin/sdb_sql_ctl" ;
   var psql = installedPath + "/bin/psql" ;
   var cmd = new Cmd() ;

   // get configure
   var allConf = getSqlConf( "postgresql", installedPath ) ;

   // create instance
   var dbName = "foo" ;
   for ( var i in allConf )
   {
      var instanceConf = allConf[i] ;
      var instanceName = instanceConf[0] ;
      var port = instanceConf[1] ;
      var databaseDir = instanceConf[2] ;
      var coordAddr = instanceConf[3] ;
      var newInst = true ;

      try
      {
         // add instance
         var command = sqlCtl + " addinst "+ instanceName +" -D " + databaseDir
                       + " -p " + port ;
         cmd.run( command ) ;
      }
      catch( e )
      {
         var rc = cmd.getLastRet() ;
         if ( rc == 8 ) // 8: instance exist
         {
            newInst = false ;
         }
         else
         {
            println( cmd.getLastOut() ) ;
            throw e ;
         }
      }

      try
      {
         // start instance
         var command = sqlCtl + " start " + instanceName ;
         cmd.run( command ) ;

         // create db
         var command = sqlCtl + " createdb " + dbName + " " + instanceName ;
         cmd.run( command ) ;

         // set coord address
         var envCmd = "export LD_LIBRARY_PATH=" + installedPath + "/lib; " ;
         var command = envCmd + psql + " -p " + port + " " + dbName +
                       " -c \"create extension sdb_fdw\"" ;
         cmd.run( command ) ;

         var command = envCmd + psql + " -p " + port + " " + dbName
                              + " -c \"create server sdb_server foreign "
                              + "data wrapper sdb_fdw options(address '"
                              + coordAddr + "', transaction 'off' );\"" ;
         cmd.run( command ) ;
      }
      catch( e )
      {
         if ( newInst )
         {
            println( cmd.getLastOut() ) ;
            throw e ;
         }
      }

      println( "Create instance: [name: " + instanceName + ", port: " + port +
               "]" ) ;
   }
}
