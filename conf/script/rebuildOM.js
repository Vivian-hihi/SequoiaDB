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

var FILE_NAME_REBUILD_OM = "rebuildOM.js" ;
var logger = new Logger(FILE_NAME_REBUILD_OM) ;

var FIELD_HOST_NAME            = "host_name" ;
var FIELD_DEFAULT_ROOT_USER    = "default_root_user" ;
var FIELD_DEFAULT_ROOT_PASSWD  = "default_root_passwd" ;
var FIELD_DEFAULT_SSH_PORT     = "default_ssh_port" ;
var FIELD_COORD_HOST_NAME      = "coord_host_name" ;
var FIELD_COORD_PORT           = "coord_port" ;
var FIELD_DB_AUTH_USER         = "db_auth_user" ;
var FIELD_DB_AUTH_PASSWD       = "db_auth_passwd" ;
var FIELD_CLUSTER_NAME         = "cluster_name" ;
var FIELD_CLUSTER_DESCRIPTION  = "cluster_description" ;
var FIELD_SDB_USER_NAME        = "sdb_user_name" ;
var FIELD_SDB_PASSWORD         = "sdb_password" ;
var FIELD_SDB_USER_GROUP_NAME  = "sdb_user_group_name" ;
var FIELD_SDB_INSTALL_PATH     = "sdb_install_path" ;

var FIELD_ROOT_USER            = "_root_user" ;
var FIELD_ROOT_PASSWD          = "_root_passwd" ;
var FIELD_SSH_PORT             = "_ssh_port" ;
var FIELD_INSTALL_PATH         = "_install_path" ;

var ts                         = null ;

var ClusterInfo = function() {
   // cluster common info
   this.clusterName         = null ;
	this.clusterDescription  = null ;
   this.sdbUserName         = null ;
   this.sdbPassword         = null ;
   this.sdbUserGroupName    = null ;
	this.sdbInstallPath      = null ;

	ClusterInfo.prototype.toString = function() {
		return "cluster info[ name: " + this.clusterName + 
			", description: " + this.clusterDescription + 
			", sdb user name: " + this.sdbUserName + 
			", sdb password: " + this.sdbPassword +
			", sdb user group: " + this.sdbUserGroupName +
			", sdb install path: " + this.sdbInstallPath ;
	}
}

function isIP( strIP ) {
   if ( strIP == undefined ) return false ;
   var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/g ;
   if( re.test(strIP) ) {
      if ( RegExp.$1 < 256 && RegExp.$2 < 256 && 
           RegExp.$3 < 256 && RegExp.$4 < 256 ) {
           return true;
      }
   }
   return false;
}

function removeQuotes( inputStr ) {
   var exp = null ;
   if ( !isString( inputStr ) ) {
      exp = new SdbError( SDB_INVALIDARG, 
         "the input argument[" + inputStr + "] is not a string" ) ;
      logger.log( PDERROR, err ) ;
      throw err ;
   }
   var len = inputStr.length ;
   var outputStr = '' ;
   for ( var i = 0; i < len; i++ ) {
      var c = inputStr.charAt( i ) ;
      if ( c != '\'' && c != '\"' ) {
         outputStr += c ;
      }
   }
   return outputStr ;
}

var HostInfo = function() {
   // host info
   // TODO: merge "address" and "hostName"
   this.address             = null ;
   this.hostName            = null ;   
   this.ip                  = null ;
   this.rootUserName        = null ;
   this.rootPassword        = null ;
   this.sshPort             = null ;
   this.installPath         = null ;

   // root ssh obj
   this.rootSshObj          = null ;
	
   // sdb account info
   this.sdbUserName         = null ;
   this.sdbPassword         = null ;
   this.sdbUserGroupName    = null ;
   
   // remote installed info
   this.installedInfo       = null ;
} ;

HostInfo.prototype.toString = function() {
    return "host info[ address: " + this.address + 
      ", hostName: " + this.hostName + 
      ", ip: " + this.ip + "]" +
	 	", rootUserName: " + this.rootUserName +
      ", rootPassword: " + this.rootPassword + 
      ", sshPort: " + this.sshPort + 
      ", installPath: " + this.installPath ;
} ;

function _init()
{
   logger.log( PDEVENT, "Begin to rebuild om" ) ;
   ts = genTimeStamp() ;
}

function _final()
{
   logger.log( PDEVENT, "Finish rebuilding om" ) ;
}

function _getFiled( omConfObj, fieldName ) {
   var ret    = null ;
   var errMsg = null ;
   var err    = null ;
   var field  = omConfObj[fieldName] ;
   if ( "" == field || null == field || "undefined" == typeof(field) ) {
      errMsg = "can not get value according to field name [" + fieldName + "]" ;
      err = new SdbError( SDB_INVALIDARG, errMsg ) ;
      logger.log( PDERROR, err ) ;
      throw err ;
   }
   ret = removeQuotes( field ) ;
   return ret ;
}

function _getClusterInfo( omConfObj ) {
	var retObj = new ClusterInfo() ;
	retObj.clusterName         = _getFiled( omConfObj, FIELD_CLUSTER_NAME ) ;
	retObj.clusterDescription  = _getFiled( omConfObj, FIELD_CLUSTER_DESCRIPTION ) ;
   retObj.sdbUserName         = _getFiled( omConfObj, FIELD_SDB_USER_NAME ) ;
   retObj.sdbPassword         = _getFiled( omConfObj, FIELD_SDB_PASSWORD ) ;
   retObj.sdbUserGroupName    = _getFiled( omConfObj, FIELD_SDB_USER_GROUP_NAME ) ;
	retObj.sdbInstallPath      = _getFiled( omConfObj, FIELD_SDB_INSTALL_PATH ) ;
	return retObj ;
}

function _appendHostName( omConfObj, hostInfoArr ) {
   var value   = _getFiled( omConfObj, FIELD_HOST_NAME ) ;
   var nameArr = value.split( ',' ) ;
   var len     = nameArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo = new HostInfo() ;
      hostInfo.address  = strTrim( nameArr[i] ) ;
		hostInfoArr.push( hostInfo ) ;
   }	
}

function _appendRootInfo( omConfObj, hostInfoArr ) {
   var defaultRootUserName  = _getFiled( omConfObj, FIELD_DEFAULT_ROOT_USER ) ;
   var defaultRootPassword  = _getFiled( omConfObj, FIELD_DEFAULT_ROOT_PASSWD ) ;
   var len = hostInfoArr.length ;
   for ( var i = 0; i < len; i++ ) {
		var hostInfo      = hostInfoArr[i] ;
      var rootUserName  = null ;
      var rootPassword  = null ;
      var field         = null ;
      var value         = null ;
      
      // get specific root info
      field          = hostInfo.address + FIELD_ROOT_USER ;
      value          = omConfObj[field] ;
      rootUserName   = ( value == undefined ) ? defaultRootUserName : 
         removeQuotes( value ) ;
      field          = hostInfo.address + FIELD_ROOT_PASSWD ;
      value          = omConfObj[field] ;
      rootPassword   = ( value == undefined ) ? defaultRootPassword : 
         removeQuotes( value ) ;
      // append root info
      hostInfo.rootUserName  = rootUserName ;
      hostInfo.rootPassword  = rootPassword ;
   }
}

function _appendSshPort( omConfObj, hostInfoArr ) {
   var value          = _getFiled( omConfObj, FIELD_DEFAULT_SSH_PORT ) ;
   var defaultSshPort = parseInt( value ) ;
   var len            = hostInfoArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo = hostInfoArr[i] ;
      var sshPort     = null ;
      var field       = null ;
      var value       = null ;
      // get specific ssh port
      field        = hostInfo.address + FIELD_SSH_PORT ;
      value        = omConfObj[field] ;
      sshPort      = ( value == undefined ) ? defaultSshPort : 
         parseInt( removeQuotes( value ) ) ;
		// append ssh info
      hostInfo.sshPort = sshPort ;
   }
}

function _appendInstallPath( omConfObj, hostInfoArr ) {
   var sdbInstallPath = _getFiled( omConfObj, FIELD_SDB_INSTALL_PATH ) ;
   var len            = hostInfoArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo = hostInfoArr[i] ;
		var installPath = null ;
      var field       = null ;
      var value       = null ;
      // get specific install path
      field        = hostInfo.address + FIELD_INSTALL_PATH ;
      value        = omConfObj[field] ;
      installPath  = ( value == undefined ) ? sdbInstallPath : 
         removeQuotes( value ) ;
      // append install path
      hostInfo.installPath = installPath ;
   }
}

function _appendAdminInfo( omConfObj, hostInfoArr ) {
   var sdbUserName       = _getFiled( omConfObj, FIELD_SDB_USER_NAME ) ;
   var sdbPassword       = _getFiled( omConfObj, FIELD_SDB_PASSWORD ) ;
   var sdbUserGroupName  = _getFiled( omConfObj, FIELD_SDB_USER_GROUP_NAME ) ;
   var len               = hostInfoArr.length ;
   for ( var i = 0; i < len; i++ ) {
      var hostInfo = hostInfoArr[i] ;
		// append admin info
      hostInfo.sdbUserName      = sdbUserName ;
      hostInfo.sdbPassword      = sdbPassword ;      
      hostInfo.sdbUserGroupName = sdbUserGroupName ;
   }
}

function _checkSsh( hostInfoArr, account ) {
	var errMsg    = null ;
	var errMsgArr = null ;
	var exp       = null ;
	var address   = null ;
	var user      = null ;
 	var passwd    = null ;
 	var sshPort   = null ;
   for ( var i = 0; i < hostInfoArr.length; i++ ) {
		address = hostInfoArr[i].address ;
		sshPort = hostInfoArr[i].sshPort ;
		if ( account == "root" ) {
			user   = hostInfoArr[i].rootUserName ;
	 		passwd = hostInfoArr[i].rootPassword ;
		} else {
			user   = hostInfoArr[i].sdbUserName ;
	 		passwd = hostInfoArr[i].sdbPassword ;
		}
	 	
      try {
         var ssh = new Ssh( address, user, passwd, sshPort ) ;
      } catch( e ) {
         errMsg = "can not use account[" + user + 
				"] to ssh to host[" + address + "], " + GETLASTERRMSG() ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
      }
   }
   if ( exp != null ) {
      throw new SdbError( SDB_INVALIDARG, errMsgArr.toString() ) ;
   }
}

function _firstlyCheck( hostInfoArr ) {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
	var address   = null ;
	logger.log( PDEVENT, "begin to firstly check" ) ;
	// 1. host is ip or not
   for ( var i = 0; i < hostInfoArr.length; i++ ) {
		address = hostInfoArr[i].address ;
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
   for ( var i = 0; i < hostInfoArr.length; i++ ) {
		address = hostInfoArr[i].address ;
      var result = eval( '(' + System.ping( address ) + ')' ) ;
      if ( result[Reachable] != true ) {
         errMsg = "host [" + address + 
            "] is unreachable, " + GETLASTERRMSG() ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
      }
   }
   if ( exp != null ) {
      throw new SdbError( SDB_INVALIDARG, errMsgArr.toString() ) ;
   }

	// 3. root account can ssh or not
	_checkSsh( hostInfoArr, "root" ) ;

	// 4. admin account can ssh or not
	_checkSsh( hostInfoArr, "admin" ) ;
   logger.log( PDEVENT, "finishing firstly check" ) ;
}

function _appendRootSshObj( hostInfoArr ) {
	var errMsg  = null ;
	var exp     = null ;
	var address = null ;
   for ( var i = 0; i < hostInfoArr.length; i++ ) {
		address = hostInfoArr[i].address ;
      try {
         var ssh = new Ssh( address, hostInfoArr[i].rootUserName, 
         	hostInfoArr[i].rootPassword, hostInfoArr[i].sshPort ) ;
			hostInfoArr[i].rootSshObj = ssh ;
      } catch( e ) {
         errMsg = "can not use root account to ssh to host[" + address + 
				"], " + GETLASTERRMSG() ;
			exp = new SdbError( GETLASTERROR(), errMsg )
         logger.log( PDERROR, exp ) ;
    		throw exp ;	
      }
   }
}

function _appendIP( hostInfoArr ) {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
   for ( var i = 0; i < hostInfoArr.length; i++ ) {
		var address = hostInfoArr[i].address ;
      var ssh = hostInfoArr[i].rootSshObj ;
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
         hostInfoArr[i].hostName = address ;
         hostInfoArr[i].ip       = removeLineBreak( ip ) ;
      } catch( e ) {
         errMsg = "failed to get host[" + address + "]'ip, " + GETLASTERRMSG() ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
      }
   }
   if ( exp != null ) {
      throw new SdbError( SDB_SYS, errMsgArr.toString() ) ;
   }
}

function _appendRemoteInstalledInfo( hostInfoArr ) {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
   
   for ( var i = 0; i < hostInfoArr.length; i++ ) {
      var infoObj        = new Object() ;
      var ssh            = hostInfoArr[i].rootSshObj ;
      var address        = hostInfoArr[i].address ;
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
            "] from host[" + address + "] to local, " + GETLASTERRMSG() ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      }
      try {
         infoObj = eval( '(' + Oma.getOmaConfigs(localFileName) + ')' ) ;
      } catch( e ) {
         errMsg = "failed to extract installed info from file[" + localFileName + 
            "], " + GETLASTERRMSG() ;
         errMsgArr.push( errMsg ) ;
         if ( exp == null ) exp = new SdbError( SDB_INVALIDARG ) ;
         logger.log( PDERROR, errMsg ) ;
         continue ;
      } finally {
         try { File.remove(localFileName) ; } catch(e) {}
      }
      hostInfoArr[i].installedInfo = infoObj ;
   }
   if ( exp != null ) {
      throw new SdbError( SDB_SYS, errMsgArr.toString() ) ;
   }
}

function _secondlyCheck( hostInfoArr ) {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
	var address   = null ;
	logger.log( PDEVENT, "begin to secondly check" ) ;
	// 1. check the specified admin accounts are the same with 
	// the one in remote or not
   for ( var i = 0; i < hostInfoArr.length; i++ ) {
		var host        = hostInfoArr[i] ;
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
   for ( var i = 0; i < hostInfoArr.length; i++ ) {
		var host = hostInfoArr[i] ;
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
}

function _connectToDB( omConfObj ) {
   var errMsg = null ;
   var exp    = null ;
	var sdb    = null ;

   var coordHostName = _getFiled( omConfObj, FIELD_COORD_HOST_NAME ) ;
   var coordPort     = _getFiled( omConfObj, FIELD_COORD_PORT ) ;
   var dbAuthUser    = _getFiled( omConfObj, FIELD_DB_AUTH_USER ) ;
   var dbAuthPasswd  = _getFiled( omConfObj, FIELD_DB_AUTH_PASSWD ) ;   

   try {
      sdb = new Sdb( coordHostName, coordPort, dbAuthUser, dbAuthPasswd ) ;
   } catch( e ) {
      errMsg = "failed to connect to coord[" + 
         coordHostName + ":" + coordPort + "], " + GETLASTERRMSG() ;
      exp = new SdbError( GETLASTERROR(), errMsg ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }
	return sdb ;
}

function _getRGInfo( sdb ) {
   var errMsg = null ;
   var exp    = null ;
	var retArr = [] ;
	var cur    = null ;
	var record = null ;
	try {
		cur = sdb.listReplicaGroups() ;
	} catch ( e ) {
		errMsg = "failed to get repplica group's info, " + GETLASTERRMSG() ;
		exp    = new SdbError( GETLASTERROR(), errMsg ) ;
		logger.log( PDERROR, exp ) ;
		throw exp ;
	}
	while ( (record = cur.next()) != undefined ) {
		var obj = record.toObj() ;
		retArr.push( obj ) ;
	}
	return retArr ;
}

function _getHostNameFromRGInfo( rgInfoArr ) {
   var errMsg       = null ;
   var exp          = null ;
	var retArr       = [] ;

	for ( var i = 0; i < rgInfoArr.length; i++ ) {
		var obj      = rgInfoArr[i] ;
		var arr      = obj[Group] ;
		for ( var j = 0; j < arr.length; j++ ) {
			var subObj   = arr[j] ;
			var hostName = subObj[HostName] ;
			retArr.push( hostName ) ;
		}
	}
	return retArr ;
}

function _thirdlyCheck( hostInfoArr, rgInfoArr ) {
   var errMsg    = null ;
   var errMsgArr = [] ;
   var exp       = null ;
	logger.log( PDEVENT, "begin to thirdly check" ) ;
	var rgHostArr = _getHostNameFromRGInfo( rgInfoArr ) ;
	for ( var i = 0; i < rgHostArr.length; i++ ) {
		var hostName = rgHostArr[i] ;
	   var isMatch  = false ;
		for ( var j = 0; j < hostInfoArr.length; j++ ) {
      	if ( hostName == hostInfoArr[j].hostName ) {
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
}




function main() 
{
   var omConfObj = null ;
   var hostInfoArr = [] ;
	var clusterInfo = null ;
	var sdb         = null ;
   var rgInfoArr   = null ;
   var errMsg      = null ;
   var debugMsg    = null ;
   var exp         = null ;
	
   // 1. check file exist or not
   if ( !File.exist(OM_CONF_FILE) ) {
      errMsg = "OM configure file[" + OM_CONF_FILE + "] does not exist" ;
      exp = new SdbError( SDB_INVALIDARG, errMsg ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }

   // 2. get om configure info
   try {
      omConfObj = eval( '(' + Oma.getOmaConfigs(OM_CONF_FILE) + ')' ) ;
		// TODO:
	   //println( "omConfObj is: " + JSON.stringify(omConfObj) ) ;
	   logger.log( PDEVENT, "omConfObj is: " + JSON.stringify(omConfObj) ) ;
   } catch( e ) {
      errMsg = "failed to get om configure info from file[" + OM_CONF_FILE + "]" ;
      exp = new SdbError( SDB_INVALIDARG, errMsg ) ;
      logger.log( PDERROR, exp ) ;
      throw exp ;
   }

	// 3. append host info and check
	_appendHostName( omConfObj, hostInfoArr ) ;
   _appendRootInfo( omConfObj, hostInfoArr ) ;
   _appendSshPort( omConfObj, hostInfoArr )	;
	_appendInstallPath( omConfObj, hostInfoArr ) ;
	_appendAdminInfo( omConfObj, hostInfoArr ) ;
	// checking
	_firstlyCheck( hostInfoArr ) ;
	_appendRootSshObj( hostInfoArr ) ;
	_appendIP( hostInfoArr ) ;
	_appendRemoteInstalledInfo( hostInfoArr ) ;
	// checking
	_secondlyCheck( hostInfoArr ) ; 
   sdb = _connectToDB( omConfObj ) ;
   rgInfoArr = _getRGInfo( sdb ) ;
   _thirdlyCheck( hostInfoArr, rgInfoArr ) ;

	// get cluster info
	clusterInfo = _getClusterInfo( omConfObj ) ;
	// TODO:
	//println( "cluster info is: " + clusterInfo ) ;

	
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

