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
#include "sptApi.hpp"
#include "ossMem.h"
#include "omagent.hpp"
#include "omagentMsgDef.hpp"
#include "omagentTask.hpp"
#include <map>
#include <string>

using namespace bson ;

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

      public:
         virtual const CHAR * name () = 0 ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) = 0 ;
         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen,
                              INT32 &returnNum ) = 0 ;
         virtual INT32 doit ( BSONObj &retObj ) { return SDB_OK ; }

     protected:
         _sptScope *_scope ;
         const CHAR *_jsFileName ;
         CHAR *_fileBuff ;
         UINT32 _buffSize ;
         UINT32 _readSize ;
         std::vector<BSONObj> _hosts ;
         std::string _content ;
   };

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

   typedef std::map<const CHAR*, OA_NEW_FUNC, _classComp> MAP_OACMD ;
   typedef std::map<const CHAR*, OA_NEW_FUNC, _classComp>::iterator MAP_OACMD_IT ;

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

         void release ( const _omaCommand *pCommand ) ;

         INT32 _register ( const CHAR *name, OA_NEW_FUNC pFunc ) ;

         OA_NEW_FUNC _find ( const CHAR * name ) ;

      private:
         MAP_OACMD _cmdMap ;
   } ;

   /*
      get omagent command builder
   */
   _omaCmdBuilder* getOmaCmdBuilder() ;

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

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;
   } ;

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

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen,
                              INT32 &returnNum ) ;

         virtual INT32 doit ( BSONObj &retObj ) ;
   } ;


   /*
      _omaBasicCheckHost
   */
   class _omaBasicCheckHost : public _omaScanHost
   {
      DECLARE_OACMD_AUTO_REGISTER()

      public:
//         _omaBasicCheckHost () ;
//         ~_omaBasicCheckHost () ;

         virtual const CHAR* name () { return OMA_CMD_BASIE_CHECK_HOST ; }
   };

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

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen,
                              INT32 &returnNum ) ;
         virtual INT32 doit ( BSONObj& retObj ) { return SDB_OK ; }

      private:
   } ;

   /*
      _omaInstallRemoteAgent
   */
   class _omaInstallRemoteAgent : public _omaCommand
   {
      public:
         _omaInstallRemoteAgent () ;
         ~_omaInstallRemoteAgent () ;

         virtual const CHAR* name () { return "install remote agent" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

         INT32 getRemoteAgentStatus ( const CHAR *pIp, const CHAR *pUsername,
                                      const CHAR *pPasswork, BSONObj &result ) ;

         CHAR* getVersion() { return "1.0" ; }

   } ;

   /*
      _omaCheckRemoteAgentProcess
   */
   class _omaCheckRemoteAgentProcess : public _omaCommand
   {
      public:
         _omaCheckRemoteAgentProcess () ;
         ~_omaCheckRemoteAgentProcess () ;

         virtual const CHAR* name () { return "install remote agent" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

         INT32 check ( const CHAR *pIp, const CHAR *pUserName,
                       const CHAR *pPassword, BSONObj &result ) ;
   } ;

   /*
      _omaInstallAgentProcess
   */
   class _omaInstallAgentProcess : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaInstallAgentProcess () ;
         ~_omaInstallAgentProcess () ;

         virtual const CHAR* name () { return "install agent process" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;
   } ;

   /*
      _omaRemoveAgentProcess
   */
   class _omaRemoveAgentProcess : public _omaCommand
   {
//      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaRemoveAgentProcess () ;
         ~_omaRemoveAgentProcess () ;

         virtual const CHAR* name () { return "remove agent process" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

   } ;

   /*
      _omaStopAgentProcess
   */
   class _omaStopAgentProcess : public _omaCommand
   {
//      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaStopAgentProcess () ;
         ~_omaStopAgentProcess () ;

         virtual const CHAR* name () { return "stop agent process" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

   } ;

   /*
      _omaRegHosts
   */
   class _omaRegHosts : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaRegHosts () ;
         ~_omaRegHosts () ;

         virtual const CHAR* name () { return "reg hosts info" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

      private:
         INT32 _getHostsTableInfo () ;

         INT32 _getContentForJS ( const CHAR *pIp, std::vector<string> &hostsInfo ) ;

         std::map<string, string> _hostsTableInfo ;
   } ;

   /*
      _omaGetHostNames
   */
   class _omaGetHostNames : public _omaCommand
   {
//      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaGetHostNames () ;
         ~_omaGetHostNames () ;

         virtual const CHAR* name () { return "get host name" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

         INT32 getHostName( const CHAR *pIp, const CHAR *pUserName,
                            const CHAR *pPassword, BSONObj &result ) ;
   } ;

   /*
      _omaInstallDBBusiness
   */
   class _omaInstallDBBusiness : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaInstallDBBusiness () ;
         ~_omaInstallDBBusiness () ;

         virtual const CHAR* name () { return "install db business" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

      private:
         BOOLEAN _createVirtualCoordSucced ;
         BOOLEAN _removeVirtualCoordSucced ;

         std::vector<BSONObj> _coord ;
         std::vector<BSONObj> _catalog ;
         std::vector<BSONObj> _data ;

         _omaTaskMgr* _taskMrg ;

   } ;

   /*
      _omaInstallDBStatus
   */
   class _omaInstallDBStatus : public _omaCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omaInstallDBStatus () ;
         ~_omaInstallDBStatus () ;

         virtual const CHAR* name () { return "get install db status" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

      private:
         UINT64       _taskID ;
         _omaTaskMgr* _taskMrg ;

   } ;


   /*
      _omaCreateVirtualCoord
   */
   class _omaCreateVirtualCoord : private _omaCommand
   {
      public:
         _omaCreateVirtualCoord ( const CHAR *username,
                                  const CHAR *password ) ;
         ~_omaCreateVirtualCoord () ;

      public:
         INT32 createVirtualCoord ( INT32 coord_service, BOOLEAN &result ) ;

      private:
         virtual const CHAR* name () { return "create virtual coord" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff )
         {
            return 0 ;
         }

         INT32 init () ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum )
         {
            return 0 ;
         }

         INT32 doit ( INT32 coord_service, BOOLEAN &result ) ;

      private:
         const CHAR *_username ;
         const CHAR *_password ;
   } ;

   /*
      _omaRemoveVirtualCoord
   */
   class _omaRemoveVirtualCoord : private _omaCommand
   {
      public:
         _omaRemoveVirtualCoord ( const CHAR* usename,
                                      const CHAR *password ) ;
         ~_omaRemoveVirtualCoord () ;

      public:
         INT32 removeVirtualCoord ( INT32 coord_service, BOOLEAN &result ) ;

      private:
         virtual const CHAR* name () { return "remove virtual coord" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff )
         {
            return 0 ;
         }

         INT32 init () ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum )
         {
            return 0 ;
         }

         INT32 doit ( INT32 coord_service, BOOLEAN &result ) ;

      private:
         const CHAR* _username ;
         const CHAR* _password ;

   } ;

   /*
      _omaPort
   */
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

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                               const CHAR *pMatcherBuff,
                               const CHAR *pSelectBuff,
                               const CHAR *pOrderByBuff,
                               const CHAR *pHintBuff ) { return 0 ; }

         INT32 init() ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum )
         { return 0 ; }

         INT32 doit ( BOOLEAN &hasUsed ) ;

         void _setPort( INT32 port ) { _port = port ; }

      private:
         INT32 _port ;
   } ;


}


#endif
