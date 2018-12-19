/*******************************************************************************

   Copyright (C) 2012-2018 SequoiaDB Ltd.

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
@description: modify sequoiasql-postgresql config
@modify list:
   2018-12-13 JiaWen He  Init

@parameter
   var BUS_JSON = {"Command":"update business config","ClusterName":"myCluster1","BusinessName":"myModule3","BusinessType":"sequoiasql-postgresql","User":"","Passwd":"","Info":{"HostName":"ubuntu-jw-02","dbpath":"/opt/sequoiasql/postgresql/database/5432","InstallPath":"/opt/sequoiasql/postgresql/"},"Config":{"property":{"a":1,"b":2}}}

@return
   RET_JSON: the format is: { "errno": 0 }
*/

function _getErrorMsg( rc, e, message )
{
   var error = null ;

   if( rc == SDB_OK )
   {
      rc = SDB_SYS ;
      error = new SdbError( rc, e.message ) ;
   }
   else if( rc )
   {
      error = new SdbError( rc, message ) ;
   }

   return error ;
}

function _getAgentPort( hostName )
{
   return Oma.getAOmaSvcName( hostName ) ;
}

function _runRemoteCmd( cmd, command, arg, timeout )
{
   var error = null ;

   try
   {
      cmd.run( command, arg, timeout ) ;
   }
   catch( e )
   {
      var rc = cmd.getLastRet() ;
      var out = cmd.getLastOut() ;

      if( rc )
      {
         error = new SdbError( rc, out ) ;
      }
      else
      {
         if( typeof( e ) == "number" )
         {
            error = new SdbError( e, "failed to exec cmd" ) ;
         }
         else
         {
            error = new SdbError( SDB_SYS, "failed to exec cmd." ) ;
         }
      }
   }

   return error ;
}

function _deleteConfig( PD_LOGGER )
{
   var result        = {} ;
   var info          = BUS_JSON[FIELD_INFO] ;
   var config        = BUS_JSON[FIELD_CONFIG] ;
   var businessName  = BUS_JSON[FIELD_BUSINESS_NAME] ;

   var hostName      = info[FIELD_HOSTNAME] ;
   var dbpath        = info[FIELD_DBPATH] ;
   var installPath   = info[FIELD_INSTALL_PATH] ;
   var agentPort     = _getAgentPort( hostName ) ;
   var deleteConfig  = config[FIELD_PROPERTY] ;

   var ctlFile       = installPath + '/bin/sdb_sql_ctl' ;
   var exec          = ctlFile ;
   var args          = '' ;
   var timeout       = 600000 ;

   result[FIELD_ERRNO] = SDB_OK ;
   result[FIELD_DETAIL] = "" ;

   try
   {
      var options = { 'sensitive': true, 'delimiter': false } ;
      var configFile = dbpath + '/postgresql.conf' ;
      var oma = new Oma( hostName, agentPort ) ;

      var newConfig = {} ;
      var oldConfig = oma.getIniConfigs( configFile, options ).toObj() ;

      for ( var key in oldConfig )
      {
         if ( deleteConfig[key] == undefined )
         {
            newConfig[key] = oldConfig[key] ;
         }
      }

      oma.setIniConfigs( newConfig, configFile, options ) ;
   }
   catch( e )
   {
      var lastErrObj = getLastErrObj() ;

      result = lastErrObj.toObj() ;

      PD_LOGGER.log( PDERROR, lastErrObj ) ;
   }

   try
   {
      var remote = new Remote( hostName, agentPort ) ;
      var cmd    = remote.getCmd() ;

      var libraryCmd = 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:' ;

      libraryCmd += installPath + '/lib ;' ;
      exec = libraryCmd + exec ;

      //add inst
      args = ' reload ' + businessName ;
      var error = _runRemoteCmd( cmd, exec, args, timeout ) ;
      if ( error !== null )
      {
         if ( error.getErrCode() > 0 )
         {
            result[FIELD_ERRNO]  = SDB_RTN_CONF_NOT_TAKE_EFFECT ;
         }
         else
         {
            result[FIELD_ERRNO]  = error.getErrCode() ;
         }

         result[FIELD_DETAIL] = error.getErrMsg() ;
         PD_LOGGER.log( PDERROR, result[FIELD_DETAIL] ) ;
      }
   }
   catch( e )
   {
      result[FIELD_ERRNO]  = SDB_RTN_CONF_NOT_TAKE_EFFECT ;
      result[FIELD_DETAIL] = getErr( SDB_RTN_CONF_NOT_TAKE_EFFECT ) ;

      PD_LOGGER.log( PDERROR, e ) ;
   }

   return result ;
}

function _updateConfig( PD_LOGGER )
{
   var result        = {} ;
   var info          = BUS_JSON[FIELD_INFO] ;
   var config        = BUS_JSON[FIELD_CONFIG] ;
   var businessName  = BUS_JSON[FIELD_BUSINESS_NAME] ;

   var hostName      = info[FIELD_HOSTNAME] ;
   var dbpath        = info[FIELD_DBPATH] ;
   var installPath   = info[FIELD_INSTALL_PATH] ;
   var agentPort     = _getAgentPort( hostName ) ;
   var updateConfig  = config[FIELD_PROPERTY] ;

   var ctlFile       = installPath + '/bin/sdb_sql_ctl' ;
   var exec          = ctlFile ;
   var args          = '' ;
   var timeout       = 600000 ;

   result[FIELD_ERRNO] = SDB_OK ;
   result[FIELD_DETAIL] = "" ;

   try
   {
      var options = { 'sensitive': true, 'delimiter': false } ;
      var configFile = dbpath + '/postgresql.conf' ;
      var oma = new Oma( hostName, agentPort ) ;

      var newConfig = oma.getIniConfigs( configFile, options ).toObj() ;

      for ( var key in updateConfig )
      {
         newConfig[key] = updateConfig[key] ;
      }

      oma.setIniConfigs( newConfig, configFile, options ) ;
   }
   catch( e )
   {
      var lastErrObj = getLastErrObj() ;

      result = lastErrObj.toObj() ;

      PD_LOGGER.log( PDERROR, lastErrObj ) ;
   }

   try
   {
      var remote = new Remote( hostName, agentPort ) ;
      var cmd    = remote.getCmd() ;

      var libraryCmd = 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:' ;

      libraryCmd += installPath + '/lib ;' ;
      exec = libraryCmd + exec ;

      //add inst
      args = ' reload ' + businessName ;
      var error = _runRemoteCmd( cmd, exec, args, timeout ) ;
      if ( error !== null )
      {
         if ( error.getErrCode() > 0 )
         {
            result[FIELD_ERRNO]  = SDB_RTN_CONF_NOT_TAKE_EFFECT ;
         }
         else
         {
            result[FIELD_ERRNO]  = error.getErrCode() ;
         }

         result[FIELD_DETAIL] = error.getErrMsg() ;
         PD_LOGGER.log( PDERROR, result[FIELD_DETAIL] ) ;
      }
   }
   catch( e )
   {
      result[FIELD_ERRNO]  = SDB_RTN_CONF_NOT_TAKE_EFFECT ;
      result[FIELD_DETAIL] = getErr( SDB_RTN_CONF_NOT_TAKE_EFFECT ) ;

      PD_LOGGER.log( PDERROR, e ) ;
   }

   return result ;
}

function run()
{
   var PD_LOGGER = new Logger( "sequoiadb.js" ) ;
   var result = null ;

   var command = BUS_JSON[FIELD_COMMAND] ;

   if ( FIELD_UPDATE_BUSINESS_CONFIG == command )
   {
      result = _updateConfig( PD_LOGGER ) ;
   }
   else if ( FIELD_DELETE_BUSINESS_CONFIG == command )
   {
      result = _deleteConfig( PD_LOGGER ) ;
   }
   else
   {
      var error = new SdbError( SDB_INVALIDARG,
                                sprintf( "Invalid command [?]", command ) ) ;
      PD_LOGGER.log( PDERROR, error ) ;
      throw error ;
   }

   return result ;
}