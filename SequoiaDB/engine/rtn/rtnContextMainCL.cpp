/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = rtnContextMainCL.cpp

   Descriptive Name = RunTime MainCL Context

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          5/26/2017   David Li  Split from rtnContext.cpp

   Last Changed =

*******************************************************************************/
#include "rtnContextMainCL.hpp"
#include "rtn.hpp"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"

namespace engine
{

   _rtnSubCLContext::_rtnSubCLContext( BSONObj& orderBy,
                                       _ixmIndexKeyGen* keyGen,
                                       INT64 contextId )
      : _rtnSubContext( orderBy, keyGen, contextId )
   {
      _remainNum = 0;
   }

   _rtnSubCLContext::~_rtnSubCLContext()
   {
   }

   const CHAR* _rtnSubCLContext::front()
   {
      return _buffer.front();
   }

   INT32 _rtnSubCLContext::pop()
   {
      INT32 rc = SDB_OK;
      BSONObj obj;
      _isOrderKeyChange = TRUE;
      _remainNum--;
      rc = _buffer.nextObj( obj );
      if ( _remainNum <= 0 )
      {
         rtnContextBuf emptyBuf;
         _buffer = emptyBuf;
      }
      return rc;
   }

   INT32 _rtnSubCLContext::popN( INT32 num )
   {
      INT32 rc = SDB_OK;
      _isOrderKeyChange = TRUE;
      if ( num >= recordNum() )
      {
         rc = popAll();
         goto done;
      }
      while ( num > 0 )
      {
         rc = pop();
         if ( rc )
         {
            goto error;
         }
         --num;
      }
   done:
      return rc;
   error:
      goto done;
   }
   INT32 _rtnSubCLContext::popAll()
   {
      _isOrderKeyChange = TRUE;
      _remainNum = 0;
      rtnContextBuf emptyBuf;
      _buffer = emptyBuf;
      return SDB_OK;
   }

   INT32 _rtnSubCLContext::recordNum()
   {
      return _remainNum;
   }

   INT32 _rtnSubCLContext::remainLength()
   {
      return _buffer.size() - _buffer.offset() ;
   }

   INT32 _rtnSubCLContext::truncate ( INT32 num )
   {
      SDB_ASSERT( num >= 0, "num can't <0 " ) ;
      return _buffer.truncate( (UINT32) num ) ;
   }

   INT32 _rtnSubCLContext::getOrderKey( _rtnOrderKey& orderKey )
   {
      INT32 rc = SDB_OK;
      if ( _isOrderKeyChange )
      {
         if ( recordNum() <= 0 )
         {
            _orderKey.clear();
         }
         else
         {
            try
            {
               BSONObj boRecord( front() );
               rc = _orderKey.generateKey( boRecord, _keyGen );
               PD_RC_CHECK( rc, PDERROR, "Failed to get order-key(rc=%d)",
                            rc );
            }
            catch ( std::exception &e )
            {
               PD_RC_CHECK( SDB_INVALIDARG, PDERROR,
                            "Occur unexpected error:%s", e.what() ) ;
            }
         }
      }
      orderKey = _orderKey;
      _isOrderKeyChange = FALSE;
   done:
      return rc;
   error:
      goto done;
   }

   rtnContextBuf _rtnSubCLContext::buffer()
   {
      return _buffer;
   }

   void _rtnSubCLContext::setBuffer( rtnContextBuf &buffer )
   {
      _buffer = buffer;
      _isOrderKeyChange = TRUE;
      _remainNum = buffer.recordNum();
   }

   RTN_CTX_AUTO_REGISTER(_rtnContextMainCL, RTN_CONTEXT_MAINCL, "MAINCL")

   _rtnContextMainCL::_rtnContextMainCL( INT64 contextID, UINT64 eduID )
      : _rtnContextMain( contextID, eduID ),
        _includeShardingOrder( FALSE )
   {
   }

   _rtnContextMainCL::~_rtnContextMainCL()
   {
      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();
      pmdEDUCB *cb = pKrcb->getEDUMgr()->getEDUByID( eduID() );

      // clean normal context
      SUBCL_CTX_MAP::iterator iter = _subContextMap.begin();
      while( iter != _subContextMap.end() )
      {
         pRtncb->contextDelete( iter->first, cb );
         SDB_OSS_DEL iter->second ;
         ++iter;
      }
      _subContextMap.clear();
   }

   std::string _rtnContextMainCL::name() const
   {
      return "MAINCL" ;
   }

   RTN_CONTEXT_TYPE _rtnContextMainCL::getType () const
   {
      return RTN_CONTEXT_MAINCL;
   }

   INT32 _rtnContextMainCL::open( const _rtnQueryOptions &options,
                                  const std::vector<string> &subs,
                                  BOOLEAN shardSort,
                                  _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      rc = _initArgs( options, subs, shardSort ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init args:%d", rc ) ;
         goto error ;
      }

      rc = _initSubCLContext( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init cl buf:%d", rc ) ;
         goto error ;
      }

      _isOpened = TRUE ;
      _hitEnd = ( 0 == _options._limit ) ||
                ( _subContextMap.empty() ) ;
   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextMainCL::open( const bson::BSONObj & orderBy,
                                  INT64 numToReturn,
                                  INT64 numToSkip )
   {
      INT32 rc = SDB_OK;
      _options._orderBy = orderBy.getOwned();
      _numToSkip = numToSkip ;
      _numToReturn = numToReturn ;
      _keyGen = SDB_OSS_NEW _ixmIndexKeyGen( _options._orderBy ) ;
      PD_CHECK( _keyGen != NULL, SDB_OOM, error, PDERROR,
                "malloc failed!" ) ;

      _isOpened = TRUE;
      _hitEnd = 0 == numToReturn ;
   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextMainCL::_initSubCLContext( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      INT32 loop = 0 ;
      SINT64 contextID = -1 ;

      loop = ( !(_options._orderBy.isEmpty()) &&
               !_includeShardingOrder ) ?
             _subs.size() : 5 ;

      while ( 0 < loop-- )
      {
         contextID = -1 ;
         rc = _getNextContext( cb, contextID ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get next context:%d", rc ) ;
            goto error ;
         }

         if ( -1 == contextID )
         {
            break ;
         }
         else
         {
            rc = addSubContext( contextID ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to add context to main context:%d", rc ) ;
               goto error ;
            }
         }
      }
   done:
      return rc ;
   error:
      if ( -1 != contextID )
      {
         sdbGetRTNCB()->contextDelete( contextID, cb ) ;
      }
      goto done ;
   }

   INT32 _rtnContextMainCL::_getNextContext( _pmdEDUCB *cb,
                                             INT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      INT64 context = -1 ;
      _SDB_RTNCB *rtnCB = sdbGetRTNCB() ;
      rtnContextBase *contextObj = NULL ;
      if ( !_subs.empty() )
      {
         // Construct query options of sub-collection
         const string &clName = *( _subs.begin() ) ;
         rtnQueryOptions subCLOptions( _options ) ;
         subCLOptions.setMainCLQuery( _options._fullName, clName.c_str() ) ;

         rc = rtnQuery( subCLOptions, cb, sdbGetDMSCB(), rtnCB, context,
                        &contextObj, TRUE ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to query on cl:%s, rc:%d",
                    clName.c_str(), rc ) ;
            goto error ;
         }

         if ( NULL != contextObj && contextObj->isWrite() )
         {
            contextObj->setWriteInfo( this->getDPSCB(),
                                      this->getW() ) ;
         }

         _subs.pop_front() ;
         /// do not use clName again.
      }
   done:
      contextID = context ;
      return rc ;
   error:
      if ( -1 != context )
      {
         rtnCB->contextDelete( context, cb ) ;
         context = -1 ;
      }
      goto done ;
   }

   INT32 _rtnContextMainCL::_initArgs( const _rtnQueryOptions &options,
                                       const std::vector<string> &subs,
                                       BOOLEAN shardSort )
   {
      INT32 rc = SDB_OK ;
      _options = options ;
      rc = _options.getOwned() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get owned:%d", rc ) ;
         goto error ;
      }

      _numToReturn = _options._limit ;
      _numToSkip = _options._skip ;
      _includeShardingOrder = shardSort ;

      /// _options._skip will be used in sub query.
      _options._skip = 0 ;
      _keyGen = SDB_OSS_NEW _ixmIndexKeyGen( _options._orderBy ) ;
      PD_CHECK( _keyGen != NULL, SDB_OOM, error, PDERROR,
                "malloc failed!" ) ;

      if ( subs.size() <= 1 )
      {
         _includeShardingOrder = FALSE ;
         _options._skip = _numToSkip ;
         _numToSkip = 0 ;
      }
      else
      {
         if ( 0 < _numToSkip && 0 < _numToReturn )
         {
            _options._limit = _numToSkip + _numToReturn ;
         }
      }

      for ( std::vector<string>::const_iterator itr = subs.begin() ;
            itr != subs.end() ;
            ++itr )
      {
         _subs.push_back( *itr ) ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextMainCL::addSubContext( INT64 contextID )
   {
      INT32 rc = SDB_OK ;
      SUBCL_CTX_MAP::iterator iter ;
      rtnSubCLContext* subCtx = NULL ;

      iter = _subContextMap.find( contextID ) ;
      if ( iter != _subContextMap.end() )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Repeat to add sub-context (ContextID=%lld)",
                 contextID ) ;
         goto error ;
      }

      subCtx = SDB_OSS_NEW _rtnSubCLContext( _options._orderBy, _keyGen, contextID ) ;
      if ( NULL == subCtx )
      {
         rc = SDB_OOM;
         PD_LOG ( PDERROR, "Failed to alloc subcl context" ) ;
         goto error ;
      }

      _subContextMap.insert( SUBCL_CTX_MAP::value_type( contextID, subCtx ) ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN _rtnContextMainCL::requireOrder () const
   {
      return !( _options._orderBy.isEmpty() ) &&
             ( _orderedContextMap.size() > 0 || _subContextMap.size() > 1 ) ;
   }

   BOOLEAN _rtnContextMainCL::_requireExplicitSorting () const
   {
      return requireOrder() && !_includeShardingOrder ;
   }

   INT32 _rtnContextMainCL::_prepareSubCLData( SINT64 contextID,
                                                _pmdEDUCB * cb,
                                                INT32 maxNumToReturn )
   {
      INT32 rc = SDB_OK;
      _SDB_RTNCB *pRtnCB = pmdGetKRCB()->getRTNCB();
      rtnContext *pContext = NULL;
      rtnContextBuf contextBuf;
      SUBCL_CTX_MAP::iterator iterSubCTX = _subContextMap.find( contextID ) ;
      if ( _subContextMap.end() == iterSubCTX )
      {
         PD_LOG( PDERROR, "can not find context[%lld] in local context buf",
                 contextID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( iterSubCTX->second->recordNum() > 0 )
      {
         goto done;
      }

      pContext = pRtnCB->contextFind( contextID );
      PD_CHECK( pContext, SDB_RTN_CONTEXT_NOTEXIST, error, PDERROR,
                "Context %lld does not exist", iterSubCTX->first );
      rc = pContext->getMore( maxNumToReturn, contextBuf, cb );
      if ( SDB_OK == rc )
      {
         iterSubCTX->second->setBuffer( contextBuf ) ;
      }
      else if ( SDB_DMS_EOC == rc )
      {
         INT32 rcTmp = SDB_OK ;
         SINT64 contextId = -1 ;
         rcTmp = _getNextContext( cb, contextId ) ;
         if ( SDB_OK != rcTmp )
         {
            PD_LOG( PDERROR, "failed to get next context:%d", rcTmp ) ;
            rc = rcTmp ;
            goto error ;
         }
         else if ( -1 != contextId )
         {
            rcTmp = addSubContext( contextId ) ;
            if ( SDB_OK != rcTmp )
            {
               PD_LOG( PDERROR, "failed to add context:%d", rc ) ;
               rc = rcTmp ;
               sdbGetRTNCB()->contextDelete( contextId, cb ) ;
               goto error ;
            }
         }
         else
         {
            SDB_ASSERT( _subs.empty(), "must be empty" ) ;
            /// do nothing.
         }
      }
      else
      {
         PD_LOG( PDERROR, "getmore failed(rc=%d)", rc );
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   void _rtnContextMainCL::_toString( stringstream &ss )
   {
      if ( !_options._orderBy.isEmpty() )
      {
         ss << ",Orderby:" << _options._orderBy.toString().c_str()
            << ",IsShardingOrder:" << _includeShardingOrder ;
      }
      if ( _numToReturn > 0 )
      {
         ss << ",NumToReturn:" << _numToReturn ;
      }
      if ( _numToSkip > 0 )
      {
         ss << ",NumToSkip:" << _numToSkip ;
      }
   }

   INT32 _rtnContextMainCL::_prepareAllSubCtxDataByOrder( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      _SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB();

      if ( !_subs.empty() )
      {
         SDB_ASSERT( FALSE, "shoud be empty" ) ;
         PD_LOG( PDERROR, "subs shoud be empty" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( _subContextMap.size() == 0 )
      {
         goto done ;
      }

      for ( SUBCL_CTX_MAP::iterator iter = _subContextMap.begin() ;
            iter != _subContextMap.end() ; )
      {
         rtnSubCLContext* subCtx = iter->second ;

         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         if ( subCtx->recordNum() <= 0 )
         {
            rtnContextBuf contextBuf;
            rtnContext* rtnCtx = rtnCB->contextFind( subCtx->contextId() );
            PD_CHECK( rtnCtx, SDB_RTN_CONTEXT_NOTEXIST, error, PDERROR,
                      "Context %lld does not exist", subCtx->contextId() );

            rc = rtnCtx->getMore( -1, contextBuf, cb );
            if ( SDB_OK == rc )
            {
               subCtx->setBuffer( contextBuf ) ;
            }
            else if ( SDB_DMS_EOC == rc )
            {
               rtnCB->contextDelete( subCtx->contextId(), cb );
               SDB_OSS_DEL iter->second ;
               iter = _subContextMap.erase( iter ) ;
               rc = SDB_OK ;
               continue ;
            }
            else
            {
               PD_LOG( PDERROR, "getmore failed(rc=%d)", rc );
               goto error;
            }
         }

         SDB_ASSERT( subCtx->recordNum() > 0, "no data for sub ctx" ) ;

         iter = _subContextMap.erase( iter ) ;
         
         rc = _saveNonEmptyOrderedSubCtx( subCtx ) ;
         if ( rc != SDB_OK )
         {
            SDB_OSS_DEL subCtx ;
            PD_LOG ( PDERROR, "Failed to get orderKey failed, rc: %d", rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextMainCL::_getNonEmptyNormalSubCtx( _pmdEDUCB* cb, rtnSubContext*& subCtx )
   {
      INT32 rc = SDB_OK ;
      SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB();

      subCtx = NULL ;

      for ( ;; )
      {
         SUBCL_CTX_MAP::iterator iter = _subContextMap.begin();
         if ( _subContextMap.end() == iter )
         {
            rc = SDB_DMS_EOC;
            goto error ;
         }

         rtnSubCLContext* ctx = iter->second ;

         if ( ctx->recordNum() <= 0 )
         {
            rc = _prepareSubCLData( ctx->contextId(), cb, -1 ) ;
            if ( rc != SDB_OK )
            {
               rtnCB->contextDelete( ctx->contextId(), cb );
               _subContextMap.erase( ctx->contextId() );
               SDB_OSS_DEL ctx ;
               if ( SDB_DMS_EOC != rc )
               {
                  goto error;
               }
               else
               {
                  rc = SDB_OK ;
                  continue ;
               }
            }
         }

         subCtx = ctx ;
         break ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextMainCL::_saveEmptyOrderedSubCtx( rtnSubContext* subCtx )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( NULL != subCtx, "subCtx should be not null" ) ;

      try
      {
         _subContextMap.insert(
            SUBCL_CTX_MAP::value_type( subCtx->contextId(),
               dynamic_cast<_rtnSubCLContext*>( subCtx ) ) ) ;
      }
      catch( std::exception& e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "occur unexpected error:%s", e.what() );
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextMainCL::_saveEmptyNormalSubCtx( rtnSubContext* subCtx )
   {
      SDB_ASSERT( NULL != subCtx, "subCtx can't be NULL" ) ;
      SDB_ASSERT( subCtx->recordNum() == 0, "sub ctx is not empty" ) ;

      // normal sub ctx is in _subContextMap,
      // no need to do anything
      return SDB_OK ;
   }

   INT32 _rtnContextMainCL::_saveNonEmptyNormalSubCtx( rtnSubContext* subCtx )
   {
      SDB_ASSERT( NULL != subCtx, "subCtx can't be NULL" ) ;
      SDB_ASSERT( subCtx->recordNum() > 0, "sub ctx is empty" ) ;

      // normal sub ctx is in _subContextMap,
      // no need to do anything
      return SDB_OK ;
   }
}

