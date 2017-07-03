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

   INT32 _rtnSubCLContext::popN( SINT32 num )
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
   :_rtnContextBase( contextID, eduID ),
    _includeShardingOrder( FALSE ),
    _keyGen( NULL ),
    _numToReturn( -1 ),
    _numToSkip( 0 )
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

      // clean ordered context
      SUBCL_ORDER_CTX_MAP::iterator orderIter = _orderContextMap.begin() ;
      while ( orderIter != _orderContextMap.end() )
      {
         pRtncb->contextDelete( orderIter->second->contextId(), cb );
         SDB_OSS_DEL orderIter->second ;
         ++orderIter ;
      }
      _orderContextMap.clear() ;

      SAFE_OSS_DELETE( _keyGen );
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
         const string &clName = *( _subs.begin() ) ;
         rc = rtnQuery( clName.c_str(),
                        _options._selector,
                        _options._query,
                        _options._orderBy,
                        _options._hint,
                        _options._flag,
                        cb,
                        _options._skip,
                        _options._limit,
                        sdbGetDMSCB(),
                        rtnCB,
                        context,
                        &contextObj,
                        TRUE ) ;
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
             ( _orderContextMap.size() > 0 || _subContextMap.size() > 1 ) ;
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

   INT32 _rtnContextMainCL::_prepareData( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      if ( requireOrder() && !_includeShardingOrder )
      {
         SDB_ASSERT( _subs.empty(), "should be empty" ) ;
         rc = _prepareDataByOrder( cb ) ;
      }
      else
      {
         rc = _prepareDataNormal( cb ) ;
      }

      return rc;
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

   INT32 _rtnContextMainCL::_prepareAllSubCLDataByOrder( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      _SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB();

      if ( _subContextMap.size() == 0 )
      {
         goto done ;
      }

      for ( SUBCL_CTX_MAP::iterator iter = _subContextMap.begin() ;
            iter != _subContextMap.end() ; )
      {
         rtnOrderKey orderKey ;
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

         rc = subCtx->getOrderKey( orderKey ) ;
         if ( rc != SDB_OK )
         {
            PD_LOG ( PDERROR, "Failed to get orderKey failed, rc: %d", rc ) ;
            goto error ;
         }

         iter = _subContextMap.erase( iter ) ;
         _orderContextMap.insert( SUBCL_ORDER_CTX_MAP::value_type(
                                    orderKey, subCtx ) ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnContextMainCL::_prepareDataByOrder( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      SDB_RTNCB *pRtncb = pmdGetKRCB()->getRTNCB();

      if ( !_subs.empty() )
      {
         SDB_ASSERT( FALSE, "shoud be empty" ) ;
         PD_LOG( PDERROR, "subs shoud be empty" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      while ( 0 != _numToReturn )
      {
         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         if ( _subContextMap.size() > 0 )
         {
            // get data from all empty sub contexts
            rc = _prepareAllSubCLDataByOrder( cb ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "Failed to prepare all sub collections' data, rc=%d", rc ) ;
               goto error ;
            }
         }

         SDB_ASSERT( _subContextMap.size() == 0, "_subContextMap should be empty" ) ;

         if ( _orderContextMap.size() == 0 )
         {
            _hitEnd = TRUE ;
            rc = SDB_DMS_EOC ;
            goto error ;
         }

         if ( 0 == _numToReturn )
         {
            _hitEnd = TRUE ;
            break ;
         }

         if ( eof() )
         {
            break ;
         }

         SUBCL_ORDER_CTX_MAP::iterator iter = _orderContextMap.begin();
         if ( _orderContextMap.end() == iter )
         {
            _hitEnd = TRUE ;
            if ( isEmpty() )
            {
               rc = SDB_DMS_EOC;
            }
            break;
         }

         rtnSubCLContext* ctx = iter->second ;

         if ( _numToSkip <= 0 )
         {
            try
            {
               BSONObj obj( ctx->front() );
               rc = append( obj ) ;
               PD_RC_CHECK( rc, PDERROR,
                            "Failed to append data(rc=%d)", rc );
            }
            catch ( std::exception &e )
            {
               PD_LOG( PDERROR, "occur unexpected error:%s", e.what() );
               goto error;
            }

            if ( _numToReturn > 0 )
            {
               --_numToReturn ;
            }
         }
         else
         {
            --_numToSkip ;
         }

         rc = ctx->pop();
         if ( rc )
         {
            goto error;
         }

         if ( ctx->recordNum() <= 0 )
         {
            _orderContextMap.erase ( iter ) ;
            _subContextMap.insert( SUBCL_CTX_MAP::value_type(
                                       ctx->contextId(),
                                       ctx ) ) ;
            break ;
         }
         else
         {
            rtnOrderKey orderKey ;
            rc = ctx->getOrderKey( orderKey ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to get orderKey, rc:%d", rc ) ;

            _orderContextMap.erase ( iter ) ;
            _orderContextMap.insert( SUBCL_ORDER_CTX_MAP::value_type( orderKey,
                                       ctx ) ) ;
         }

         // make sure we still have room to read another
         // record_max_sz (i.e. 16MB). if we have less than 16MB
         // to 256MB, we can't safely assume the next record we
         // read will not overflow the buffer, so let's just break
         // before reading the next record
         if ( buffEndOffset() + DMS_RECORD_MAX_SZ >
              RTN_RESULTBUFFER_SIZE_MAX )
         {
            // let's break if there's no room for another max record
            break ;
         }
      }

      if ( 0 == _numToReturn )
      {
         _hitEnd = TRUE ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 _rtnContextMainCL::_prepareDataNormal( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN hasData = FALSE;
      SDB_RTNCB *pRtncb = pmdGetKRCB()->getRTNCB();

      // buffer is empty and not need order,
      // directly get data from sub-context
      while( !hasData )
      {
         if ( cb->isInterrupted() )
         {
            rc = SDB_APP_INTERRUPT ;
            goto error ;
         }

         // skip the records
         while ( _numToSkip > 0 )
         {
            SUBCL_CTX_MAP::iterator iterSubCTXSkip = _subContextMap.begin();
            if ( _subContextMap.end() == iterSubCTXSkip ||
                 iterSubCTXSkip->second->recordNum() <= 0 )
            {
               break;
            }
            if ( _numToSkip >= iterSubCTXSkip->second->recordNum() )
            {
               _numToSkip -= iterSubCTXSkip->second->recordNum();
               iterSubCTXSkip->second->popAll();
            }
            else
            {
               iterSubCTXSkip->second->popN( _numToSkip );
               _numToSkip = 0;

               // popN() only changed the offset of rtnContextBuf, so it's
               // need to jump over '_numToSkip' records
               rtnContextBuf buf = iterSubCTXSkip->second->buffer() ;
               rc = appendObjs( buf.front(),
                                buf.size() - buf.offset(),
                                iterSubCTXSkip->second->recordNum() ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to append objs, rc: %d", rc ) ;

               /// clear data in buff
               iterSubCTXSkip->second->popAll();
               goto done ;
            }
         }

         SINT64 curContext = -1 ;
         SUBCL_CTX_MAP::iterator iterSubCTX = _subContextMap.begin();
         if ( _subContextMap.end() == iterSubCTX )
         {
            _hitEnd = TRUE ;
            _isOpened = FALSE ;
            rc = SDB_DMS_EOC;
            goto error ;
         }

         curContext = iterSubCTX->first ;
         if ( iterSubCTX->second->recordNum() <= 0 )
         {
            rc = _prepareSubCLData( curContext, cb, -1 ) ;
            if ( rc != SDB_OK )
            {
               pRtncb->contextDelete( curContext, cb );
               _subContextMap.erase( curContext );
               if ( SDB_DMS_EOC != rc )
               {
                  goto error;
               }
            }
            continue ;
         }

         rtnContextBuf buffObj = iterSubCTX->second->buffer() ;
         iterSubCTX->second->popAll() ;

         if ( _numToReturn > 0 )
         {
            if ( buffObj.recordNum() > _numToReturn )
            {
               buffObj.truncate( _numToReturn ) ;
            }
            _numToReturn -= buffObj.recordNum() ;
         }

         rc = appendObjs( buffObj.front(),
                          buffObj.size() - buffObj.offset(),
                          buffObj.recordNum() ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to append objs, rc: %d", rc ) ;
         hasData = TRUE ;
      }

      if ( 0 == _numToReturn )
      {
         _hitEnd = TRUE ;
      }

   done:
      return rc;
   error:
      goto done;
   }
}

