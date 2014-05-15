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

   Source File Name = catMainController.hpp

   Descriptive Name = Process MoDel Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains structure kernel control block,
   which is the most critical data structure in the engine process.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CAT_MAIN_CONTROLLER_HPP__
#define CAT_MAIN_CONTROLLER_HPP__

#include "core.hpp"
#include "netMsgHandler.hpp"
#include "catEventProcessor.hpp"
#include "msgCatalog.hpp"
#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "ossEvent.hpp"
#include <map>

namespace engine
{

   class _SDB_DMSCB ;
   class _dpsLogWrapper ;
   class _SDB_RTNCB ;
   class _authCB ;
   class sdbCatalogueCB ;
   class _SDB_KRCB ;
   class _clsMgr ;

   /*
      catMainController define
   */
   class catMainController : public _netMsgHandler, public catEventProcessor
   {
   typedef std::multimap< UINT32, SINT64 > CONTEXT_LIST ;

   public:
      catMainController() ;
      virtual ~catMainController() ;

      INT32 handleMsg( const NET_HANDLE &handle,
                       const _MsgHeader *header,
                       const CHAR *msg ) ;
      void handleClose( const NET_HANDLE &handle, _MsgRouteID id );
      INT32 init() ;
      INT32 active() ;
      INT32 deactive() ;

      void  attachCB( pmdEDUCB *cb ) ;
      void  detachCB( pmdEDUCB *cb ) ;
      ossEvent* getAttachEvent() { return &_attachEvent ; }

   private :
      INT32 catBuildMsgEvent ( const NET_HANDLE &handle, const MsgHeader *pMsg,
                               EvntCatalogInternalEvent *&pEvent ) ;
      INT32 processGetMoreMsg ( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processQueryDataGrp( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processQueryCollections( const NET_HANDLE &handle,
                                     const CHAR *pMsg ) ;
      INT32 processQueryCollectionSpaces ( const NET_HANDLE &handle,
                                           const CHAR *pMsg ) ;
      INT32 processQueryMsg( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processKillContext(const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processAuthenticate( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processAuthCrt( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processAuthDel( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processCheckRouteID( const NET_HANDLE &handle, const CHAR *pMsg ) ;
      INT32 processQueryDomain ( const NET_HANDLE &handle, const CHAR *pMsg ) ;

      INT32 processMsg( void *pMsg ) ;
      INT32 _ensureMetadata() ;
      INT32 _createSysIndex ( const CHAR *pCollection,
                              const CHAR *pIndex,
                              pmdEDUCB *cb ) ;
      INT32 _createSysCollection ( const CHAR *pCollection,
                                   pmdEDUCB *cb ) ;
      INT32 _processQueryRequest ( const NET_HANDLE &handle,
                                   const CHAR *pMsg,
                                   const CHAR *pCollectionName ) ;
      INT32 postMsg( const NET_HANDLE &handle, const MsgHeader *pHead );
      void addContext( const UINT32 &handle, INT64 contextID );
      void delContext( const UINT32 &handle );
      void delContext( const UINT32 &handle, INT64 contextID );

   private :
      EDUID             _nodeManagerEDUID;
      EDUID             _catalogManagerEDUID;
      _SDB_KRCB         *_pKrcb;
      pmdEDUMgr         *_pEduMgr;
      sdbCatalogueCB    *_pCatCB;
      _SDB_DMSCB        *_pDmsCB;
      _dpsLogWrapper    *_pDpsCB;
      pmdEDUCB          *_pEDUCB;
      _SDB_RTNCB        *_pRtnCB;
      _authCB           *_pAuthCB;
      pmdEDUCB          *_pNodeMgrCB;
      pmdEDUCB          *_pCataMgrCB;
      _clsMgr           *_pClsCB;
      CONTEXT_LIST      _contextLst;

      ossEvent          _attachEvent ;
   } ;

}

#endif // CAT_MAIN_CONTROLLER_HPP__

