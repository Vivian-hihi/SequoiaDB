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

   Source File Name = rtnCB.hpp

   Descriptive Name = RunTime Control Block Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Control Block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNCB_HPP_
#define RTNCB_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "rtnContext.hpp"
#include "ossLatch.hpp"
#include "ossAtomic.hpp"
#include "pd.hpp"
#include "monEDU.hpp"
#include "dmsCB.hpp"
#include "pmdEDU.hpp"
#include "sdbInterface.hpp"
#include "utilConcurrentMap.hpp"
#include <map>
#include <set>
#include "pmdRemoteSession.hpp"

#include "netMsgHandler.hpp"

#define SDB_EXT_DATA_NODE_ID           999999999

namespace engine
{
   class _rtnMsgHandler : public _netMsgHandler
   {
      public:
         _rtnMsgHandler( _pmdRemoteSessionMgr *pRSManager ) ;
         virtual ~_rtnMsgHandler() ;

         void  attach( _pmdEDUCB *cb ) ;
         void  detach() ;

         virtual INT32 handleMsg( const NET_HANDLE &handle,
                                  const _MsgHeader *header,
                                  const CHAR *msg ) ;
         virtual void  handleClose( const NET_HANDLE &handle, _MsgRouteID id ) ;
         virtual void  handleConnect( const NET_HANDLE &handle,
                                      _MsgRouteID id,
                                      BOOLEAN isPositive ) ;

      protected:
         _pmdRemoteSessionMgr                *_pRSManager ;
   } ;
   typedef _rtnMsgHandler rtnMsgHandler ;
   /*
      _SDB_RTNCB define
   */
   class _SDB_RTNCB : public _IControlBlock
   {
   private :
      typedef utilConcurrentMap<INT64, rtnContext*> RTN_CTX_MAP ;

      ossAtomicSigned64 _contextIdGenerator ;
      RTN_CTX_MAP       _contextMap ;
      BOOLEAN           _enableMixCmp ;
      pmdRemoteSessionMgr _rsMgr ;
      rtnMsgHandler     *_msgHandler ;
      netRouteAgent     *_routeAgent ;  // For communication with search engine adapter.
      INT64             _textIdxVersion ;

   public :
      _SDB_RTNCB() ;
      virtual ~_SDB_RTNCB() ;

      virtual SDB_CB_TYPE cbType() const { return SDB_CB_RTN ; }
      virtual const CHAR* cbName() const { return "RTNCB" ; }

      virtual INT32  init () ;
      virtual INT32  active () ;
      virtual INT32  deactive () ;
      virtual INT32  fini () ;
      virtual void   onConfigChange () ;

      SINT32 contextNew ( RTN_CONTEXT_TYPE type, rtnContext **context,
                          SINT64 &contextID, _pmdEDUCB * pEDUCB ) ;

      void contextDelete ( SINT64 contextID, _pmdEDUCB *cb ) ;

      OSS_INLINE rtnContext *contextFind ( SINT64 contextID )
      {
         std::pair<rtnContext*, bool> ret = _contextMap.find( contextID ) ;
         return ret.second ? ret.first : NULL ;
      }

      OSS_INLINE INT32 contextNum ()
      {
         return _contextMap.size() ;
      }

      OSS_INLINE void contextDump ( std::map<UINT64, std::set<SINT64> > &contextList,
                                    EDUID filterEDUID = PMD_INVALID_EDUID )
      {
         FOR_EACH_CMAP_ELEMENT_S( RTN_CTX_MAP, _contextMap )
         {
            INT64 contextID = -1  ;
            EDUID eduID = (*it).second->eduID() ;

            if ( PMD_INVALID_EDUID != filterEDUID &&
                 eduID != filterEDUID )
            {
               continue ;
            }

            contextID = (*it).second->contextID() ;
            contextList[ eduID ].insert( contextID ) ;
         }
         FOR_EACH_CMAP_ELEMENT_END
      }

      OSS_INLINE void monContextSnap ( std::map<UINT64,std::set<monContextFull> > &contextList,
                                       EDUID filterEDUID = PMD_INVALID_EDUID )
      {
         FOR_EACH_CMAP_ELEMENT_S( RTN_CTX_MAP, _contextMap )
         {
            INT64 contextID = -1  ;
            monContextCB *monCB = NULL ;
            EDUID eduID = (*it).second->eduID() ;

            if ( PMD_INVALID_EDUID != filterEDUID &&
                 eduID != filterEDUID )
            {
               continue ;
            }

            contextID = (*it).second->contextID() ;
            monCB = (*it).second->getMonCB() ;

            monContextFull item( contextID, *monCB ) ;
            item._typeDesp = (*it).second->name() ;
            item._info = (*it).second->toString() ;

            contextList[ eduID ].insert( item ) ;
         }
         FOR_EACH_CMAP_ELEMENT_END
      }

      OSS_INLINE void monContextSnap( UINT64 eduID,
                                      std::set<monContextFull> &contextList )
      {
         FOR_EACH_CMAP_ELEMENT_S( RTN_CTX_MAP, _contextMap )
         {
            INT64 contextID = (*it).second->contextID() ;
            monContextCB* monCB = (*it).second->getMonCB() ;

            monContextFull item( contextID, *monCB ) ;
            item._typeDesp = (*it).second->name() ;
            item._info = (*it).second->toString() ;

            contextList.insert( item ) ;
         }
         FOR_EACH_CMAP_ELEMENT_END
      }

      OSS_INLINE BOOLEAN isEnabledMixCmp () const
      {
         return _enableMixCmp ;
      }

      OSS_INLINE netRouteAgent* getRTAgent()
      {
         return _routeAgent ;
      }

      OSS_INLINE INT64 getTextIdxVersion()
      {
         return _textIdxVersion ;
      }
      OSS_INLINE void incTextIdxVersion()
      {
         ++_textIdxVersion ;
      }
   } ;
   typedef class _SDB_RTNCB SDB_RTNCB ;

   /*
      get global rtn cb
   */
   SDB_RTNCB* sdbGetRTNCB () ;

}

#endif //RTNCB_HPP_

