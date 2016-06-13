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
@description: rebuild OM
@modify list:
   2016-5-27 Zhaobo Tan  Init
@parameter
   OM_CONF_FILE: OM configure file
@return
   void 
*/

// TODO:
setLogLevel( PDDEBUG ) ;
//setLogLevel( PDWARNING ) ;

var FILE_NAME_REBUILD_OM = "rebuildOM.js" ;
var logger = new Logger(FILE_NAME_REBUILD_OM) ;

var ts = null ;

function _init() {
   logger.log( PDEVENT, "Begin to rebuild om" ) ;
   ts = genTimeStamp() ;
} ;

function _final() {
   logger.log( PDEVENT, "Finish rebuilding om" ) ;
} ;

var ConfigMgr = function ( omConfigFile ) {
   this._configFile  = omConfigFile ;
   this._omConfObj   = null ;
   this._sdb         = null ;
   this._clusterInfo = null ;
   this._rgInfoArr   = [] ;
   this._hostInfoArr = [] ;

} ;

ConfigMgr.prototype._getRGInfoArr = function ConfigMgr__getRGInfoArr() {
   return this._rgInfoArr ;
} ;

ConfigMgr.prototype._getHostInfoArr = function ConfigMgr__getHostInfoArr() {
   return this._hostInfoArr ;
} ;

ConfigMgr.prototype._getSdbObj = function ConfigMgr__getSdbObj() {
   return this._sdb ;
} ;

ConfigMgr.prototype._getCluterInfo = function ConfigMgr__getClusterInfo() {
   return this._clusterInfo ;
} ;

ConfigMgr.prototype._getField = 
   function ConfigMgr__getField( fieldName ) {
   var ret    = null ;
   var errMsg = null ;
   var err    = null ;
   var field  = this._omConfObj[fieldName] ;
   if ( "" == field || null == field || "undefined" == typeof(field) ) {
      errMsg = "can not get value according to field name [" + fieldName + "]" ;
      err = new SdbError( SDB_INVALIDARG, errMsg ) ;
      logger.log( PDERROR, err ) ;
      throw err ;
   }
   ret = removeQuotes( field ) ;
   return ret ;
} ;

ConfigMgr.prototype._getClusterInfo = 
   function ConfigMgr__getClusterInfo() {
   var retObj = new ClusterInfo() ;
   retObj.clusterName         = this._getField( FIELD_CLUSTER_NAME ) ;
   retObj.clusterDescription  = this._getField( FIELD_CLUSTER_DESCRIPTION ) ;
   retObj.sdbUserName         = this._getField( FIELD_SDB_USER_NAME ) ;
   retObj.sdbPassword         = this._getField( FIELD_SDB_PASSWORD ) ;
   retObj.sdbUserGroupName    = this._getField( FIELD_SDB_USER_GROUP_NAME ) ;
   retObj.sdbInstallPath      = this._getField( FIELD_SDB_INSTALL_PATH ) ;
   return retObj ;
} ;

ConfigMgr.prototype._appendHostName = 
   function ConfigMgr__appendHostName() {
   var value   = this._getField( FIELD_HOST_NAME ) ;
   var nameArr = value.split( ',' ) ;
   var len     = nameArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo = new HostInfo() ;
      hostInfo.address  = strTrim( nameArr[i] ) ;
      this._hostInfoArr.push( hostInfo ) ;
   }  
} ;

ConfigMgr.prototype._appendRootInfo = 
   function ConfigMgr__appendRootInfo() {
   var defaultRootUserName  = this._getField( FIELD_DEFAULT_ROOT_USER ) ;
   var defaultRootPassword  = this._getField( FIELD_DEFAULT_ROOT_PASSWD ) ;
   var len = this._hostInfoArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo      = this._hostInfoArr[i] ;
      var rootUserName  = null ;
      var rootPassword  = null ;
      var field         = null ;
      var value         = null ;
      
      // get specific root info
      field          = hostInfo.address + FIELD_ROOT_USER ;
      value          = this._omConfObj[field] ;
      rootUserName   = ( value == undefined ) ? defaultRootUserName : 
         removeQuotes( value ) ;
      field          = hostInfo.address + FIELD_ROOT_PASSWD ;
      value          = this._omConfObj[field] ;
      rootPassword   = ( value == undefined ) ? defaultRootPassword : 
         removeQuotes( value ) ;
      // append root info
      hostInfo.rootUserName  = rootUserName ;
      hostInfo.rootPassword  = rootPassword ;
   }
} ;

ConfigMgr.prototype._appendSshPort = 
   function ConfigMgr__appendSshPort() {
   var value          = this._getField( FIELD_DEFAULT_SSH_PORT ) ;
   var defaultSshPort = parseInt( value ) ;
   var len            = this._hostInfoArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo = this._hostInfoArr[i] ;
      var sshPort     = null ;
      var field       = null ;
      var value       = null ;
      // get specific ssh port
      field        = hostInfo.address + FIELD_SSH_PORT ;
      value        = this._omConfObj[field] ;
      sshPort      = ( value == undefined ) ? defaultSshPort : 
         parseInt( removeQuotes( value ) ) ;
      // append ssh info
      hostInfo.sshPort = sshPort ;
   }
} ;

ConfigMgr.prototype._appendInstallPath =
   function ConfigMgr__appendInstallPath() {
   var sdbInstallPath = this._getField( FIELD_SDB_INSTALL_PATH ) ;
   var len            = this._hostInfoArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo = this._hostInfoArr[i] ;
      var installPath = null ;
      var field       = null ;
      var value       = null ;
      // get specific install path
      field        = hostInfo.address + FIELD_INSTALL_PATH ;
      value        = this._omConfObj[field] ;
      installPath  = ( value == undefined ) ? sdbInstallPath : 
         removeQuotes( value ) ;
      // append install path
      hostInfo.installPath = installPath ;
   }
} ;

ConfigMgr.prototype._appendAdminInfo = 
   function ConfigMgr__appendAdminInfo() {
   var sdbUserName       = this._getField( FIELD_SDB_USER_NAME ) ;
   var sdbPassword       = this._getField( FIELD_SDB_PASSWORD ) ;
   var sdbUserGroupName  = this._getField( FIELD_SDB_USER_GROUP_NAME ) ;
   var len               = this._hostInfoArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo = this._hostInfoArr[i] ;
      // append admin info
      hostInfo.sdbUserName      = sdbUserName ;
      hostInfo.sdbPassword      = sdbPassword ;      
      hostInfo.sdbUserGroupName = sdbUserGroupName ;
   }
} ;

ConfigMgr.prototype._checkSsh = 
   function ConfigMgr__checkSsh( account ) {
   var errMsg    = null ;
   var errMsgArr = null ;
   var exp       = null ;
   var address   = null ;
   var user      = null ;
   var passwd    = null ;
   var sshPort   = null ;
   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      address = this._hostInfoArr[i].address ;
      sshPort = this._hostInfoArr[i].sshPort ;
      if ( account == "root" ) {
         user   = this._hostInfoArr[i].rootUserName ;
         passwd = this._hostInfoArr[i].rootPassword ;
      } else {
         user   = this._hostInfoArr[i].sdbUserName ;
         passwd = this._hostInfoArr[i].sdbPassword ;
      }
      
      try {
         var ssh = new Ssh( address, user, passwd, sshPort ) ;
      } catch( e ) {
         errMsg = "can not use account[" + user + 
            "] to ssh to host[" + address + "]" ;
         errMsgArr.push( new SdbError( e, errMsg ).toString() ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
      }
   }
   if ( exp != null ) {
      throw new SdbError( SDB_INVALIDARG, errMsgArr.toString() ) ;
   }
} ;

ConfigMgr.prototype._firstlyCheck = 
   function ConfigMgr_firstlyCheck() {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
   var address   = null ;
   logger.log( PDEVENT, "begin to firstly check" ) ;
   // 1. host is ip or not
   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      address = this._hostInfoArr[i].address ;
      errMsg  = null ;
      if ( isIP(address) ) {
         errMsg = "should offer hostname but not ip[" + address + "]" ;
      } else if ( address == "localhost" ) {
         errMsg = "should offer hostname but not localhost" ;        
      }
      if ( errMsg != null ) {
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
      }
   }
   if ( exp != null ) {
      throw new SdbError( SDB_INVALIDARG, errMsgArr.toString() ) ;
   }

   // 2. can ping or not
   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      address = this._hostInfoArr[i].address ;
      var result = eval( '(' + System.ping( address ) + ')' ) ;
      if ( result[Reachable] != true ) {
         errMsg = "host [" + address + "] is unreachable" ;
         errMsgArr.push( new SdbError( e, errMsg ).toString() ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
      }
   }
   if ( exp != null ) {
      throw new SdbError( SDB_INVALIDARG, errMsgArr.toString() ) ;
   }

   // 3. root account can ssh or not
   this._checkSsh( "root" ) ;

   // 4. admin account can ssh or not
   this._checkSsh( "admin" ) ;
   logger.log( PDEVENT, "finishing firstly check" ) ;
} ;

ConfigMgr.prototype._appendRootSshObj = 
   function ConfigMgr__appendRootSshObj() {
   var errMsg  = null ;
   var exp     = null ;
   var address = null ;
   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      address = this._hostInfoArr[i].address ;
      try {
         var ssh = new Ssh( address, this._hostInfoArr[i].rootUserName, 
            this._hostInfoArr[i].rootPassword, this._hostInfoArr[i].sshPort ) ;
         this._hostInfoArr[i].rootSshObj = ssh ;
      } catch( e ) {
         errMsg = "can not use root account to ssh to host[" + address + 
            "]" ;
         exp = new SdbError( e, errMsg )
         logger.log( PDERROR, exp ) ;
         throw exp ; 
      }
   }
} ;

ConfigMgr.prototype._appendIP = function ConfigMgr__appendIP() {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      var address = this._hostInfoArr[i].address ;
      var ssh = this._hostInfoArr[i].rootSshObj ;
      try {
         var ip = ssh.getPeerIP() ;
         if ( "string" != typeof(ip) ) {
            errMsg = "the return content[" + ip + "] from remote [" + 
               address +"] is not a ip address" ;
            errMsgArr.push( errMsg ) ;
            if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
            logger.log( PDERROR, errMsg ) ;
            continue ;
         }
         // append host name and ip info
         this._hostInfoArr[i].hostName = address ;
         this._hostInfoArr[i].ip       = removeLineBreak( ip ) ;
      } catch( e ) {
         errMsg = "failed to get host[" + address + "]'ip" ;
         errMsgArr.push( new SdbError( e, errMsg ).toString() ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
      }
   }
   if ( exp != null ) {
      throw new SdbError( SDB_SYS, errMsgArr.toString() ) ;
   }
} ;

ConfigMgr.prototype._appendRemoteInstalledInfo = 
   function ConfigMgr__appendRemoteInstalledInfo() {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
   
   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      var infoObj        = new Object() ;
      var ssh            = this._hostInfoArr[i].rootSshObj ;
      var address        = this._hostInfoArr[i].address ;
      var localFileName  = address + "_" + ts ;
      var remoteFileName = OMA_FILE_INSTALL_INFO ;
      logger.log( PDDEBUG, "localFileName is: " + localFileName + 
         ", remoteFileName is: " + remoteFileName ) ;
      if ( !ssh.isPathExist(remoteFileName) ) {
         errMsg = "file[" + remoteFileName + 
            "] does not exist in host[" + address + "]" ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      }
      try {
         ssh.pull( remoteFileName, localFileName ) ;
      } catch( e ) {
         errMsg = "failed to pull file[" + remoteFileName + 
            "] from host[" + address + "] to local" ;
         errMsgArr.push( new SdbError( e, errMsg ).toString() ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      }
      try {
         infoObj = eval( '(' + Oma.getOmaConfigs(localFileName) + ')' ) ;
      } catch( e ) {
         errMsg = "failed to extract installed info from file[" + localFileName + 
            "]" ;
         errMsgArr.push( new SdbError( e, errMsg ).toString() ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      } finally {
         try { File.remove(localFileName) ; } catch(e) {}
      }
      this._hostInfoArr[i].installedInfo = infoObj ;
   }
   if ( exp != null ) {
      throw new SdbError( SDB_SYS, errMsgArr.toString() ) ;
   }
} ;

ConfigMgr.prototype._secondlyCheck = 
   function ConfigMgr__secondlyCheck() {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
   var address   = null ;
   logger.log( PDEVENT, "begin to secondly check" ) ;
   // 1. check the specified admin accounts are the same with 
   // the one in remote or not
   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      var host        = this._hostInfoArr[i] ;
      // check the sdb admin user is the same with the one in remote or not
      if ( host.installedInfo[SDBADMIN_USER] != host.sdbUserName ) {
         errMsg = "the offered sdb admin account is different from " + 
            "the one in host[" + host.address + "]" ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      }
   }
   if ( exp != null ) {
      throw new SdbError( SDB_INVALIDARG, errMsgArr.toString() ) ;
   }
   
   // 2. check the specified install paths are the same with
   // the one in remote or not
   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      var host = this._hostInfoArr[i] ;
      var path = host.installPath ;
      var ssh  = host.rootSshObj ;
      // install path exist or not
      if ( !ssh.isPathExist(path) ) {
         errMsg = "path[" + path + "] does not exist in host[" + 
            host.address + "]" ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      }
      // install path the same with the one in remote host or not
      if ( host.installedInfo[INSTALL_DIR] != path ) {
         errMsg = "the offered install path[" + path + "] of host[" + 
            host.address + "] is different from the one in remote" ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      }
      // TODO: check whether pivotal executable program exist or not
   }
   if ( exp != null ) {
      throw new SdbError( SDB_INVALIDARG, errMsgArr.toString() ) ;
   }
   logger.log( PDEVENT, "finishing secondly check" ) ;
} ;

ConfigMgr.prototype._getHostNameFromRGInfo = 
   function ConfigMgr__getHostNameFromRGInfo() {
   var errMsg       = null ;
   var exp          = null ;
   var retArr       = [] ;

   for ( var i = 0; i < this._rgInfoArr.length; i++ ) {
      var obj      = this._rgInfoArr[i] ;
      var arr      = obj[Group] ;
      for ( var j = 0; j < arr.length; j++ ) {
         var subObj   = arr[j] ;
         var hostName = subObj[HostName] ;
         retArr.push( hostName ) ;
      }
   }
   return retArr ;
} ;

ConfigMgr.prototype._thirdlyCheck = 
   function ConfigMgr__thirdlyCheck() {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;

   logger.log( PDEVENT, "begin to thirdly check" ) ;
   // get the host from the return rg info
   var rgHostArr = this._getHostNameFromRGInfo() ;
   for ( var i = 0; i < rgHostArr.length; i++ ) {
      var hostName = rgHostArr[i] ;
      var isMatch  = false ;
      for ( var j = 0; j < this._hostInfoArr.length; j++ ) {
         if ( hostName == this._hostInfoArr[j].hostName ) {
            isMatch = true ;
            break ;
         }
      }
      if ( !isMatch ) {
         errMsg = "should offer information about host[" + hostName + "]" ; 
         errMsgArr.push( hostName ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      }
   }
   if ( exp != null ) {
      errMsg = "should offer information abount the follow hosts[" + 
         errMsgArr.toString() + "]" ;
      exp = new SdbError( SDB_INVALIDARG, errMsg ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   logger.log( PDEVENT, "finishing thirdly check" ) ;
} ;

ConfigMgr.prototype._init = function ConfigMgr__init() {
   var errMsg      = null ;
   var exp         = null ;

   if ( SYS_LINUX != SYS_TYPE ) {
      exp = new SdbError( SDB_SYS, 
         "not support current operating system[" + SYS_TYPE + "]" ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   
   // 1. check file exist or not
   if ( !File.exist( this._configFile ) ) {
      errMsg = "OM configure file[" + this._configFile + "] does not exist" ;
      exp = new SdbError( SDB_INVALIDARG, errMsg ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }

   // 2. get om configure info
   try {
      this._omConfObj = 
         eval( '(' + Oma.getOmaConfigs( this._configFile ) + ')' ) ;
      logger.log( PDEVENT, 
         "om config info is: " + JSON.stringify( this._omConfObj ) ) ;
   } catch( e ) {
      errMsg = 
         "failed to get om configure info from file[" + this._configFile + "]" ;
      exp = new SdbError( SDB_INVALIDARG, errMsg ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }

   // 3. get info for creating db obj
   var coordHostName  = this._getField( FIELD_COORD_HOST_NAME ) ;
   var coordSvcName   = this._getField( FIELD_COORD_PORT ) ;
   var dbAuthUser     = this._getField( FIELD_DB_AUTH_USER ) ;
   var dbAuthPassword = this._getField( FIELD_DB_AUTH_PASSWD ) ;
   
   try {
      this._sdb = new Sdb( coordHostName, coordSvcName, 
                           dbAuthUser, dbAuthPassword ) ;
   } catch( e ) {
      errMsg = "failed to connect to coord[" + 
         coordHostName + ":" + coordPort + "]" ;
      exp = new SdbError( e, errMsg ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }

   var cur    = null ;
   var record = null ;
   try {
      cur = this._sdb.listReplicaGroups() ;
   } catch ( e ) {
      var exp = new SdbError( e, "failed to get replica group's info" ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   while ( (record = cur.next()) != undefined ) {
      var obj = record.toObj() ;
      this._rgInfoArr.push( obj ) ;
   }
   
   // get cluster info
   this._clusterInfo = this._getClusterInfo() ;
   
} ;

ConfigMgr.prototype._doit = function ConfigMgr__doit() {
   this._init() ;

   // append host info and check
   this._appendHostName() ;
   this._appendRootInfo() ;
   this._appendSshPort()   ;
   this._appendInstallPath() ;
   this._appendAdminInfo() ;
   // checking
   this._firstlyCheck() ;
   this._appendRootSshObj() ;
   this._appendIP() ;
   this._appendRemoteInstalledInfo() ;
   // checking
   this._secondlyCheck() ; 
   // checking
   this._thirdlyCheck() ;
   
} ;

var RGInfoHelper = function( rgInfoArr ) {
   this._rgInfoArr = rgInfoArr ;
   this._init() ;
} ;

RGInfoHelper.prototype._init = function RGInfoHelper__init() {
   if ( !isArray(this._rgInfoArr) || this._rgInfoArr.length == 0 ) {
      var exp = new SdbError( SDB_SYS, 
         "no replica group's info in catalog, RGInfoHelper::_rgInfoArr is: " + 
         this._rgInfoArr ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
} ;

RGInfoHelper.prototype._getUsedNodeIDFromDB = 
   function RGInfoHelper__getUsedNodeIDFromDB() {
   var retArr  = [] ;
   var nodeArr = [] ;
   var rgArr   = [] ;
   var exp     = null ;
   var len     = this._rgInfoArr.length ;
   
   // get catalog and coord rg's info
   for ( var i = 0; i < len; i++ ) {
      var info = this._rgInfoArr[i] ;
      if ( info[GroupName] == "SYSCatalogGroup" || 
           info[GroupName] == "SYSCoord" ) {
         rgArr.push( info ) ;
      }
   }
   if ( rgArr.lenth == 0 ) {
      exp = new SdbError( SDB_SYS, "no catalog or coord group's info" ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   for ( var i = 0 ; i < rgArr.length; i++ ) {
      logger.log( PDDEBUG, JSON.stringify(rgArr[i]) ) ;
   }
   // get catalog or coord node id
   for ( var i = 0; i < rgArr.length; i++ ) {
      var rgInfo = rgArr[i] ;
      nodeArr = rgInfo[Group] ;
      if ( nodeArr == null ) {
         if ( rgInfo[GroupName] == "SYSCatalogGroup" ) {
            exp = new SdbError( SDB_SYS, "no node's info in catalog group" ) ;
            logger.log( PDERROR, exp ) ;
            throw exp ;      
         } else {
            // coord group may have no node's info, let's skip
            continue ;
         }
      }
      for ( var j = 0; j < nodeArr.length; j++ ) {
         var nodeID = nodeArr[j][FIELD_COORD_INFO_NODEID] ;
         if ( !isNumber(nodeID) ) {
            exp = new SdbError( SDB_SYS, 
               "get invalid node id[" + nodeID + 
               "] from group " + rgInfo[GroupName] ) ;
            logger.log( PDERROR, exp ) ;
            throw exp ;
         }
         retArr.push( nodeID ) ;
      }
   }

   logger.log( PDDEBUG, 
      "the used node id of coord or catalog group are as below: " + retArr ) ;
   return retArr ;
} ;

RGInfoHelper.prototype._genNodeID = function RGInfoHelper__genNodeID( num ) {
   var retArr        = [] ;
   var exp           = null ;
   var usedNodeIDArr = this._getUsedNodeIDFromDB() ;
   var id            = 1 ;
   var maxID         = 1000 ;

   for ( var i = 0, id = 1; i < num && id < maxID; i++ ) {
      var isMatch = false ;
      for ( var j = 0; j < usedNodeIDArr.length; j++ ) {
         if ( id == usedNodeIDArr[j] ) {
            isMatch = true ;
            break ;
         }
      }
      if ( !isMatch ) {
         retArr.push(id) ;
         usedNodeIDArr.push(id) ;
      } else {
         i-- ;
      }
      id++ ;
   }
   logger.log( PDDEBUG, 
      sprintf( "expect ? node id(s), actually get ?, " + 
               "the return nodeID array is: ?", 
               num, retArr.length, retArr.toString() ) ) ;
   if ( retArr.length != num ) {
      exp = new SdbError( "SDB_SYS", 
         "no invalid node id for the coord nodes" ) ;
      logger.log( PDERROR, exp ) ;
   }
   return retArr ;
} ;

RGInfoHelper.prototype._getNodeAddrFromDB = 
   function RGInfoHelper__getNodeAddrFromDB( role, type ) {
   var retArr   = [] ;
   var nodeArr  = null ;
   var exp      = null ;
   var rgInfo   = null ;
   var rgName   = null ;
   var portType = null ;

   if ( type == "local" ) portType = 0 ;
   else if ( type == "catalog" ) portType = 3 ;
   
   if ( role == "catalog" ) {
      rgName   = "SYSCatalogGroup" ;
   } else if ( role == "coord" ) {
      rgName   = "SYSCoord" ;
   } else {
      throw new SdbError( PDERROR, "invalid role: " + role ) ;
   }
   
   // get rg's info
   for ( var i = 0; i < this._rgInfoArr.length; i++ ) {
      var info = this._rgInfoArr[i] ;
      if ( rgName == info[GroupName] ) {
         rgInfo = info ;
         break ;
      }
   }
   if ( rgInfo == null ) {
      logger.log( PDWARNING, 
         role + " group's info does not exist in catalog" ) ;
      return retArr ;
   }
   // get node's info
   nodeArr = rgInfo[Group] ;
   if ( nodeArr == null ) {
      logger.log( PDWARNING, 
         role + " group does not contain any nodes" ) ;
      return retArr ;
   }
   for ( var i = 0; i < nodeArr.length; i++ ) {
      var node     = nodeArr[i] ;
      var hostName = node[HostName] ;
      var svcArr   = node[Service] ;
      var nodeInfo = null ;
      for ( var j = 0; j < svcArr.length; j++ ) {
         var svc  = svcArr[j] ;
         var type = svc[Type] ;
         if ( portType == type ) {
            nodeInfo = hostName + ":" + svc[Name] ;
            break ;
         }
      }
      if ( nodeInfo != null ) {
         retArr.push( nodeInfo ) ;
      }
   }
   logger.log( PDEVENT, sprintf( "group[?] has ? node(s) which type" +
                           " is [?] as below: ? ", role, retArr.length,
                           portType, retArr ) ) ;
   return retArr ;  
} ;

RGInfoHelper.prototype._getCataAddrFromDB = 
   function RGInfoHelper__getCataAddrFromDB( type ) {
   var exp         = null ;
   var cataAddrArr = this._getNodeAddrFromDB( "catalog", type ) ;
   if ( cataAddrArr == null || cataAddrArr.length == 0 ) {
      exp = new SdbError( SDB_SYS, 
         "can not get any info about catalog group from database" ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   logger.log( PDDEBUG, 
      "the catalog address with the type[" + type + "] is: " + cataAddrArr ) ;
   return cataAddrArr ;
} ;

RGInfoHelper.prototype._getCoordAddrFromDB = 
   function RGInfoHelper__getCoordAddrFromDB( type ) {
   var retArr = this._getNodeAddrFromDB( "coord", type ) ;
   logger.log( PDDEBUG, 
      "the coord address with the type[" + type + "] is: " + retArr ) ;
   return retArr ;
} ;

RGInfoHelper.prototype._getOriginalCoordRGInfo = 
   function RGInfoHelper__getOriginalCoordRGInfo() {
   var retObj = null ;
   // get coord rg
   for ( var i = 0; i < this._rgInfoArr.length; i++ ) {
      var info = this._rgInfoArr[i] ;
      if ( "SYSCoord" == info[GroupName] ) {
         retObj = info ;
         break ;
      }
   }
   if ( retObj == null ) {
      logger.log( PDWARNING, "database has no coord group" ) ;
   }
   return retObj ;
} ;

RGInfoHelper.prototype._getOriginalCoordNodes = 
   function RGInfoHelper__getOriginalCoordRGNodes() {
   var retArr     = [] ;
   var coordRGObj = this._getOriginalCoordRGInfo() ;
   
   if ( coordRGObj == null || coordRGObj[Group] == null ) {
      logger.log( PDWARNING, "there has no any coord nodes" ) ;
      return retArr ;
   }
   var nodeArr = coordRGObj[Group] ;
   for ( var i = 0; i < nodeArr.length; i++ ) {
      retArr.push( nodeArr[i] ) ;
   }
   return retArr ;
} ;

// TODO: maybe "FilterCoordInfo" can combined with "UpdateCoordInfo"
var FilterCoordInfo = function( hostInfoArr, rgInfoHelper ) {
   this._hostInfoArr  = hostInfoArr ;
   this._rgInfoHelper = rgInfoHelper ;
} ;

FilterCoordInfo.prototype._collectCoordInfoFromHost = 
   function FilterCoordInfo__collectCoordInfoFromHost() {
   var retArr = [] ;
   var exp    = null ;
   var str1   = "\' var arr = Sdbtool.listNodes({type:\"db\", role:\"coord\", mode:\"local\", expand:true}); \'" ;
   var str2   = "\' arr.next(); \'" ;
   var str3   = "\' quit \'" ;

   for ( var i = 0; i < this._hostInfoArr.length; i++ ) {
      var host        = this._hostInfoArr[i] ;
      var ssh         = host.rootSshObj ;
      var sdbExecFile = adaptPath(host.installPath) + OMA_PROG_BIN_SDB ;
      var cmd1        = sdbExecFile + " -s " + str1 ;
      var cmd2        = sdbExecFile + " -s " + str2 ;
      var cmd3        = sdbExecFile + " -s " + str3 ;
      var retStr      = null ;
      logger.log( PDDEBUG, 
         "cmd to get coord info at host[" + host.hostName + "] is: " ) ;
      logger.log( PDDEBUG, cmd1 ) ;
      // get coord node's info from remote host
      try {
         ssh.exec( cmd1 ) ;
      } catch( e ) {
         exp = new SdbError( PDERROR, "failed to execute command[" + cmd1 + 
            "] in host[" + host.hostName + "], for " + ssh.getLastOut() + 
            ", errno: " + ssh.getLastRet() ) ;
         logger.log( PDERROR, exp ) ;
         throw exp ;
      } finally {
         if ( exp != null ) {
            try { ssh.exec( cmd3 ); } catch(e) {}
         }
      }
      // extract the info back to local
      while( true ) {
         var obj = null ;
         try {
            retStr = ssh.exec( cmd2 ) ;
         } catch( e ) {
            exp = new SdbError( PDERROR, "failed to execute command[" + cmd2 + 
               "] in host[" + host.hostName + "], for " + ssh.getLastOut() + 
               ", errno: " + ssh.getLastRet() ) ;
            logger.log( PDERROR, exp ) ;
            throw exp ;
         } finally {
            if ( exp != null ) {
               try { ssh.exec( cmd3 ); } catch(e) {}
            }
         }
         try {
            obj = eval( '(' + retStr + ')' ) ;
            if ( !isObject(obj) ) {
               logger.log( PDWARNING, 
                  "the string[" + retStr + "] return by executing command[" + 
                  cmd2 + "] in host [" + host.hostName + 
                  "] can not be eval to an object"  ) ;
               continue ;
            }
         } catch(e) {
            // TODO: lost of error msg here, find out the reason
            logger.log( PDWARNING, "we get exception[" + e + 
               "], we take it end of traversal cursor" ) ;
            try { ssh.exec( cmd3 ); } catch(e) {}
            break ;
         }
         obj[FIELD_CONF_HOST_NAME] = host.hostName ;
         obj[FIELD_CONF_IP]        = host.ip ;
         retArr.push( obj ) ;
      }
   }
   logger.log( PDDEBUG, 
      "the number of collecting coord's info from host is: " + retArr.length + 
      ", they are as below: " ) ;
   for ( var i = 0; i < retArr.length; i++ ) {
       logger.log( PDDEBUG, JSON.stringify(retArr[i]) ) ;
   }
   return retArr ;
} ;

FilterCoordInfo.prototype._extractCoordInfo = 
   function FilterCoordInfo__extractCoordInfo( coordInfoObjArr ) {
   var retArr = [] ;
   var len    = coordInfoObjArr.length ;

   for ( var i = 0; i < len; i++ ) {
      var wapper          = new CoordInfoWrapper() ;
      var obj             = new CoordInfo() ;
      var coordInfoObj    = coordInfoObjArr[i] ;
      obj.HostName        = coordInfoObj[FIELD_CONF_HOST_NAME] ;
      obj.dbpath          = coordInfoObj[FIELD_CONF_DBPATH] ;
      obj.Service[0].Name = coordInfoObj[FIELD_CONF_SVC_NAME] ;
      obj.Service[1].Name = coordInfoObj[FIELD_CONF_REPL_NAME] ;
      obj.Service[2].Name = coordInfoObj[FIELD_CONF_CATALOG_NAME] ;
      wapper.ip           = coordInfoObj[FIELD_CONF_IP] ;
      wapper.catalogAddr  = coordInfoObj[FIELD_CONF_CATALOG_ADDR] ;
      wapper.infoObj      = obj ;
      retArr.push( wapper ) ;
   }
   logger.log( PDDEBUG, "the number of extract coord info is: " + 
      retArr.length + ", they are as below: " ) ;
   for ( var i = 0 ; i < retArr.length; i++ ) {
      logger.log( PDDEBUG, JSON.stringify(retArr[i]) ) ;
   }
   return retArr ;
} ;

FilterCoordInfo.prototype._filterCoordInfo = 
   function FilterCoordInfo__filterCoordInfo( hostCoordInfoWrapperArr, 
                                           dbCoordAddrArr, 
                                           dbCatalogAddrArr ) {
   var exp           = null ;
   var retArr        = [] ;

   if ( !isArray(dbCatalogAddrArr) || dbCatalogAddrArr.length == 0 ) {
      exp = new SdbError( SDB_SYS, 
         "no catalog address for comparing, dbCatalogAddrArr is: " + 
         dbCatalogAddrArr ) ;
      logger.log( SDB_SYS, exp ) ;
      throw exp ;
   }
   for ( var i = 0; i < hostCoordInfoWrapperArr.length; i++ ) {
      var hostCoordInfo = hostCoordInfoWrapperArr[i].infoObj ;
      var hostName     = hostCoordInfo[FIELD_COORD_INFO_HOSTNAME] ;
      var svcName      = null ;
      var needRestart  = false ;
      for ( var j = 0; j < 2; j++ ) {
         var type = 
            hostCoordInfo[FIELD_COORD_INFO_SERVICE][j][FIELD_COORD_INFO_TYPE] ;
         if ( type == 0 ) {
            svcName = 
               hostCoordInfo[FIELD_COORD_INFO_SERVICE][j][FIELD_COORD_INFO_NAME] ;
            break ;
         }
      }
      var hostCoordAddr = hostName + ":" + svcName ;
      // filter the coord which info has been in catalog
      for ( var j = 0; j < dbCoordAddrArr.length; j++ ) {
         var dbCoordAddr = dbCoordAddrArr[j] ;
         if ( hostCoordAddr == dbCoordAddr ) {
            needRestart = true ;
            break ;
         }
      }
      if ( needRestart ) continue ;
      // filter the coord which is not below to the specified catalog
      var hostCataAddrArr = 
         hostCoordInfoWrapperArr[i].catalogAddr.split( ',' ) ;
      if ( !isArray(hostCataAddrArr) || hostCataAddrArr.length == 0 ) {
         logger.log( PDWARNING, "no catalog info in coord node[" +
            hostCoordAddr + "], hostCataAddrArr is: " + hostCataAddrArr ) ;
         continue ;
      }
      for ( var m = 0; m < hostCataAddrArr.length; m++ ) {
         var hostCataAddr = hostCataAddrArr[m] ;
         var isMatch      = false ;
         for ( var n = 0; n < dbCatalogAddrArr.length; n++ ) {
            var dbCatalogAddr = dbCatalogAddrArr[n] ;
            if ( hostCataAddr == dbCatalogAddr ) {
               isMatch = true ;
               break ;
            }
         }
         if ( isMatch ) {
            retArr.push( hostCoordInfo ) ;
            break ;
         }
      }
   }
   logger.log( PDDEBUG, 
      "the number of matched coord for inserting into catalog is: " + 
      retArr.length + ", they are as below: " ) ;
   for ( var i = 0; i < retArr.length; i++ ) {
      logger.log( PDDEBUG, JSON.stringify(retArr[i]) ) ;
   }
   return retArr ;
} ;

FilterCoordInfo.prototype._appendNodeID =
   function FilterCoordInfo__appendNodeID( matchedCoordInfoArr ) {
   var retArr    = [] ;
   var nodeIDArr = [] ;
   // get usable node id
   nodeIDArr = this._rgInfoHelper._genNodeID( matchedCoordInfoArr.length ) ;
   // generate inserted records
   for ( var i = 0; i < matchedCoordInfoArr.length; i++ ) {
      var obj                      = matchedCoordInfoArr[i] ;
      obj[FIELD_COORD_INFO_NODEID] = nodeIDArr[i] ;
      retArr.push( obj ) ;
   }
   logger.log( PDDEBUG, "the number of inserted coord info is " + retArr.length + 
      ", the info are as below: " ) ;
   for ( var i = 0; i < retArr.length; i++ ) {
      logger.log( PDDEBUG, JSON.stringify(retArr[i]) ) ;
   }
   return retArr ;
} ;

FilterCoordInfo.prototype._init = function FilterCoordInfo__init() {
} ;

FilterCoordInfo.prototype._doit = 
   function FilterCoordInfo__doit() {
   var retArr = [] ;

   this._init() ;

   // ssh to remote to get all the coord's info to local
   var coordInfoObjArr = this._collectCoordInfoFromHost() ;
   // extract some useful info of the coord to an array
   var hostCoordInfoArr = this._extractCoordInfo( coordInfoObjArr ) ;
   // get existing coord's info from database
   var dbCoordAddrArr = this._rgInfoHelper._getCoordAddrFromDB( "local" ) ;
   // get catalog's info from database
   var dbCatalogAddrArr = this._rgInfoHelper._getCataAddrFromDB( "catalog" ) ;
   // pick up the coord info which should be in current cluster but 
   // have not been included in
   var filteredCoordInfoArr = 
      this._filterCoordInfo( hostCoordInfoArr, dbCoordAddrArr, dbCatalogAddrArr ) ;
   // append node id for those coords
   var retArr = this._appendNodeID( filteredCoordInfoArr ) ;
   
   return retArr ;
} ;

var UpdateCoordInfo = function( sdb, rgInfoHelper, coordInfoArr ) {
   this._sdb          = sdb ;
   this._rgInfoHelper = rgInfoHelper ;
   this._infoArr      = coordInfoArr ;
   this._originalCoordRGInfo = null ;
} ;

UpdateCoordInfo.prototype._closeConnections = 
   function UpdateCoordInfo__closeConnections( connArr ) {
   for ( var i = 0; i < connArr.length; i++ ) {
      try { connArr[i].close() ; } catch( e ) {}
   }
} ;

UpdateCoordInfo.prototype._buildUpdateRule = 
   function UpdateCoordInfo__buildUpdateRule( type ) {
   var retObj = null ;
   try {
      var recordObj          = new Object() ;
      var subObj             = new Object() ;
      if ( type == "rebuild" ) {
         subObj[Group]       = this._addExistedCoordInfo() ; ;
         recordObj["$set"]   = subObj ;
         retObj              = recordObj ;
      } else if ( type == "rollback" ) {
         if ( this._originalCoordRGInfo != null ) {
            recordObj["$set"] = this._originalCoordRGInfo ;
            retObj            = recordObj ;
         } else {
            // in this case, that means the original coord rg does not exist
            // so we don't need to build update rule
            retObj            = null ;
         }
      }
   } catch( e ) {
      var exp = new SdbError( e, 
         sprintf( "failed to build update coord rule with the type \'?\'",
            type ) ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   logger.log( PDEVENT, sprintf( "the update rule for [?] is: ", 
      type ) + JSON.stringify( retObj ) ) ;
   return retObj ;
} ;

UpdateCoordInfo.prototype._buildCond =
   function UpdateCoordInfo__buildCond() {
   var retObj = null ;
   var exp    = null ;
   var obj    = new Object() ;
   var subObj = new Object() ;
   subObj["$et"]  = "SYSCoord" ;
   obj[GroupName] = subObj ;
   retObj = obj ;
   return retObj ;
} ;

UpdateCoordInfo.prototype._buildUpdateCond =
   function UpdateCoordInfo__buildUpdateCond() {
   var retObj = null ;
   try {
      retObj = this._buildCond() ;
   } catch( e ) {
      var exp = new SdbError( e,
         "failed to build update coord info's condition" ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   logger.log( PDEVENT, "update coord info's condition is: " + 
      JSON.stringify( retObj ) ) ;
   return retObj ;
} ;

UpdateCoordInfo.prototype._buildQueryCond =
   function UpdateCoordInfo__buildQueryCond() {
   var retObj = null ;
   try {
      retObj = this._buildCond() ;
   } catch( e ) {
      var exp = new SdbError( e,
         "failed to build query coord info's condition" ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   logger.log( PDEVENT, "query coord info's condition is: " + 
      JSON.stringify( retObj ) ) ;
   return retObj ;
} ;

UpdateCoordInfo.prototype._addExistedCoordInfo = 
   function UpdateCoordInfo__addExistedCoordInfo() {
   var retArr             = this._infoArr ;
   var originalCoordNodes = this._rgInfoHelper._getOriginalCoordNodes() ;
   logger.log( PDDEBUG, 
      "the original coord info is: " + JSON.stringify(originalCoordNodes) ) ;
   logger.log( PDDEBUG, 
      "the new coord info is: " + JSON.stringify(this._infoArr) ) ;
   for ( var i = 0; i < originalCoordNodes.length; i++ ) {
      retArr.push( originalCoordNodes[i] ) ;
   }
   logger.log( PDDEBUG, 
      "the total coord info is: " + JSON.stringify(retArr) ) ;
   return retArr ;
} ;

UpdateCoordInfo.prototype._rollback = 
      function UpdateCoordInfo__rollback( rollbackCLArr, rule, cond ) {
   var exp       = null ;
   var errMsg    = null ;
   var errMsgArr = [] ;

   for ( var i = 0; i < rollbackCLArr.length; i++ ) {
      var cl = null ;
      try {
         cl = rollbackCLArr[i] ;
         logger.log( PDEVENT, 
            sprintf( "begin to restore coord info to table[?]", cl ) ) ;
         if ( rule != null ) {
            cl.update( rule, cond ) ;
         } else {
            // in this case, "SYSCoord" does not exist in catalog group
            // so, let's remove the inserted coord info
            cl.remove( cond ) ; 
         }
         logger.log( PDEVENT, 
            sprintf( "succeed to restore coord info to table[?]", cl ) ) ;
      } catch( e ) {
         exp = new SdbError( e, 
            sprintf( "failed to restore the original coord info in [?] with " + 
               "the rule [?] and condition [?]", cl, 
               JSON.stringify( rule ), JSON.stringify( cond ) ) ) ;
         errMsg = exp.toString() ;
         errMsgArr.push( errMsg ) ;
         logger.log( PDERROR, errMsg ) ;
         println( errMsg ) ;
      }
   }
} ;

UpdateCoordInfo.prototype._prepareCoordRG = 
   function UpdateCoordInfo__prepareCoordRG() {
   var num = null ;
   var exp = null ;

   try {
      var cur = this._sdb.list( SDB_LIST_GROUPS, this._buildQueryCond() ) ;
      num     = cur.size() ;
   } catch( e ) {
      exp = new SdbError( e, 
         "failed to check whether coord group existed or not" ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   if ( num == 0 ) {
      logger.log( PDEVENT, 
         "there is no coord group in current cluster, let's build one" ) ;
      try {
         this._sdb.createRG( "SYSCoord" ) ;
      } catch ( e ) {
         exp = new SdbError( e, "failed to build coord group" ) ;
         logger.log( PDERROR, exp ) ;
         throw exp ;
      }
      logger.log( PDEVENT, "finish building coord group" ) ;
   } else if ( num != 1 ) {
      exp = SdbError( SDB_SYS, "invalid amount of coord group: " + num ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
} ;

UpdateCoordInfo.prototype._updateCoordInfo = 
   function UpdateCoordInfo__updateCoordInfo() {
   var exp           = null ;
   var cdbArr        = [] ;
   var clArr         = [] ;
   var rollbackCLArr = [] ;

   var cataAddrArr = this._rgInfoHelper._getCataAddrFromDB( "local" ) ;
   var rule1       = this._buildUpdateRule( "rebuild" ) ;
   var rule2       = this._buildUpdateRule( "rollback" ) ;
   var cond        = this._buildUpdateCond() ;

   for ( var i = 0; i < cataAddrArr.length; i++ ) {
      try {
         var cdb = new Sdb( cataAddrArr[i] ) ;
         cdbArr.push( cdb ) ;
         var cl  = cdb.getCS( "SYSCAT" ).getCL( "SYSNODES" ) ;
         clArr.push( cl ) ;
      } catch( e ) {
         exp = new SdbError( e, 
            "failed to get system table 'SYSCAT.SYSNODES' from catalog[" + 
            cataAddrArr[i] + "]" ) ;
         logger.log( PDERROR, exp ) ;
         // disconnect
         this._closeConnections( cdbArr ) ;
         throw exp ;
      }
   }
   for ( var i = 0; i < clArr.length; i++ ) {   
      var cl = clArr[i] ;
      // update
      try {
         logger.log( PDEVENT, 
            sprintf( "begin to update coord info to catalog[?]", 
               cataAddrArr[i] ) ) ;
         cl.update( rule1, cond ) ;
         logger.log( PDEVENT, 
            sprintf( "succeed to update coord info to catalog[?]", 
               cataAddrArr[i] ) ) ;
      } catch( e ) {
         exp = new SdbError( e, 
            "failed to update coord info to table 'SYSCAT.SYSNODES' " + 
            "in catalog[" + cataAddrArr[i] + "]" ) ;
         logger.log( PDERROR, exp ) ;
         rollbackCLArr.push( cl ) ;
         // rollback
         this._rollback( rollbackCLArr, rule2, cond ) ;
         // disconnect
         this._closeConnections( cdbArr ) ;
         throw exp ;
      }
      rollbackCLArr.push( cl ) ;
   }
   // disconnect
   this._closeConnections( cdbArr ) ;
   
} ;

UpdateCoordInfo.prototype._init = function UpdateCoordInfo__doit() {
   if ( !isArray(this._infoArr) ) {
      var exp = new SdbError( SDB_SYS, 
         sprintf( "the coord info array[?] for updating is invalid", 
            this._infoArr ) ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
   this._originalCoordRGInfo = this._rgInfoHelper._getOriginalCoordRGInfo()
   // should set the log level to event,
   // we want the original coord group info is recorded in log
   logger.log( PDEVENT, 
      "the original coord group info is: " + 
      JSON.stringify( this._originalCoordRGInfo ) ) ;
} ;

UpdateCoordInfo.prototype._doit = function UpdateCoordInfo__doit() {
   // init
   this._init() ;
   if ( this._infoArr.length == 0 ) {
      logger.log( PDEVENT, "no new coord info for updating" ) ;
      return ;
   } else {
      logger.log( PDEVENT, "the new coord info for updating is: " + 
         JSON.stringify( this._infoArr ) ) ;
   }
   this._prepareCoordRG() ;
   this._updateCoordInfo() ;
} ;


function main() {
   var errMsg      = null ;
   var debugMsg    = null ;
   var exp         = null ;

   // extract config info from config file and 
   // check whether the offered info is ok
   var configMgr = new ConfigMgr( OM_CONF_FILE ) ;
   configMgr._doit() ;
   
   ///*
   var hostInfoArr  = configMgr._getHostInfoArr() ;
   var rgInfoArr    = configMgr._getRGInfoArr() ;
   var sdb          = configMgr._getSdbObj() ;
   var rgInfoHelper = new RGInfoHelper( rgInfoArr ) ;

   var filterCoordInfo = new FilterCoordInfo( hostInfoArr, rgInfoHelper ) ;
   var updateCoordInfoArr = filterCoordInfo._doit() ;
   var updateCoord = new UpdateCoordInfo( sdb, rgInfoHelper, updateCoordInfoArr ) ;
   updateCoord._doit() ;
   //*/

   // get the info about the host in current cluster
   // those info will be saved in "SYSDEPLOY.SYSHOST" table in OM
   
   
   
}




// execute
try {
   _init() ;
   main() ;
} catch( e ) {
   logger.log( PDERROR, "failed to rebuild om: " + e ) ;
   throw e ;
} finally {
   _final() ;
}

