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
   /*
      _coordOrderKey implement
   */
   _coordOrderKey::_coordOrderKey ( const _coordOrderKey &orderKey )
   {
      _orderBy = orderKey._orderBy ;
      _hash = orderKey._hash ;
      _keyObj = orderKey._keyObj ;
      _arrEle = orderKey._arrEle ;
   }

   _coordOrderKey::_coordOrderKey ()
   {
      _hash.hash = 0 ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORDERKEY_OPELT, "coordOrderKey::operator<" )
   BOOLEAN _coordOrderKey::operator<( const _coordOrderKey &rhs ) const
   {
      PD_TRACE_ENTRY ( SDB_COORDERKEY_OPELT ) ;
      BOOLEAN result = FALSE ;
      INT32 rsCmp = _keyObj.woCompare( rhs._keyObj, _orderBy, FALSE ) ;
      if ( rsCmp < 0
         || ( 0 == rsCmp && _hash.hash < rhs._hash.hash ))
      {
         result = TRUE ;
      }
      PD_TRACE1 ( SDB_COORDERKEY_OPELT, PD_PACK_INT(result) );
      PD_TRACE_EXIT ( SDB_COORDERKEY_OPELT ) ;
      return result;
   }

   void _coordOrderKey::clear()
   {
      _arrEle = BSONElement() ;
      _hash.columns.hash1 = 0 ;
      _hash.columns.hash2 = 0 ;
   }

   void _coordOrderKey::setOrderBy( const BSONObj &orderBy )
   {
      _orderBy = orderBy;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORDERKEY_GENKEY, "coordOrderKey::generateKey" )
   INT32 _coordOrderKey::generateKey( const BSONObj &record,
                                    _ixmIndexKeyGen *keyGen )
   {
      INT32 rc = SDB_OK;
      SDB_ASSERT( keyGen != NULL, "keyGen can't be null!" ) ;
      PD_TRACE_ENTRY ( SDB_COORDERKEY_GENKEY ) ;
      clear();
      BSONObjSet keySet( _orderBy ) ;

      rc = keyGen->getKeys( record, keySet, &_arrEle ) ;
      PD_RC_CHECK( rc, PDERROR,
                  "failed to generate order-key(rc=%d)",
                  rc ) ;
      SDB_ASSERT( !keySet.empty(), "empty key-set!" ) ;
      _keyObj = *(keySet.begin()) ;
      if ( _arrEle.eoo() )
      {
         _hash.hash = 0 ;
      }
      else
      {
         ixmMakeHashValue( _arrEle, _hash ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB_COORDERKEY_GENKEY, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   _rtnSubCLBuf::_rtnSubCLBuf()
   {
      _isOrderKeyChange = TRUE;
      _remainNum = 0;
   }

   _rtnSubCLBuf::_rtnSubCLBuf( BSONObj &orderBy,
                               _ixmIndexKeyGen *keyGen )
   {
      _orderKey.setOrderBy( orderBy );
      _isOrderKeyChange = TRUE;
      _remainNum = 0;
      _keyGen = keyGen;
   }

   _rtnSubCLBuf::~_rtnSubCLBuf()
   {
      _keyGen = NULL;
   }

   const CHAR* _rtnSubCLBuf::front()
   {
      return _buffer.front();
   }

   INT32 _rtnSubCLBuf::pop()
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

   INT32 _rtnSubCLBuf::popN( SINT32 num )
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
   INT32 _rtnSubCLBuf::popAll()
   {
      _isOrderKeyChange = TRUE;
      _remainNum = 0;
      rtnContextBuf emptyBuf;
      _buffer = emptyBuf;
      return SDB_OK;
   }

   INT32 _rtnSubCLBuf::recordNum()
   {
      return _remainNum;
   }

   INT32 _rtnSubCLBuf::getOrderKey( coordOrderKey &orderKey )
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

   rtnContextBuf _rtnSubCLBuf::buffer()
   {
      return _buffer;
   }

   void _rtnSubCLBuf::setBuffer( rtnContextBuf &buffer )
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
      SubCLBufList::iterator iterLst = _subCLBufList.begin();
      while( iterLst != _subCLBufList.end() )
      {
         pRtncb->contextDelete( iterLst->first, cb );
         ++iterLst;
      }
      _subCLBufList.clear();
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

      rc = _initCLBuf( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init cl buf:%d", rc ) ;
         goto error ;
      }

      _isOpened = TRUE ;
      _hitEnd = ( 0 == _options._limit ) ||
                ( _subCLBufList.empty() ) ;
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

   INT32 _rtnContextMainCL::_initCLBuf( _pmdEDUCB *cb )
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
                                             SINT64 &contextID )
   {
      INT32 rc = SDB_OK ;
      SINT64 context = -1 ;
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
                        cb, _options._skip,
                        _options._limit,
                        sdbGetDMSCB(), rtnCB,
                        context,
                        &contextObj ) ;
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

   INT32 _rtnContextMainCL::addSubContext( SINT64 contextID )
   {
      rtnSubCLBuf emptyCTXBuf( _options._orderBy, _keyGen ) ;
      _subCLBufList[ contextID ] = emptyCTXBuf;
      return SDB_OK;
   }

   BOOLEAN _rtnContextMainCL::requireOrder () const
   {
      return 1 < _subCLBufList.size() && !(_options._orderBy.isEmpty() ) ;
   }

   INT32 _rtnContextMainCL::getMore( INT32 maxNumToReturn,
                                     rtnContextBuf &buffObj,
                                     _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      BOOLEAN hasData = FALSE;
      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();

      // release buff obj
      buffObj.release() ;

      if ( !isOpened() )
      {
         rc = SDB_DMS_CONTEXT_IS_CLOSE ;
         goto error ;
      }
      else if ( eof() && isEmpty() )
      {
         rc = SDB_DMS_EOC ;
         _isOpened = FALSE ;
         goto error ;
      }

      // OrderBy: get data one by one and caused copy
      if ( !isEmpty() || ( requireOrder() && !_includeShardingOrder ) )
      {
         rc = this->_rtnContextBase::getMore( maxNumToReturn, buffObj,
                                              cb );
         goto done;
      }

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
            SubCLBufList::iterator iterSubCTXSkip = _subCLBufList.begin();
            if ( _subCLBufList.end() == iterSubCTXSkip ||
                 iterSubCTXSkip->second.recordNum() <= 0 )
            {
               break;
            }
            if ( _numToSkip >= iterSubCTXSkip->second.recordNum() )
            {
               _numToSkip -= iterSubCTXSkip->second.recordNum();
               iterSubCTXSkip->second.popAll();
            }
            else
            {
               iterSubCTXSkip->second.popN( _numToSkip );
               _numToSkip = 0;

               // popN() only changed the offset of rtnContextBuf, so it's
               // need to jump over '_numToSkip' records
               rtnContextBuf buf = iterSubCTXSkip->second.buffer() ;
               rc = appendObjs( buf.front(),
                                buf.size() - buf.offset(),
                                iterSubCTXSkip->second.recordNum() ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to append objs, rc: %d", rc ) ;

               /// clear data in buff
               iterSubCTXSkip->second.popAll();

               rc = this->_rtnContextBase::getMore( maxNumToReturn,
                                                    buffObj, cb );
               goto done ;
            }
         }

         SINT64 curContext = -1 ;
         SubCLBufList::iterator iterSubCTX = _subCLBufList.begin();
         if ( _subCLBufList.end() == iterSubCTX )
         {
            _hitEnd = TRUE ;
            _isOpened = FALSE ;
            rc = SDB_DMS_EOC;
            goto error ;
         }

         curContext = iterSubCTX->first ;
         if ( iterSubCTX->second.recordNum() <= 0 )
         {
            rc = _prepareSubCTXData( curContext, cb, maxNumToReturn ) ;
            if ( rc != SDB_OK )
            {
               pRtncb->contextDelete( curContext, cb );
               _subCLBufList.erase( curContext );
               if ( SDB_DMS_EOC != rc )
               {
                  goto error;
               }
            }
            continue ;
         }
         buffObj = iterSubCTX->second.buffer() ;
         iterSubCTX->second.popAll() ;
         hasData = TRUE ;

         if ( _numToReturn > 0 )
         {
            if ( buffObj.recordNum() > _numToReturn )
            {
               buffObj.truncate( _numToReturn ) ;
            }
            _numToReturn -= buffObj.recordNum() ;
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

   INT32 _rtnContextMainCL::_prepareSubCTXData( SINT64 contextID,
                                                _pmdEDUCB * cb,
                                                INT32 maxNumToReturn )
   {
      INT32 rc = SDB_OK;
      _SDB_RTNCB *pRtnCB = pmdGetKRCB()->getRTNCB();
      rtnContext *pContext = NULL;
      rtnContextBuf contextBuf;
      SubCLBufList::iterator iterSubCTX = _subCLBufList.find( contextID ) ;
      if ( _subCLBufList.end() == iterSubCTX )
      {
         PD_LOG( PDERROR, "can not find context[%lld] in local context buf",
                 contextID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( iterSubCTX->second.recordNum() > 0 )
      {
         goto done;
      }

      pContext = pRtnCB->contextFind( contextID );
      PD_CHECK( pContext, SDB_RTN_CONTEXT_NOTEXIST, error, PDERROR,
                "Context %lld does not exist", iterSubCTX->first );
      rc = pContext->getMore( maxNumToReturn,
                              contextBuf,
                              cb );
      if ( SDB_OK == rc )
      {
         iterSubCTX->second.setBuffer( contextBuf ) ;
      }
      else if ( SDB_DMS_EOC == rc )
      {
         INT32 rcTmp = SDB_OK ;
         SINT64 context = -1 ;
         rcTmp = _getNextContext( cb, context ) ;
         if ( SDB_OK != rcTmp )
         {
            PD_LOG( PDERROR, "failed to get next context:%d", rcTmp ) ;
            rc = rcTmp ;
            goto error ;
         }
         else if ( -1 != context )
         {
            rcTmp = addSubContext( context ) ;
            if ( SDB_OK != rcTmp )
            {
               PD_LOG( PDERROR, "failed to add context:%d", rc ) ;
               rc = rcTmp ;
               sdbGetRTNCB()->contextDelete( context, cb ) ;
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
      SDB_ASSERT( requireOrder(), "here should be order!" );
      SDB_ASSERT( _subs.empty(), "should be empty" ) ;
      rc = _prepareDataByOrder( cb );
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

   INT32 _rtnContextMainCL::_prepareDataByOrder( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK;
      pmdKRCB *pKrcb = pmdGetKRCB();
      SDB_RTNCB *pRtncb = pKrcb->getRTNCB();

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

         SubCLBufList::iterator iterSubCTXFirst = _subCLBufList.begin();
         if ( _subCLBufList.end() == iterSubCTXFirst )
         {
            _hitEnd = TRUE ;
            if ( isEmpty() )
            {
               rc = SDB_DMS_EOC;
            }
            break;
         }
         if ( iterSubCTXFirst->second.recordNum() <= 0 )
         {
            if ( !isEmpty() )
            {
               goto done;
            }
            /// _subs shoud be empty in this function.
            /// so no new element will be put into _subCLBufList.
            rc = _prepareSubCTXData( iterSubCTXFirst->first, cb, -1 );
            if ( rc )
            {
               pRtncb->contextDelete( iterSubCTXFirst->first, cb );
               _subCLBufList.erase( iterSubCTXFirst );
               if ( rc != SDB_DMS_EOC )
               {
                  goto error;
               }
               continue;
            }
         }
         SubCLBufList::iterator iterSubCTXCur = iterSubCTXFirst;
         ++iterSubCTXCur;
         coordOrderKey firstOrderKey;
         coordOrderKey curOrderKey;
         rc = iterSubCTXFirst->second.getOrderKey( firstOrderKey );
         PD_RC_CHECK( rc, PDERROR,
                      "Failed to generate order-key(rc=%d)", rc );
         while( iterSubCTXCur != _subCLBufList.end() )
         {
            if ( iterSubCTXCur->second.recordNum() <= 0 )
            {
               if ( !isEmpty() )
               {
                  goto done;
               }
               rc = _prepareSubCTXData( iterSubCTXCur->first, cb, -1 );
               if ( rc )
               {
                  pRtncb->contextDelete( iterSubCTXCur->first, cb );
                  iterSubCTXCur = _subCLBufList.erase( iterSubCTXCur );
                  if ( rc != SDB_DMS_EOC )
                  {
                     goto error;
                  }
                  continue;
               }
            }
            rc = iterSubCTXCur->second.getOrderKey( curOrderKey );
            PD_RC_CHECK( rc, PDERROR,
                         "Failed to generate order-key(rc=%d)", rc );
            if ( curOrderKey < firstOrderKey )
            {
               iterSubCTXFirst = iterSubCTXCur;
               firstOrderKey = curOrderKey;
            }
            ++iterSubCTXCur;
         }

         if ( _numToSkip <= 0 )
         {
            try
            {
               BSONObj obj( iterSubCTXFirst->second.front() );
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
         rc = iterSubCTXFirst->second.pop();
         if ( rc )
         {
            goto error;
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
}

