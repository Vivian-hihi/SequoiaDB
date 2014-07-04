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
#include <map>
#include <string>

using namespace bson;

namespace engine
{
   class omAuthCommand : public omCommandInterface
   {
      public:
         omAuthCommand( restAdaptor *pRestAdaptor, pmdRestSession *pRestSession, 
                       const CHAR *pRootPath) ;

         ~omAuthCommand() ;

      public:
         virtual INT32   doCommand() ;

      private:
         void  _sendErrorRes2Web( INT32 rc, const CHAR* detail ) ;
         
      private:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
         string          _rootPath ;
   };

   class omCheckSessionCommand : public omCommandInterface
   {
      public:
         omCheckSessionCommand( restAdaptor *pRestAdaptor, 
                                pmdRestSession *pRestSession ) ;

         ~omCheckSessionCommand() ;

      public:
         virtual INT32   doCommand() ;
         
      private:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
   };

   class omCreateClusterCommand : public omCommandInterface
   {
      public:
         omCreateClusterCommand( restAdaptor *pRestAdaptor, 
                                 pmdRestSession *pRestSession ) ;

         virtual ~omCreateClusterCommand() ;

      public:
         virtual INT32   doCommand() ;

      protected:
         void            _sendErrorRes2Web( INT32 rc, const CHAR* detail ) ;

      private:
         INT32           _getClusterInfo( string &clusterName, string &desc ) ;
         
      protected:
         restAdaptor*    _restAdaptor ;
         pmdRestSession* _restSession ;
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
         void            _checkHostExistence(list<BSONObj> &hostInfoList, 
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
         INT32           _installAgent( list<BSONObj> &hostInfoList ) ;
         INT32           _addCheckHostReq( omManager *om,
                                           pmdRemoteSession *remoteSession,
                                           list<BSONObj> &hostInfoList,
                                           list<BSONObj> &hostResult ) ;
         INT32           _checkHostEnv( list<BSONObj> &hostInfoList, 
                                        list<BSONObj> &hostResult ) ;
         INT32           _uninstallAgent( list<BSONObj> &hostInfoList ) ;
         void            _eraseFromList( list<BSONObj> &hostInfoList, 
                                         BSONObj &oneHost ) ;
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
         INT32           _addHost( list<BSONObj> &hostInfoList, 
                                   INT32 &transationID ) ;
         void            _generateAddHostReq( list<BSONObj> &hostInfoList, 
                                              BSONObj &bsonRequest ) ;
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

   
   class omQueryBusinessCommand : public omCreateClusterCommand
   {
      public:
         omQueryBusinessCommand(  restAdaptor *pRestAdaptor, 
                                  pmdRestSession *pRestSession, 
                                  const CHAR *pRootPath, 
                                  const CHAR *pSubPath ) ;
         virtual ~omQueryBusinessCommand() ;

      public:
         virtual INT32  doCommand() ;

      protected:
         string          _rootPath ;
         string          _subPath ;

   } ;

   class omQueryBusinessTemplateCommand : public omQueryBusinessCommand
   {
      public:
         omQueryBusinessTemplateCommand(  restAdaptor *pRestAdaptor, 
                                          pmdRestSession *pRestSession, 
                                          const CHAR *pRootPath, 
                                          const CHAR *pSubPath ) ;
         virtual ~omQueryBusinessTemplateCommand() ;

      public:
         virtual INT32  doCommand() ;

      protected:

   } ;
   
   class omConfigBusinessCommand : public omQueryBusinessCommand
   {
      public:
         omConfigBusinessCommand(  restAdaptor *pRestAdaptor, 
                                   pmdRestSession *pRestSession, 
                                   const CHAR *pRootPath, 
                                   const CHAR *pSubPath ) ;
         virtual ~omConfigBusinessCommand() ;

      public:
         virtual INT32  doCommand() ;

      protected:
      private:
         INT32          _generateConfig( const BSONObj &bsonTemplate, 
                                         const BSONObj &bsonHostInfo, 
                                         const BSONObj &bsonConfigItem, 
                                         BSONObj &bsonConfig ) ;
         INT32          _getConfigItem( const BSONObj &bsonTemplate, 
                                        BSONObj &bsonConfigItem ) ;
         INT32          _getTemplateInfo( BSONObj &bsonTemplate, 
                                          BSONObj &bsonHostInfo ) ;

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

   class restFileController
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

