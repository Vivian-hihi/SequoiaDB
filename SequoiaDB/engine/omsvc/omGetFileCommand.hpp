/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = omGetFileCommand.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/12/2014  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_GETFILECOMMAND_HPP__
#define OM_GETFILECOMMAND_HPP__

#include "omCommandInterface.hpp"
#include "restAdaptor.hpp"
#include "pmdRestSession.hpp"
#include "pmdRemoteSession.hpp"
#include "rtnCB.hpp"
#include "pmd.hpp"
#include "dmsCB.hpp"
#include "omManager.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <map>
#include <string>

using namespace bson;
using namespace boost::property_tree;

namespace engine
{
   class omAuthCommand : public omCommandInterface
   {
      public:
         omAuthCommand( restAdaptor *pRestAdaptor, 
                        pmdRestSession *pRestSession ) ;

         ~omAuthCommand() ;

      public:
         virtual INT32   doCommand() ;

      protected:
         void            _sendErrorRes2Web( INT32 rc, const CHAR* detail ) ;
         void            _sendErrorRes2Web( INT32 rc, const string &detail ) ;
         void            _decryptPasswd( const string encryptPasswd, 
                                         string time,
                                         string &decryptPasswd) ;

      protected:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
         string          _errorDetail ;
         string          _errorPosition ;
   };

   class omLogoutCommand : public omAuthCommand
   {
      public:
         omLogoutCommand( restAdaptor *pRestAdaptor, 
                          pmdRestSession *pRestSession ) ;
         ~omLogoutCommand() ;

      public:
         virtual INT32   doCommand() ;
   };

   class omChangePasswdCommand : public omAuthCommand
   {
      public:
         omChangePasswdCommand( restAdaptor *pRestAdaptor, 
                                pmdRestSession *pRestSession ) ;
         ~omChangePasswdCommand() ;

      public:
         virtual INT32   doCommand() ;

      private:
         INT32           _getRestDetail( string &user, string &oldPasswd, 
                                         string &newPasswd, string &time ) ;
   };

   class omCheckSessionCommand : public omAuthCommand
   {
      public:
         omCheckSessionCommand( restAdaptor *pRestAdaptor, 
                                pmdRestSession *pRestSession ) ;

         ~omCheckSessionCommand() ;

      public:
         virtual INT32   doCommand() ;

      protected:
   };

   class omCreateClusterCommand : public omCheckSessionCommand
   {
      public:
         omCreateClusterCommand( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession ) ;

         virtual ~omCreateClusterCommand() ;

      public:
         virtual INT32   doCommand() ;

      private:
         INT32           _getClusterInfo( string &clusterName, string &desc,
                                          string &sdbUsr, string &sdbPasswd,
                                          string &sdbUsrGroup,
                                          string &installPath ) ;
   };

   class omQueryClusterCommand : public omCreateClusterCommand 
   {
      public:
         omQueryClusterCommand( restAdaptor *pRestAdaptor, 
                                pmdRestSession *pRestSession ) ;

         ~omQueryClusterCommand() ;

      public:
         virtual INT32   doCommand() ;

   };

   class omScanHostCommand : public omCreateClusterCommand
   {
      public:
         omScanHostCommand( restAdaptor *pRestAdaptor, 
                            pmdRestSession *pRestSession, 
                            string localAgentHost, string localAgentService ) ;

         ~omScanHostCommand() ;

      public:
         virtual INT32   doCommand() ;

      protected:
         bool            _isHostExist( BSONObj &host ) ;
         void            _filterExistHost( list<BSONObj> &hostInfoList, 
                                           list<BSONObj> &hostResult ) ;
         void            _generateArray( list<BSONObj> &hostInfoList, 
                                         string arrayKeyName, 
                                         BSONObj &result ) ;
         void            _sendOkRes2Web( list<BSONObj> &hostResult ) ;
         INT32           _sendMsgToLocalAgent( omManager *om,
                                               pmdRemoteSession *remoteSession, 
                                               MsgHeader *pMsg ) ;
         INT32           _receiveFromAgent( pmdRemoteSession *remoteSession,
                                            BSONObj &result ) ;
         INT32           _getHostList( string &clusterName, 
                                       list<BSONObj> &hostInfo ) ;
         void            _clearSession( omManager *om, 
                                        pmdRemoteSession *remoteSession) ;

      private:
         INT32           _parseResonpse( VEC_SUB_SESSIONPTR &subSessionVec, 
                                         BSONObj &response, 
                                         list<BSONObj> &bsonResult ) ;

      protected:
         string          _localAgentHost ;
         string          _localAgentService ;

      private:


   };

   class omCheckHostCommand : public omScanHostCommand
   {
      public:
         omCheckHostCommand( restAdaptor *pRestAdaptor, 
                             pmdRestSession *pRestSession,
                             string localAgentHost, string localAgentService ) ;

         ~omCheckHostCommand() ;

      public:
         virtual INT32   doCommand() ;

      private:
         INT32           _doBasicCheck( list<BSONObj> &hostInfoList, 
                                        list<BSONObj> &hostResult ) ;
         INT32           _doCheck( list<BSONObj> &hostInfoList, 
                                        list<BSONObj> &hostResult ) ;
         INT32           _installAgent( list<BSONObj> &hostInfoList,
                                        list<BSONObj> &needUninstallHost ) ;
         INT32           _addCheckHostReq( omManager *om,
                                           pmdRemoteSession *remoteSession,
                                           list<BSONObj> &hostInfoList ) ;
         void            _updateDiskInfo( BSONObj &onehost ) ;
         INT32           _checkHostEnv( list<BSONObj> &hostInfoList, 
                                        list<BSONObj> &hostResult ) ;
         INT32           _uninstallAgent( list<BSONObj> &hostInfoList ) ;
         void            _eraseFromList( list<BSONObj> &hostInfoList, 
                                         BSONObj &oneHost ) ;
         void            _eraseFromListByIP( list<BSONObj> &hostInfoList, 
                                             const string &ip ) ;
         void            _eraseFromListByHost( list<BSONObj> &hostInfoList, 
                                               const string &hostName ) ;

         INT32           _notifyAgentExit( list<BSONObj> &hostInfoList ) ;
         INT32           _addAgentExitReq( omManager *om,
                                           pmdRemoteSession *remoteSession,
                                           list<BSONObj> &hostInfoList ) ;
   };

   class omAddHostCommand : public omScanHostCommand
   {
      public:
         omAddHostCommand( restAdaptor *pRestAdaptor, 
                           pmdRestSession *pRestSession,
                           string localAgentHost, string localAgentService ) ;

         ~omAddHostCommand() ;

      public:
         virtual INT32   doCommand() ;

      protected:
                         // overwrite
         INT32           _getHostList( string &clusterName, 
                                       list<BSONObj> &hostInfo ) ;

      private:
         void            _transactionRollBack( string host, string service, 
                                               INT32 transactionID ) ;
         INT32           _storeHostInfo( string clusterName, 
                                         list<BSONObj> &hostInfoList ) ;
         void            _generateTableField( BSONObjBuilder &builder, 
                                              string newFieldName,
                                              BSONObj &bsonOld,
                                              string oldFiledName ) ;
         INT32           _addHost( string clusterName, 
                                   list<BSONObj> &hostInfoList, 
                                   INT32 &transationID ) ;
         INT32           _generateAddHostReq( string clusterName,
                                              list<BSONObj> &hostInfoList, 
                                              BSONObj &bsonRequest ) ;
         INT32           _getSdbUsrInfo( string clusterName, string &sdbUser, 
                                         string &sdbPasswd, 
                                         string &sdbUserGroup ) ;
         INT32           _getClusterInstallPath( string clusterName, 
                                                 string &installPath ) ;
         INT32           _getPacketFullPath( char *path ) ;
   };

   class omQueryHostCommand : public omCreateClusterCommand
   {
      public:
         omQueryHostCommand( restAdaptor *pRestAdaptor, 
                             pmdRestSession *pRestSession ) ;

         ~omQueryHostCommand() ;

      public:
         virtual INT32   doCommand() ;
   } ;


   class omQueryBusinessTypeCommand : public omCreateClusterCommand
   {
      public:
         omQueryBusinessTypeCommand( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession, 
                                 const CHAR *pRootPath, 
                                 const CHAR *pSubPath ) ;
         virtual ~omQueryBusinessTypeCommand() ;

      public:
         virtual INT32  doCommand() ;

      protected:
         INT32          _readConfigFile( string file, BSONObj &obj ) ;
         void           _recurseParseObj( ptree &pt, BSONObj &out ) ;
         void           _parseArray( ptree &pt, 
                                     BSONArrayBuilder &arrayBuilder ) ;
         BOOLEAN        _isStringValue( ptree &pt ) ;
         BOOLEAN        _isArray( ptree &pt ) ;

      protected:
         string          _rootPath ;
         string          _subPath ;

   } ;

   class omQueryBusinessTemplateCommand : public omQueryBusinessTypeCommand
   {
      public:
         omQueryBusinessTemplateCommand( restAdaptor *pRestAdaptor, 
                                         pmdRestSession *pRestSession, 
                                         const CHAR *pRootPath, 
                                         const CHAR *pSubPath ) ;
         virtual ~omQueryBusinessTemplateCommand() ;

      public:
         virtual INT32  doCommand() ;

      protected:
         INT32          _readConfTemplate( string businessType, string file, 
                                          list<BSONObj> &clusterTypeList ) ;
         INT32          _readConfDetail( string file, 
                                         BSONObj &bsonConfDetail ) ;

      protected:

   } ;

   class omConfigBusinessCommand : public omQueryBusinessTemplateCommand
   {
      public:
         omConfigBusinessCommand( restAdaptor *pRestAdaptor, 
                                  pmdRestSession *pRestSession, 
                                  const CHAR *pRootPath, 
                                  const CHAR *pSubPath ) ;
         virtual ~omConfigBusinessCommand() ;

      public:
         virtual INT32  doCommand() ;

      protected:
         INT32          _fillHostInfo( string clusterName, string businessName,
                                       BSONObj &bsonHostInfo ) ;

         INT32          _checkBusiness( string businessName, 
                                        const string &businessType,
                                        const string &deployMod,
                                        const string &clusterName ) ;

      private:
         INT32          _generateConfig( const BSONObj &bsonTemplate, 
                                         const BSONObj &bsonHostInfo, 
                                         const BSONObj &bsonConfigItem, 
                                         BSONObj &bsonConfig ) ;
         void           _addProperties( BSONObjBuilder &builder, 
                                        const BSONObj &bsonTemplate, 
                                        const BSONObj &bsonConfDetail ) ;
         INT32          _getConfigDetail( const BSONObj &bsonTemplate, 
                                        BSONObj &bsonConfDetail ) ;
         INT32          _getTemplateInfo( BSONObj &bsonTemplate, 
                                          BSONObj &bsonHostInfo ) ;
         INT32          _fillTemplateInfo( BSONObj &bsonTemplate ) ;
         INT32          _getPropertyNameValue( BSONObj &bsonTemplate, 
                                               string propertyName, 
                                               string &value ) ;
         INT32          _getHostConfig( string hostName, string businessName,
                                        BSONObj &config ) ;

         INT32          _getExistBusiness( const string &businessName, 
                                           string &businessType,
                                           string &deployMod,
                                           string &clusterName ) ;
      protected:
         string         _clusterName ;
         string         _deployMod ;
         string         _businessType ;
         string         _businessName ;

   } ;

   class omInstallBusinessReq : public omConfigBusinessCommand
   {
      public:
         omInstallBusinessReq( restAdaptor *pRestAdaptor, 
                               pmdRestSession *pRestSession, 
                               const CHAR *pRootPath, 
                               const CHAR *pSubPath,
                               string localAgentHost, 
                               string localAgentService ) ;
         virtual ~omInstallBusinessReq() ;

      public:
         virtual INT32  doCommand() ;

      private:
         INT32          _combineConfDetail( string businessType, 
                                            string clusterType, 
                                            BSONObj &bsonConfDetail ) ;
         INT32          _extractHostInfo( BSONObj &bsonConfValue, 
                                          BSONObj &bsonHostInfo ) ;

         INT32          _applyInstallRequest( const BSONObj &bsonConfValue ) ;

         INT32          _sendMsgToLocalAgent( omManager *om,
                                              pmdRemoteSession *remoteSession, 
                                              MsgHeader *pMsg ) ;
         INT32          _receiveFromAgent( pmdRemoteSession *remoteSession,
                                           BSONObj &result ) ;
         void           _compeleteConfValue( const BSONObj &bsonHostInfo, 
                                             BSONObj &bsonConfValue ) ;
         void           _clearSession( omManager *om, 
                                       pmdRemoteSession *remoteSession) ;
         INT32          _getRestInfo( BSONObj &bsonConfValue ) ;
      private:
         string         _localAgentHost ;
         string         _localAgentService ;
   } ;

   class omQueryInstallProgress : public omCreateClusterCommand
   {
      public:
         omQueryInstallProgress( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession ) ;
         virtual ~omQueryInstallProgress() ;

      public:
         virtual INT32  doCommand() ;
      private:
         void           _testSaveTask() ;
         void           _testUpdateTask() ;
         void           _testFinishTask() ;
   } ;

   class omQueryNodeCommand : public omAuthCommand
   {
      public:
         omQueryNodeCommand( restAdaptor *pRestAdaptor, 
                             pmdRestSession *pRestSession ) ;
         virtual ~omQueryNodeCommand() ;

      public:
         virtual INT32  doCommand() ;

      protected:
         INT32          _getNodeInfo( string businessName, 
                                      map<string, BSONObj> &mapHostConf ) ;

      private:
         void           _sendNodeInfo2Web( map<string, BSONObj> &mapHostConf ) ;
         INT32          _test() ;
   } ;

   class omQueryBusinessCommand : public omAuthCommand
   {
      public:
         omQueryBusinessCommand( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession ) ;
         virtual ~omQueryBusinessCommand() ;

      public:
         virtual INT32  doCommand() ;

      private:
         void           _sendBusinessInfo2Web( list<BSONObj> &listBusiness ) ;
         INT32          _getBusinessInfo( string clusterName, 
                                          list<BSONObj> &listBusiness ) ;
   } ;

   class omStartBusinessCommand : public omAuthCommand
   {
      public:
         omStartBusinessCommand( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession ) ;
         virtual ~omStartBusinessCommand() ;

      public:
         virtual INT32  doCommand() ;
   } ;

   class omStopBusinessCommand : public omAuthCommand
   {
      public:
         omStopBusinessCommand( restAdaptor *pRestAdaptor, 
                                pmdRestSession *pRestSession ) ;
         virtual ~omStopBusinessCommand() ;

      public:
         virtual INT32  doCommand() ;
   } ;

   class omRemoveClusterCommand : public omAuthCommand
   {
      public:
         omRemoveClusterCommand( restAdaptor *pRestAdaptor, 
                                pmdRestSession *pRestSession ) ;
         virtual ~omRemoveClusterCommand() ;

      public:
         virtual INT32  doCommand() ;

      private:
         INT32          _getClusterExistHostFlag( const string &clusterName, 
                                                  BOOLEAN &flag ) ;
         INT32          _getClusterExistFlag( const string &clusterName, 
                                              BOOLEAN &flag ) ;
         INT32          _removeCluster( const string &clusterName ) ;
   } ;

   struct simpleHostInfo
   {
      string hostName ;
      string ip ;
      string user ;
      string passwd ;
      string installPath ;
   } ;

   class omRemoveHostCommand : public omScanHostCommand
   {
      public:
         omRemoveHostCommand( restAdaptor *pRestAdaptor, 
                              pmdRestSession *pRestSession,
                              string localAgentHost, 
                              string localAgentService ) ;
         virtual ~omRemoveHostCommand() ;

      public:
         virtual INT32  doCommand() ;

      private:
         INT32          _getHostExistBusinessFlag( const string &hostName, 
                                                   BOOLEAN &flag ) ;
         INT32          _getHostInfo( const string &hostName,
                                      simpleHostInfo &hostInfo,
                                      BOOLEAN &isExistFlag ) ;
         INT32          _removeHost( const simpleHostInfo &hostInfo, 
                                     BOOLEAN isForced ) ;
         INT32          _removeHostByAgent( const simpleHostInfo &hostInfo ) ;
         INT32          _deleteHostRecord( const string &hostName ) ;
         INT32          _getHostName( string &hostName, BOOLEAN &isForced ) ;
   } ;

   class omGetFileCommand : public omCommandInterface
   {
      public:
         omGetFileCommand( restAdaptor *pRestAdaptor, 
                           pmdRestSession *pRestSession, 
                           const CHAR *pRootPath, const CHAR *pSubPath ) ;
         virtual ~omGetFileCommand() ;

      public:
         virtual INT32   doCommand() ;
         virtual INT32   undoCommand() ;

      private:
         INT32           _getFileContent( string filePath, CHAR **pFileContent, 
                                          INT32 &fileContentLen ) ;

      private:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
         string          _rootPath ;
         string          _subPath ;

   };

   class restFileController : public SDBObject
   {
      public:
         static restFileController* getTransferInstance() ;

         INT32 getTransferedPath( const char *src_file, string &transfered ) ;

         bool isFileAuthorPublic( const char *file ) ;

      private:
         restFileController() ;
         restFileController(const restFileController &) ;
         restFileController& operator = ( const restFileController & ) ;

      private:
         typedef map < string, string >::iterator mapIteratorType ; 
         typedef map < string, string >::value_type mapValueType ;
         map < string, string > _transfer ;

         map < string, string > _publicAccessFiles ;
   };
}

#endif /* OM_GETFILECOMMAND_HPP__ */

