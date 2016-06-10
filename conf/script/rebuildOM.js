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

var FIELD_COORD_INFO_HOSTNAME  = "HostName" ;
var FIELD_COORD_INFO_DBPATH    = "dbpath" ;
var FIELD_COORD_INFO_SERVICE   = "Service" ;
var FIELD_COORD_INFO_TYPE      = "Type" ;
var FIELD_COORD_INFO_NAME      = "Name" ;
var FIELD_COORD_INFO_NODEID    = "NodeID" ;
var FIELD_COORD_INFO_STATUS    = "Status" ;


var CoordInfo = function() {
   this.HostName = null ;
	this.dbpath   = null ;
	this.Service  = [ { "Type" : 0, "Name" : "" }, 
		               { "Type" : 1, "Name" : "" }, 
		               { "Type" : 2, "Name" : "" } ] ;
	this.NodeID   = null ;
	this.Status   = null ;

	CoordInfo.prototype.toString = function() {
      return JSON.stringify( this ) ;
	} ;
} ;

var FIELD_CONF_DBPATH          = "dbpath" ;
var FIELD_CONF_HOST_NAME       = "hostname" ;
var FIELD_CONF_IP              = "ip" ;
var FIELD_CONF_SVC_NAME        = "svcname" ;
var FIELD_CONF_REPL_NAME       = "replname" ;
var FIELD_CONF_CATALOG_NAME    = "catalogname" ;
var FIELD_CONF_SHARD_NAME      = "shardname" ;
var FIELD_CONF_HTTP_NAME       = "httpname" ;
var FIELD_CONF_ROLE            = "coord" ;
var FIELD_CONF_CATALOG_ADDR    = "catalogaddr" ;
//var FIELD_CONF_
//var FIELD_CONF_

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
      return JSON.stringify( this ) ;
	}
   /*
	ClusterInfo.prototype.toString = function() {
		return "cluster info[ name: " + this.clusterName + 
			", description: " + this.clusterDescription + 
			", sdb user name: " + this.sdbUserName + 
			", sdb password: " + this.sdbPassword +
			", sdb user group: " + this.sdbUserGroupName +
			", sdb install path: " + this.sdbInstallPath ;
	} ;
	*/
} ;

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

	HostInfo.prototype.toString = function() {
      return JSON.stringify( this ) ;
	} ;
} ;
/*
var CoordInfo = function() {
   this.HostName = null ;
	this.dbpath   = null ;
	this.Service  = [ { "Type" : 0, "Name" : "" }, 
		               { "Type" : 1, "Name" : "" }, 
		               { "Type" : 2, "Name" : "" } ] ;
	this.NodeID   = null ;
	this.Status   = 1 ;

	CoordInfo.prototype.toString = function() {
      return JSON.stringify( this ) ;
	} ;
} ;
*/
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

function _init() {
   logger.log( PDEVENT, "Begin to rebuild om" ) ;
   ts = genTimeStamp() ;
}

function _final() {
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
         coordHostName + ":" + coordPort + "]" ;
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

var AddCoordInfo = function( rgInfoArr ) {
	this._rgInfoArr = rgInfoArr ;
} ;

AddCoordInfo.prototype._getUsedNodeIDFromDB = 
	function AddCoordInfo__getUsedNodeIDFromDB() {
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
	// TODO: debug
	logger.log( PDDEBUG, "catalog or coord rg info size is: " + rgArr.length ) ;
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

AddCoordInfo.prototype._genNodeID = function AddCoordInfo__genNodeID( num ) {
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

AddCoordInfo.prototype._getNodeAddrFromDB = 
	function AddCoordInfo__getNodeAddrFromDB( role ) {
   var retArr   = [] ;
	var nodeArr  = null ;
   var exp      = null ;
	var rgInfo   = null ;
	var rgName   = null ;
	var portType = null ;
	
	if ( role == "catalog" ) {
		rgName   = "SYSCatalogGroup" ;
		portType = 3 ; 
	} else if ( role == "coord" ) {
		rgName   = "SYSCoord" ;
		portType = 0 ;
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
	logger.log( PDEVENT, role + " group has " + retArr.length + 
		" node(s) as below: " + retArr ) ;
	return retArr ;  
} ;

AddCoordInfo.prototype._getCataAddrFromDB = 
	function AddCoordInfo__getCataAddrFromDB() {
   var exp         = null ;
   var cataAddrArr = this._getNodeAddrFromDB( "catalog" ) ;
	if ( cataAddrArr == null || cataAddrArr.length == 0 ) {
   	exp = new SdbError( SDB_SYS, 
			"can not get any info about catalog group from database" ) ;
		logger.log( PDERROR, exp ) ;
		throw exp ;
	}
	logger.log( PDDEBUG, "_getCoordAddrFromDB returns: " + cataAddrArr ) ;
	return cataAddrArr ;
} ;

AddCoordInfo.prototype._getCoordAddrFromDB = 
	function AddCoordInfo__getCoordAddrFromDB() {
   var retArr = this._getNodeAddrFromDB( "coord" ) ;
	logger.log( PDDEBUG, "_getCoordAddrFromDB returns: " + retArr ) ;
	return retArr ;
} ;

AddCoordInfo.prototype._collectCoordInfoFromHost = 
	function AddCoorInfo__collectCoordInfoFromHost( hostInfoArr ) {
	var retArr = [] ;
   var exp    = null ;
	var str1   = "\' var arr = Sdbtool.listNodes({type:\"db\", role:\"coord\", mode:\"local\", expand:true}); \'" ;
	var str2   = "\' arr.next(); \'" ;
	var str3   = "\' quit \'" ;

   for ( var i = 0; i < hostInfoArr.length; i++ ) {
      var host        = hostInfoArr[i] ;
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
			   // TODO: get error msg
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

var CoordInfo = function() {
   this.HostName = null ;
	this.dbpath   = null ;
	this.Service  = [ { "Type" : 0, "Name" : "" }, 
		               { "Type" : 1, "Name" : "" }, 
		               { "Type" : 2, "Name" : "" } ] ;
	this.NodeID   = null ;
	this.Status   = 1 ;

	CoordInfo.prototype.toString = function() {
      return JSON.stringify( this ) ;
	} ;
} ;

var CoordInfoWrapper = function() {
	this.ip          = null ;
	this.catalogAddr = null ;
   this.infoObj     = null ;

	CoordInfoWrapper.prototype.toString = function() {
      return JSON.stringigy( this ) ;
	} ;
} ;

AddCoordInfo.prototype._extractCoordInfo = 
	function AddCoordInfo__extractCoordInfo( coordInfoObjArr ) {
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

AddCoordInfo.prototype._filterCoordInfo = 
	function AddCoordInfo__filterCoordInfo( hostCoordInfoWrapperArr, 
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

AddCoordInfo.prototype._appendNodeID =
	function AddCoordInfo__appendNodeID( matchedCoordInfoArr ) {
	var retArr    = [] ;
	var nodeIDArr = [] ;
	// get usable node id
	nodeIDArr = this._genNodeID( matchedCoordInfoArr.length ) ;
   // generate inserted records
	for ( var i = 0; i < matchedCoordInfoArr.length; i++ ) {
      var obj                      = matchedCoordInfoArr[i] ;
		// TODO: append node id
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

AddCoordInfo.prototype._init = function AddCoordInfo__init() {
   if ( !isArray(this._rgInfoArr) || this._rgInfoArr.length == 0 ) {
		var exp = new SdbError( SDB_SYS, 
			"no replica group's info in catalog, AddCoordInfo::_rgInfoArr is: " + 
			this._rgInfoArr ) ;
      logger.log( PDERROR, exp ) ;
		throw exp ;
	}
} ;

AddCoordInfo.prototype._doit = 
	function AddCoordInfo__doit( hostInfoArr ) {
   var retArr = [] ;

   // 
   this._init() ;

	// 1. ssh to remote to get all the coord's info to local
	var coordInfoObjArr = this._collectCoordInfoFromHost( hostInfoArr ) ;

	var hostCoordInfoArr = this._extractCoordInfo( coordInfoObjArr ) ;

	// . get existing coord's info from database
   var dbCoordAddrArr = this._getCoordAddrFromDB() ;

	// . get catalog's info from database
   var dbCatalogAddrArr = this._getCataAddrFromDB() ;

	// . filter the existed coords
	var matchedCoordInfoArr = 
		this._filterCoordInfo( hostCoordInfoArr, dbCoordAddrArr, dbCatalogAddrArr ) ;
   // . gen inserted coord info
   retArr = this._appendNodeID( matchedCoordInfoArr ) ;

	return retArr ;
} ;

function main() {
   var omConfObj = null ;
   var hostInfoArr = [] ;
	var clusterInfo = null ;
	var sdb         = null ;
   var rgInfoArr   = null ;
   var errMsg      = null ;
   var debugMsg    = null ;
   var exp         = null ;

	if ( SYS_LINUX != SYS_TYPE ) {
      exp = new SdbError( SDB_SYS, 
			"not support current operating system[" + SYS_TYPE + "]" ) ;
		logger.log( PDERROR, exp ) ;
		throw exp ;
	}
	
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

	// 4. get coord info
	var addCoord = new AddCoordInfo( rgInfoArr ) ;
   println( addCoord._doit(hostInfoArr) ) ;
   
	
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

