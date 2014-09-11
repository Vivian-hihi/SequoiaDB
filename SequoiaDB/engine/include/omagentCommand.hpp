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

   Source File Name = omagentCommand.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/30/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_COMMAND_HPP_
#define OMAGENT_COMMAND_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossTypes.h"
#include "ossUtil.h"
#include "../bson/bson.h"
#include "ossMem.h"
#include "ossSocket.hpp"
#include "omagent.hpp"
#include "omagentMsgDef.hpp"
#include "omagentTask.hpp"
#include "sptScope.hpp"
#include <map>
#include <string>

using namespace bson ;
using namespace std ;

namespace engine
{
   #define DECLARE_OACMD_AUTO_REGISTER()                       \
      public:                                                  \
         static _omaCommand *newThis () ;                      \

   #define IMPLEMENT_OACMD_AUTO_REGISTER(theClass)             \
      _omaCommand* theClass::newThis ()                        \
      {                                                        \
         return SDB_OSS_NEW theClass() ;                       \
      }                                                        \
      _omaCmdAssit theClass##Assit ( theClass::newThis ) ;     \

   /*
      _omaCommand
   */
   class _omaCommand : public SDBObject
   {
      public:
         _omaCommand () ;
         virtual ~_omaCommand () ;

         INT32 setJSFile ( const CHAR *fileName ) ;

      public:
         virtual const CHAR * name () = 0 ;

         virtual INT32 init ( const CHAR *pInstallInfo ) = 0 ;

         virtual INT32 doit ( BSONObj &retObj ) = 0 ;

      protected:
         _sptScope            *_scope ;
         CHAR                 _jsFileName[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR                 _jsFileArgs[ JS_ARG_LEN + 1 ] ;
         CHAR                 *_fileBuff ;
         UINT32               _buffSize ;
         UINT32               _readSize ;
         vector<BSONObj> _hosts ;
         string          _content ;
   } ;

   typedef _omaCommand* (*OA_NEW_FUNC) () ;

   /*
      _omaCmdAssit
   */
   class _omaCmdAssit : public SDBObject
   {
      public:
         _omaCmdAssit ( OA_NEW_FUNC ) ;
         virtual ~_omaCmdAssit () ;
   } ;

   struct _classComp
   {
      bool operator()( const CHAR *lhs, const CHAR *rhs ) const
      {
         return ossStrcmp( lhs, rhs ) < 0 ;
      }
   } ;

   typedef map<const CHAR*, OA_NEW_FUNC, _classComp>     MAP_OACMD ;
#if defined (_WINDOWS)
   typedef MAP_OACMD::iterator                           MAP_OACMD_IT ;
#else
   typedef map<const CHAR*, OA_NEW_FUNC>::iterator       MAP_OACMD_IT ;
#endif // _WINDOWS

   /*
      _omaCmdBuilder
   */
   class _omaCmdBuilder : public SDBObject
   {
      friend class _omaCmdAssit ;

      public:
         _omaCmdBuilder () ;
         ~_omaCmdBuilder () ;

      public:
         _omaCommand *create ( const CHAR *command ) ;

         void release ( _omaCommand *&pCommand ) ;

         INT32 _register ( const CHAR *name, OA_NEW_FUNC pFunc ) ;

         OA_NEW_FUNC _find ( const CHAR * name ) ;

      private:
         MAP_OACMD _cmdMap ;
   } ;

   /*
      get omagent command builder
   */
   _omaCmdBuilder* getOmaCmdBuilder() ;

   /******************************* scan host ********************************/
   /*
      _omaScanHost
   */
   class _omaScanHost : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER()
      public:
         _omaScanHost () ;
         ~_omaScanHost () ;

         virtual const CHAR* name () { return OMA_CMD_SCAN_HOST ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;
   } ;


   /******************************* basic check *******************************/
   /*
      _omaBasicCheckHost
   */
   class _omaBasicCheckHost : public _omaScanHost
   {
      DECLARE_OACMD_AUTO_REGISTER()
      public:
         _omaBasicCheckHost () ;
         ~_omaBasicCheckHost () ;

         virtual const CHAR* name () { return OMA_CMD_BASIE_CHECK_HOST ; }
   } ;

   /******************************* check host ********************************/
   /*
      _omaCheckHost
   */
   class _omaCheckHost : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaCheckHost () ;
         ~_omaCheckHost () ;

         virtual const CHAR* name () { return OMA_CMD_CHECK_HOST ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj& retObj ) ;

      private:
         INT32 _adaptTheResult ( BSONObj &obj, BSONObj &result ) ;
         INT32 _adaptIP ( BSONObj &obj, BSONObjBuilder &builder ) ;
         INT32 _adaptCpu ( BSONObj &obj, BSONObjBuilder &builder ) ;
         INT32 _adaptNet ( BSONObj &obj, BSONObjBuilder &builder ) ;
         INT32 _adaptDisk ( BSONObj &obj, BSONObjBuilder &builder ) ;
         INT32 _adaptMemory ( BSONObj &obj, BSONObjBuilder &builder ) ;
         INT32 _adaptPortStatus ( BSONObj &obj, BSONObjBuilder &builder ) ;
         INT32 _adaptService ( BSONObj &obj, BSONObjBuilder &builder ) ;
         INT32 _adaptOMStatus ( BSONObj &obj, BSONObjBuilder &builder ) ;
         INT32 _adaptSafety ( BSONObj &obj, BSONObjBuilder &builder ) ;

         const CHAR *_pIp ;
         const CHAR *_pHostName ;
         const CHAR *_pUserName ;
         const CHAR *_pPassword ;

   } ;

   /******************************* install remote agent **********************/
   /*
      _omaInstallRemoteAgent
   */
   class _omaInstallRemoteAgent : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER()
      public:
         _omaInstallRemoteAgent () ;
         ~_omaInstallRemoteAgent () ;

         virtual const CHAR* name () { return OMA_CMD_INSTALL_REMOTE_AGENT ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj& retObj ) ;

         INT32 getRemoteAgentStatus ( const CHAR *pIp, const CHAR *pUsername,
                                      const CHAR *pPasswork, BSONObj &result ) ;

         CHAR* getVersion() { return "1.0" ; }

      private:
         INT32 setLocalPath() ;

         CHAR _prog_path[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR _spt_path[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR _conf_path[ OSS_MAX_PATHSIZE + 1 ] ;
   } ;

   /******************************* uninstall remote agent *******************/
   /*
      _omaUninstallRemoteAgent
   */
   class _omaUninstallRemoteAgent : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaUninstallRemoteAgent () ;
         ~_omaUninstallRemoteAgent () ;

         virtual const CHAR* name () { return OMA_CMD_UNINSTALL_REMOTE_AGENT ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

   } ;

   /******************************* add host ********************************/
   /*
      _omaAddHost
   */
   class _omaAddHost : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER()
      public:
         _omaAddHost () ;
         ~_omaAddHost () ;

         virtual const CHAR * name () { return OMA_CMD_ADD_HOST ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         INT32 rollback_internal () ;

         CHAR _packet_path[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR _sdb_user[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR _sdb_passwd[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR _sdb_user_group[ OSS_MAX_PATHSIZE + 1 ] ;

         BOOLEAN                          _needRollback ;
         vector<AddHost>                  _hasAddHosts ;
         INT32                            _transactionID ;
   } ;

   /******************************* install db business ***********************/
   /*
      _omaInstallDBBusiness
   */
   class _omaTaskMgr ;
   class _omaInstallDBBusiness : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaInstallDBBusiness () ;
         ~_omaInstallDBBusiness () ;

         virtual const CHAR* name () { return OMA_CMD_INSTALL_DB_BUSINESS ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         INT32 _genVCoordSvcName( CHAR *pSvcName, INT32 bufLen ) ;
         CHAR _omaHostName[OSS_MAX_HOSTNAME + 1] ;
         CHAR _omaSvcName[OSS_MAX_SERVICENAME + 1] ;
         CHAR _vCoordSvcName[OSS_MAX_SERVICENAME + 1] ;

         vector<BSONObj>             _coord ;
         vector<BSONObj>             _catalog ;
         vector<BSONObj>             _data ;
         vector<BSONObj>             _standalone ;

         _omaTaskMgr* _taskMrg ;

   } ;

   /******************************* query install db business status *********/
   /*
      _omaInstallDBStatus
   */
   class _omaInstallDBStatus : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaInstallDBStatus () ;
         ~_omaInstallDBStatus () ;

         virtual const CHAR* name ()
         { 
            return OMA_CMD_QUERY_INSTALL_DB_BUSINESS_PROGRESS ;
         }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         UINT64       _taskID ;
         _omaTaskMgr* _taskMrg ;

   } ;

   // _omaCreateVirtualCoord
   class _omaCreateVirtualCoord : private _omaCommand
   {
      public:
         _omaCreateVirtualCoord ( const CHAR *omaHostName,
                                  const CHAR *omaSvcName,
                                  const CHAR *vCoordSvcName ) ;
         ~_omaCreateVirtualCoord () ;

      public:
         INT32 createVirtualCoord ( BOOLEAN &result ) ;

      private:
         virtual const CHAR* name () { return "" ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         const CHAR *_omaHostName ;
         const CHAR *_omaSvcName ;
         const CHAR *_vCoordSvcName ;
   } ;

   // _omaRemoveVirtualCoord
   class _omaRemoveVirtualCoord : private _omaCommand
   {
      public:
         _omaRemoveVirtualCoord ( const CHAR *omaHostName,
                                  const CHAR *omaSvcName,
                                  const CHAR *vCoordSvcName ) ;
         ~_omaRemoveVirtualCoord () ;

      public:
         INT32 removeVirtualCoord ( BOOLEAN &result ) ;

      private:
         virtual const CHAR* name () { return "" ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         const CHAR *_omaHostName ;
         const CHAR *_omaSvcName ;
         const CHAR *_vCoordSvcName ;
   } ;

   // _omaGetRemoteAgentStatus
   class _omaGetRemoteAgentStatus : public _omaCommand
   {
      public:
         _omaGetRemoteAgentStatus () ;
         ~_omaGetRemoteAgentStatus () ;

         virtual const CHAR* name () { return "get remote agent status" ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

         INT32 getStatus ( const CHAR *pIp, const CHAR *pUserName,
                           const CHAR *pPassword, BSONObj &result ) ;
   } ;

   // _omaPort
   class _omaPort : private _omaCommand
   {
      public:
         _omaPort () ;
         _omaPort ( INT32 port ) ;
         ~_omaPort () ;

      public:

         INT32 getValidPort( INT32 range_beg, INT32 range_end, INT32 &result ) ;

         INT32 getPortStatus( INT32 port, BOOLEAN hasUsed ) ;

      private:
         virtual const CHAR* name () { return "" ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) { return 0 ; } 

         virtual INT32 doit ( BSONObj &retObj ) { return 0 ; }

         INT32 init() ;

         INT32 doit ( BOOLEAN &hasUsed ) ;

         void _setPort( INT32 port ) { _port = port ; }

      private:
         INT32 _port ;
   } ;

   // _omaRegHosts
   class _omaRegHosts : public _omaCommand
   {
      public:
         _omaRegHosts () ;
         ~_omaRegHosts () ;

         virtual const CHAR* name () { return "" ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         INT32 _getHostsTableInfo () ;

         INT32 _getHostsToReg ( const CHAR *pIp,
                                vector<string> &hostsInfo ) ;

         std::map<string, string> _hostsTableInfo ;
   } ;

   // _omaGetHostNames
   class _omaGetHostNames : public _omaCommand
   {
      public:
         _omaGetHostNames () ;
         ~_omaGetHostNames () ;

         virtual const CHAR* name () { return "get host name" ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

         INT32 getHostName( const CHAR *pIp, const CHAR *pUserName,
                            const CHAR *pPassword, BSONObj &result ) ;
   } ;

   // _omaAddHostRollbackInternal
   class _omaAddHostRollbackInternal : public _omaCommand
   {
      public:
         _omaAddHostRollbackInternal() ;
         ~_omaAddHostRollbackInternal () ;

         virtual const CHAR* name () { return "" ; }

         virtual INT32 init ( const CHAR *pInstallInfo ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;

         INT32 rollback( vector<AddHost> &hosts ) ;
      private:
         const CHAR *_pIp ;
         const CHAR *_pUserName ;
         const CHAR *_pPassword ;
         const CHAR *_pInstallPath ;
   } ;

   // run install catalog job
   class _omaRunInstallCatalogJob : public _omaCommand
   {
      public:
         _omaRunInstallCatalogJob ( string &vCoordHostName,
                                    string &vCoordSvcName,
                                    InstallInfo &info ) ;
         virtual ~_omaRunInstallCatalogJob () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         BSONObj                                        _installInfo ;
         InstallInfo                                    _info ;
         string                                         _vCoordHostName ;
         string                                         _vCoordSvcName ;
   } ;

   // run install coord job
   class _omaRunInstallCoordJob : public _omaCommand
   {
      public:
         _omaRunInstallCoordJob ( string &vCoordHostName,
                                  string &vCoordSvcName,
                                  InstallInfo &info ) ;
         virtual ~_omaRunInstallCoordJob () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         BSONObj                                        _installInfo ;
         InstallInfo                                    _info ;
         string                                         _vCoordHostName ;
         string                                         _vCoordSvcName ;
   } ;

   // run install data node job
   class _omaRunInstallDataNodeJob : public _omaCommand
   {
      public:
         _omaRunInstallDataNodeJob ( string &vCoordHostName,
                                     string &vCoordSvcName,
                                     InstallInfo &info ) ;
         virtual ~_omaRunInstallDataNodeJob () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         BSONObj                                        _installInfo ;
         InstallInfo                                    _info ;
         string                                         _vCoordHostName ;
         string                                         _vCoordSvcName ;
   } ;

   // install db business task run rollback coord job
   class _omaRunRollbackCoordJob : public _omaCommand
   {
      public:
         _omaRunRollbackCoordJob ( string &vCoordHostName,
                                   string &vCoordSvcName, 
                                   map< string, vector<InstalledNode> > &info
                                 ) ;
         ~_omaRunRollbackCoordJob () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         map< string, vector< InstalledNode > >         &_info ;
         string                                         _vCoordHostName ;
         string                                         _vCoordSvcName ;
   } ;

   // install db business task run rollback catalog job
   class _omaRunRollbackCatalogJob : public _omaCommand
   {
      public:
         _omaRunRollbackCatalogJob ( string &vCoordHostName,
                                     string &vCoordSvcName, 
                                     map< string, vector<InstalledNode> > &info
                                   ) ;
         ~_omaRunRollbackCatalogJob () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         map< string, vector< InstalledNode > >         &_info ;
         string                                         _vCoordHostName ;
         string                                         _vCoordSvcName ;
   } ;

   // install db business task run rollback data nodes job
   class _omaRunRollbackDataNodeJob : public _omaCommand
   {
      public:
         _omaRunRollbackDataNodeJob ( string &vCoordHostName,
                                      string &vCoordSvcNamem,
                                      map< string, vector<InstalledNode> > &info
                                    ) ;
         ~_omaRunRollbackDataNodeJob () ;

      public:
         virtual const CHAR* name () { return "" ; }
         virtual INT32 init ( const CHAR *pInstallInfo ) ;
         virtual INT32 doit ( BSONObj &retObj ) ;

      private:
         void _getInstalledDataGroupInfo( BSONObj& obj ) ;         

         map< string, vector< InstalledNode > >         &_info ;
         string                                         _vCoordHostName ;
         string                                         _vCoordSvcName ;
   } ;



}


#endif // OMAGENT_COMMAND_HPP_
