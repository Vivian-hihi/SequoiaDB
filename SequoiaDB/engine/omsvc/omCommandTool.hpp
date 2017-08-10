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

   Source File Name = omCommandTool.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/17/2017  HJW Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_COMMAND_TOOL_HPP__
#define OM_COMMAND_TOOL_HPP__

#include "rtnCB.hpp"
#include "pmd.hpp"
#include "omManager.hpp"
#include "restAdaptor.hpp"
#include "../bson/bson.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <map>
#include <string>

using namespace bson ;
using namespace std ;
using namespace boost::property_tree;

namespace engine
{
   class omXmlTool ;
   class omConfigTool ;
   class omDatabaseTool ;
   class omTaskTool ;
   class omErrorTool ;

   struct simpleAddressInfo : public SDBObject
   {
      string hostName ;
      string port ;
   } ;


   class omXmlTool : public SDBObject
   {
   public:
      INT32 readXml2Bson( const string &fileName, BSONObj &obj ) ;

   private:
      BOOLEAN _isStringValue( ptree &pt ) ;
      BOOLEAN _isArray( ptree &pt ) ;
      void _recurseParseObj( ptree &pt, BSONObj &out ) ;
      void _parseArray( ptree &pt, BSONArrayBuilder &arrayBuilder ) ;
      void _xml2Bson( ptree &pt, BSONObj &out ) ;
   } ;

   class omConfigTool : public omXmlTool
   {
   public:
      omConfigTool( string &rootPath, string &languageFileSep ) :
            _rootPath( rootPath ),
            _languageFileSep( languageFileSep )
      {
      }

      string getBuzTemplatePath( const string &businessType,
                                 const string &operationType ) ;
      string getBuzConfigPath( const string &businessType,
                               const string &deployMod,
                               BOOLEAN isSeparateConfig ) ;
      string getBuzConfigPath( const string &businessType,
                               const string &deployMod,
                               const string &isSeparateConfig ) ;

      INT32 readBuzTypeList( list<BSONObj> &businessList ) ;
      INT32 readBuzTemplate( const string &businessType,
                             const string &operationType,
                             list<BSONObj> &objList ) ;
      INT32 readBuzConfig( const string &businessType,
                           const string &deployMod,
                           BOOLEAN isSeparateConfig,
                           BSONObj &obj ) ;
      INT32 readBuzConfig( const string &businessType,
                           const string &deployMod,
                           const string &isSeparateConfig,
                           BSONObj &obj ) ;

   private:
      string _rootPath ;
      string _languageFileSep ;
   } ;

   class omDatabaseTool : public SDBObject
   {
   public:

      omDatabaseTool( pmdEDUCB* cb ) : _cb( cb )
      {
         _pKRCB  = pmdGetKRCB() ;
         _pRTNCB = _pKRCB->getRTNCB() ;
         _pDMSCB = _pKRCB->getDMSCB() ;
      }

   public:

      //task
      INT64 getTaskIdOfRunningBuz( const string &businessName ) ;

      //business
      INT32 addBusinessInfo( const INT32 addType,
                             const string &clusterName,
                             const string &businessName,
                             const string &businessType,
                             const string &deployMod,
                             const BSONObj &businessInfo ) ;

      INT32 getBusinessInfo( const string &businessName,
                             BSONObj &businessInfo ) ;

      INT32 getBusinessInfoOfCluster( const string &clusterName,
                                      BSONObj &clusterBusinessInfo ) ;

      INT32 getBusinessAddress( const string &businessName,
                                vector<simpleAddressInfo> &addressList ) ;

      BOOLEAN businessIsExist( const string &businessName ) ;

      INT32 upsertBusinessInfo( const string &businessName,
                                const BSONObj &newBusinessInfo,
                                INT64 &updateNum ) ;

      //cluster
      INT32 addCluster( const BSONObj &clusterInfo ) ;
      INT32 getClusterInfo( const string &clusterName,
                            BSONObj &clusterInfo ) ;

      INT32 updateClusterInfo( const string &clusterName,
                               const BSONObj &clusterInfo ) ;

      BOOLEAN clusterIsExist( const string &clusterName ) ;

      INT32 updateClusterGrantConf( const string &clusterName,
                                    const string &grantName,
                                    const BOOLEAN privilege ) ;

      //configure
      INT32 getOneHostConfig( const string &hostName,
                              BSONObj &config ) ;
      INT32 getHostConfigOfCluster( const string &clusterName,
                                    BSONObj &config ) ;
      INT32 getHostUsedPort( const string &hostName,
                             vector<string> &portList ) ;

      INT32 upsertConfigure( const string &businessName,
                             const string &hostName,
                             const BSONObj &newConfig,
                             INT64 &updateNum ) ;

      INT32 addNodeConfigOfBusiness( const string &clusterName,
                                     const string &businessName,
                                     const string &businessType,
                                     const BSONObj &newConfig ) ;
      INT32 updateNodeConfigOfBusiness( const string &businessName,
                                        const BSONObj &newConfig ) ;
      INT32 removeConfigure( const BSONObj &condition ) ;
      INT32 removeConfigure( const string &businessName,
                             const string &hostName ) ;

      //auth
      INT32 upsertAuth( const string &businessName, const string &authUser,
                        const string &authPasswd ) ;
      INT32 removeAuth( const string &businessName ) ;

      //host
      INT32 getHostNameByAddress( const string &address,
                                  string &hostName ) ;
      BOOLEAN isHostExistOfCluster( const string &hostName,
                                    const string &clusterName ) ;

   private:
      //host
      INT32 _getOneHostInfo( const BSONObj &matcher, const BSONObj &selector,
                             BSONObj &hostInfo ) ;

   private:

      pmdEDUCB    *_cb ;
      SDB_RTNCB   *_pRTNCB ;
      pmdKRCB     *_pKRCB ;
      SDB_DMSCB   *_pDMSCB ;
   } ;

   class omRestTool : public SDBObject
   {
   public:

      omRestTool( restAdaptor *pRestAdaptor, pmdRestSession *pRestSession ) ;

      void sendRecord2Web( list<BSONObj> &records,
                           const BSONObj *pFilter = NULL,
                           BOOLEAN inFilter = TRUE ) ;

      void sendOkRespone() ;

      void sendRespone( INT32 rc, const string &detail ) ;
      void sendRespone( INT32 rc, const char *pDetail ) ;

      void appendResponeMsg( const BSONObj &msg ) ;

   private:

      restAdaptor    *_pRestAdaptor ;
      pmdRestSession *_pRestSession ;

      list<BSONObj> _msgList ;
   } ;

   class omTaskTool : public SDBObject
   {
   public:
      omTaskTool( pmdEDUCB *cb, string &localAgentHost,
                  string &localAgentService) :
            _cb( cb ),
            _localAgentHost( localAgentHost ),
            _localAgentService( localAgentService )
      {
      }
      INT32 createTask( INT32 taskType, INT64 taskID, const string &taskName,
                        const BSONObj &taskInfo, const BSONArray &resultInfo ) ;
      INT32 notifyAgentMsg( const CHAR *pCmd, const BSONObj &request,
                            string &errDetail, BSONObj &result ) ;
      INT32 notifyAgentTask( INT64 taskID, string &errDetail ) ;

   private:
      INT32 _sendMsgToLocalAgent( omManager *om,
                                  pmdRemoteSession *pRemoteSession,
                                  MsgHeader *pMsg ) ;
      INT32 _receiveFromAgent( pmdRemoteSession *pRemoteSession,
                               SINT32 &flag, BSONObj &result ) ;
      void _clearSession( omManager *om, pmdRemoteSession *pRemoteSession ) ;

   private:
      pmdEDUCB* _cb ;
      string _localAgentHost ;
      string _localAgentService ;
   } ;

   class omErrorTool : public SDBObject
   {
   public:
      omErrorTool() : _isSet( FALSE )
      {
      }
      void setError( BOOLEAN isCover, const CHAR *pFormat, ... ) ;
      const CHAR *getError() ;

   private:
      CHAR _errorDetail[ PD_LOG_STRINGMAX + 1 ] ;
      BOOLEAN _isSet ;
   } ;
}

#endif /* OM_COMMAND_TOOL_HPP__ */
