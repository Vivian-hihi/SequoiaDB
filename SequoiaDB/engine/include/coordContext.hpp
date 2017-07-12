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

   Source File Name = coordContext.hpp

   Descriptive Name = Context Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/04/2017  XJH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef COORD_CONTEXT_HPP__
#define COORD_CONTEXT_HPP__

#include "rtnContext.hpp"
#include "rtnContextMain.hpp"
#include "coordDef.hpp"

using namespace bson ;

namespace engine
{

   class _pmdRemoteSessionSite ;
   class _pmdRemoteSession ;

   /*
      coordSubContext define
   */
   class _coordSubContext : public _rtnSubContext
   {
   public:
      _coordSubContext ( BSONObj& orderBy,
                         _ixmIndexKeyGen* keyGen,
                         INT64 contextID,
                         MsgRouteID routeID ) ;
      ~_coordSubContext () ;


   public:
      void           appendData ( MsgOpReply *pReply ) ;
      void           clearData () ;
      MsgRouteID     getRouteID() ;
      const CHAR*    front () ;
      INT32          pop() ;
      INT32          popN( INT32 num ) ;
      INT32          popAll() ;
      INT32          recordNum() ;
      INT32          remainLength() ;
      INT32          truncate ( INT32 num ) ;
      INT32          getOrderKey( rtnOrderKey &orderKey ) ;

   private:
      // disallow copy and assign
      _coordSubContext () ;
      _coordSubContext ( const _coordSubContext& ) ;
      void operator=( const _coordSubContext& ) ;

   private:
      MsgRouteID           _routeID ;
      INT32                _curOffset ;
      MsgOpReply*          _pData ;
      INT32                _recordNum ;
   } ;
   typedef _coordSubContext coordSubContext ;

   typedef _utilMap< UINT64, coordSubContext*, 20 >         EMPTY_CONTEXT_MAP ;
   typedef _utilMap< UINT64, MsgRouteID, 20 >               PREPARE_NODES_MAP ;

   /*
      _rtnContextCoord define
   */
   class _rtnContextCoord : public _rtnContextMain
   {
      DECLARE_RTN_CTX_AUTO_REGISTER()
      public:
         _rtnContextCoord ( INT64 contextID, UINT64 eduID,
                           BOOLEAN preRead = TRUE ) ;
         virtual ~_rtnContextCoord () ;

         INT32    addSubContext ( MsgRouteID routeID, SINT64 contextID ) ;
         INT32    addSubContext ( MsgOpReply *pReply, BOOLEAN &takeOver ) ;

         void     addSubDone( _pmdEDUCB *cb ) ;

         INT32    open( const BSONObj &orderBy,
                        const BSONObj &selector,
                        INT64 numToReturn = -1,
                        INT64 numToSkip = 0,
                        BOOLEAN preRead = TRUE ) ;
         INT32    reopen () ;

         void     killSubContexts( _pmdEDUCB *cb ) ;

         INT64    getSkipNum() const { return _numToSkip ; }
         void     setSkipNum( INT64 numToSkip ) { _numToSkip = numToSkip ; }
         INT64    getLimitNum() const { return _numToReturn ; }
         void     setLimitNum( INT64 limitNum ) { _numToReturn = limitNum ; }

         virtual void     getErrorInfo( INT32 rc,
                                        pmdEDUCB *cb,
                                        rtnContextBuf &buffObj ) ;

         virtual UINT32   getCachedRecordNum() ;

      public:
         virtual std::string      name() const ;
         virtual RTN_CONTEXT_TYPE getType () const ;

         OSS_INLINE  BOOLEAN requireOrder () const ;

         void enablePreRead() { _preRead = TRUE ; }
         void disablePreRead() { _preRead = FALSE ; }

      protected:
         virtual void    _toString( stringstream &ss ) ;

      protected:
         BOOLEAN _requireExplicitSorting () const ;
         INT32   _prepareAllSubCtxDataByOrder( _pmdEDUCB *cb ) ;
         INT32   _getNonEmptyNormalSubCtx( _pmdEDUCB *cb, rtnSubContext*& subCtx ) ;
         INT32   _saveEmptyOrderedSubCtx( rtnSubContext* subCtx ) ;
         INT32   _saveEmptyNormalSubCtx( rtnSubContext* subCtx ) ;
         INT32   _saveNonEmptyNormalSubCtx( rtnSubContext* subCtx ) ;
         INT32   _doAfterPrepareData( _pmdEDUCB *cb ) ;

      private:
         INT32    _appendSubData ( CHAR *pData ) ;

         void     _delPrepareContext( const MsgRouteID &routeID ) ;

         INT32    _send2EmptyNodes( _pmdEDUCB *cb ) ;
         INT32    _getPrepareNodesData( _pmdEDUCB *cb, BOOLEAN waitAll ) ;

         INT32    _reOrderSubContext() ;
         INT32    _prepareSubCtxData( _pmdEDUCB *cb ) ;

      private:
         EMPTY_CONTEXT_MAP          _emptyContextMap ;
         EMPTY_CONTEXT_MAP          _prepareContextMap ;

         rtnOrderKey                _emptyKey ;
         BSONObj                    _orderBy ;
         BOOLEAN                    _preRead ;

         BOOLEAN                    _needReOrder ;
         /// error info
         ROUTE_RC_MAP               _nokRC ;

         _pmdRemoteSessionSite      *_pSite ;
         _pmdRemoteSession          *_pSession ;
   } ;
   typedef _rtnContextCoord rtnContextCoord ;

   /*
      _rtnContextCoord OSS_INLINE functions
   */
   OSS_INLINE BOOLEAN _rtnContextCoord::requireOrder () const
   {
      if ( _orderBy.isEmpty() ||
           _orderedContextMap.size() + _emptyContextMap.size() +
           _prepareContextMap.size() <= 1 )
      {
         return FALSE ;
      }
      return TRUE ;
   }

}

#endif //COORD_CONTEXT_HPP__

