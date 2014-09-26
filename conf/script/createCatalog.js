/*******************************************************************************

   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*******************************************************************************/
/*
@description: create catalog
@modify list:
   2014-7-26 Zhaobo Tan  Init
@parameter
   BUS_JSON: the format is: { "InstallHostName": "rhel64-test8", "InstallSvcName": "11800", "InstallPath": "/opt/sequoiadb/database/catalog", "InstallConfig": { "diaglevel": 3, "role": "catalog", "logfilesz": 64, "logfilenum": 20, "transactionon": "false", "preferedinstance": "A", "numpagecleaners": 1, "pagecleaninterval": 10000, "hjbuf": 128, "logbuffsize": 1024, "maxprefpool": 200, "maxreplsync": 10, "numpreload": 0, "sortbuf": 512, "syncstrategy": "none" } }
   SYS_JSON: the format is: { "VCoordSvcName": "11792", "SdbUser": "sdbadmin", "SdbPasswd": "sdbadmin", "SdbUserGroup": "sdbadmin_group", "User": "root", "Passwd": "sequoiadb" } 
   ENV_JSON:
@return
   RET_JSON: the format is:
*/

//var BUS_JSON = { "InstallHostName": "rhel64-test8", "InstallSvcName": "12000", "InstallPath": "/tmp/sequoiadb/database/catalog/12000", "InstallConfig": { "diaglevel": 3, "role": "catalog", "logfilesz": 64, "logfilenum": 20, "transactionon": "false", "preferedinstance": "A", "numpagecleaners": 1, "pagecleaninterval": 10000, "hjbuf": 128, "logbuffsize": 1024, "maxprefpool": 200, "maxreplsync": 10, "numpreload": 0, "sortbuf": 512, "syncstrategy": "none", "VCoordSvcName": "11792" } } ;

//var SYS_JSON = { "VCoordSvcName": "11792", "SdbUser": "sdbadmin", "SdbPasswd": "sdbadmin", "SdbUserGroup": "sdbadmin_group", "User": "root", "Passwd": "sequoiadb" } ;

var RET_JSON     = new Object() ;
RET_JSON[Rc]     = SDB_OK ;
RET_JSON[Detail] = "" ;

/* *****************************************************************************
@discretion: wait catalog to be ok
@author: Tanzhaobo
@parameter
   db[object]: Sdb object
@return void
***************************************************************************** */
function waitCatalogRGReady( db )
{
   var i = 0 ;
   for ( ; i < OMA_WAIT_CATA_RG_TRY_TIMES; i++ )
   {
print("i is: " + i + "\n")
      try
      {
         db.list( SDB_LIST_GROUPS ) ;
         break ;
      }
      catch ( e )
      {
print("wait e is: " + e + "\n")
         if ( SDB_RTN_NO_PRIMARY_FOUND == e )
         {
print("22222222222222222222222\n")
            sleep( OMA_SLEEP_TIME ) ;
            continue ;
         }
         else
         {
print("11111111111111111111111111111111111111111\n")
            if ( "number" == typeof( e ) )
            {
               setLastErrMsg( "Failed to wait catalog to be ready: " + getErr( e ) ) ;
               setLastError( e ) ;
               throw e ;
            }
            else
            {
               throw e ;
            }
         }
      }
   }
   if ( OMA_WAIT_CATA_RG_TRY_TIMES <= i )
   {
      setLastErrMsg( "Wait catalog to be ready timeout" ) ;
      setLastError( SDB_SYS ) ;
      throw SDB_SYS ;
   }
}

function createCatalogNode( db, hostname, svcname, installpath, config )
{
   // try to get system catalog group
   var rg = null ;
   var node = null ;
   try
   {
      rg = db.getRG( OMA_SYS_CATALOG_RG ) ;
   }
   // catalog has not been created
   catch ( e )
   {
print("999999999999999999 e is : " + e + "\n")
      if ( SDB_CAT_NO_ADDR_LIST == e )
      {
         try
         {
            rg = db.createCataRG( hostname, svcname,
                                  installpath, config ) ;
            return ;
         }
         catch ( e )
         {
print("88888888888888 e is: " + e + "\n")
            if ( "number" == typeof( e ) )
            {
               setLastErrMsg( "Failed to create catalog group: " + getErr( e ) ) ;
               setLastError( e ) ;
               throw e ;
            }
            else
            {
               throw e ;
            }
         }
      }
      else
      {
print("77777777777777 e is: " + e + "\n")
         if ( "number" == typeof( e ) )
         {
            setLastErrMsg( "Failed to get catalog group: " + getErr( e ) ) ;
            setLastError( e ) ;
            throw e ;
         }
         else
         {
            throw e ;
         }
      }
   }
print("66666666666666666666666666666\n")
   // catalog has been created
   try
   {
      node = rg.createNode( hostname, svcname, installpath, config ) ;
print("5555555555555555555555555\n")
   }
   catch ( e )
   {
      if ( "number" == typeof( e ) )
      {
         setLastErrMsg( "Failed to create catalog: " + getErr( e ) ) ;
         setLastError( e ) ;
         throw e ;
      }
      else
      {
         throw e ;
      }
   }
   try
   {
print("4444444444444444444444444\n")
      node.start() ;
print("333333333333333333333\n")
   }
   catch ( e )
   {
      if ( "number" == typeof( e ) )
      {
         setLastErrMsg( "Failed to start catalog node: " + getErr( e ) ) ;
         setLastError( e ) ;
         throw e ;
      }
      else
      {
         throw e ;
      }
   }
}

/*
var BUS_JSON = { "InstallHostName": "rhel64-test8", "InstallSvcName": "11800", "InstallPath": "/opt/sequoiadb/database/catalog", "InstallConfig": { "diaglevel": 3, "role": "catalog", "logfilesz": 64, "logfilenum": 20, "transactionon": "false", "preferedinstance": "A", "numpagecleaners": 1, "pagecleaninterval": 10000, "hjbuf": 128, "logbuffsize": 1024, "maxprefpool": 200, "maxreplsync": 10, "numpreload": 0, "sortbuf": 512, "syncstrategy": "none", "VCoordSvcName": "11792" } } ;

var SYS_JSON = { "VCoordSvcName": "11792", "SdbUser": "sdbadmin", "SdbPasswd": "sdbadmin", "SdbUserGroup": "sdbadmin_group", "User": "root", "Passwd": "sequoiadb" } ;
*/

function main()
{
    var vCoordHostName  = System.getHostName() ;
    var vCoordSvcName   = SYS_JSON[VCoordSvcName] ;
    var sdbUser         = SYS_JSON[SdbUser] ;
    var sdbUserGroup    = SYS_JSON[SdbUserGroup] ;
    var user            = SYS_JSON[User] ;
    var passwd          = SYS_JSON[Passwd] ;    
    var installHostName = BUS_JSON[InstallHostName] ;
    var installSvcName  = BUS_JSON[InstallSvcName] ;
    var installPath     = BUS_JSON[InstallPath] ;
    var installConfig   = BUS_JSON[InstallConfig] ;


print("00000000000000000000000000000\n")
print("user is: " + user + "\n")
print("passwd is: " + passwd + "\n")
    var ssh             = new Ssh( installHostName, user, passwd ) ;
    var osInfo          = System.type() ; 
print("1111111111111111\n")
    // change install path owner
    changeDirOwner( ssh, osInfo, installPath, sdbUser, sdbUserGroup ) ;
print("222222222222222222222\n")
    // connect to virtual coord
    var db = new Sdb( vCoordHostName, vCoordSvcName, "", "" ) ;
    // create catalog node
print("3333333333333333333333333333\n") ;
    createCatalogNode( db, installHostName, installSvcName,
                       installPath, installConfig ) ;
print("444444444444444444444444444444\n")
    // wait catalog to be available
    waitCatalogRGReady( db ) ; 
print("5555555555555555555555555555555\n")
}

// execute
   main() ;

