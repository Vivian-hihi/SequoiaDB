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

   Source File Name = omManager.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_MANAGER_HPP__
#define OM_MANAGER_HPP__

#include "omDef.hpp"
#include "ossLatch.hpp"
#include "pmdRestSession.hpp"
#include "restAdaptor.hpp"
#include "sdbInterface.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "dmsCB.hpp"
#include "rtnCB.hpp"
#include "netRouteAgent.hpp"

#include <vector>
#include <string>
#include <map>

using namespace std ;
using namespace bson ;

namespace engine
{
   class _pmdEDUCB ;

   /*
      _omManager define
   */
   class _omManager : public _IControlBlock
   {
      public:
         _omManager() ;
         virtual ~_omManager() ;

         virtual SDB_CB_TYPE cbType() const { return SDB_CB_OMSVC ; }
         virtual const CHAR* cbName() const { return "OMSVC" ; }

         virtual INT32  init () ;
         virtual INT32  active () ;
         virtual INT32  deactive () ;
         virtual INT32  fini () ;
         virtual void   onConfigChange() {}

         CHAR*       allocFixBuf() ;
         INT32       getFixBufSize() const { return _fixBufSize ; }
         void        releaseFixBuf( CHAR *pBuff ) ;

         // comm interface
         netRouteAgent* getRouteAgent() ;
         MsgRouteID     updateAgentInfo( const CHAR *pHost,
                                         const CHAR *pService ) ;
         MsgRouteID     getAgentIDByHost( const CHAR *pHost ) ;
         INT32          sendMsgToAgent( const CHAR *pHost,
                                        MsgHeader *pMsg ) ;

         restSessionInfo*  attachSessionInfo( const string &id ) ;
         void              detachSessionInfo( restSessionInfo *pSessionInfo ) ;
         void              invalidSessionInfo( restSessionInfo *pSessionInfo ) ;

         restSessionInfo*  newSessionInfo( const string &userName,
                                           UINT32 localIP ) ;
         void              releaseSessionInfo ( const string &sessionID ) ;

         restAdaptor*      getRestAdptor() { return &_restAdptor ; }

         INT32             authenticate( BSONObj &obj, _pmdEDUCB *cb ) ;

      protected:

         string            _makeID( restSessionInfo *pSessionInfo ) ;

         void              _add2UserMap( const string &user,
                                         restSessionInfo *pSessionInfo ) ;

         INT32             _initOmTables();
         
         INT32             _createCollectionIndex ( const CHAR *pCollection,
                                                    const CHAR *pIndex,
                                                    pmdEDUCB *cb );

         INT32             _createCollection ( const CHAR *pCollection, pmdEDUCB *cb );

      private:
         vector< CHAR* >                        _vecFixBuf ;
         const INT32                            _fixBufSize ;

         map<string, restSessionInfo*>          _mapSessions ;
         map<string, vector<restSessionInfo*> > _mapUser2Sessions ;
         UINT32                                 _sequence ;

         ossSpinSLatch                          _omLatch ;

         restAdaptor                            _restAdptor ;

         // configure info
         INT32                                  _maxRestBodySize ;
         INT32                                  _restTimeout ;

         pmdKRCB*                               _pKrcb ;
         SDB_DMSCB*                             _pDmsCB ;
         SDB_RTNCB*                             _pRtnCB ;

         string                                 _wwwRootPath ;

   } ;

   typedef _omManager omManager ;
   /*
      get the global om manager object point
   */
   omManager *sdbGetOMManager() ;

}

#endif // OM_MANAGER_HPP__

