/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = utilResult.cpp

   Descriptive Name = util result

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          02/18/2019  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "utilResult.hpp"
#include "msgDef.hpp"
#include "msg.h"
#include "pd.hpp"

using namespace bson ;

namespace engine
{
   utilResult::utilResult()
   {
   }

   utilResult::~utilResult()
   {
   }

   /*
      utilWriteResult implement
   */
   utilWriteResult::utilWriteResult( UINT32 mask )
   {
      _resultMask = mask ;
   }

   utilWriteResult::~utilWriteResult()
   {
   }

   void utilWriteResult::reset()
   {
      utilResult::reset() ;
      resetDupInfo() ;
      resultResultInfo() ;
   }

   BSONObj utilWriteResult::toBSON() const
   {
      BSONObjBuilder builder( 128 ) ;
      toBSON( builder ) ;
      return builder.obj() ;
   }

   void utilWriteResult::setResultObj( const BSONObj &obj )
   {
      try
      {
         _resultObj = obj.getOwned() ;
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "Save result object occur exception: %s", e.what() ) ;
      }
   }

   void utilWriteResult::resultResultInfo()
   {
      _resultObj = BSONObj() ;
   }

   BOOLEAN utilWriteResult::_filterResultElement( const BSONElement &e ) const
   {
      if ( 0 == ossStrcmp( OP_ERRNOFIELD, e.fieldName() ) ||
           0 == ossStrcmp( OP_ERRDESP_FIELD, e.fieldName() ) ||
           0 == ossStrcmp( OP_ERR_DETAIL, e.fieldName() ) )
      {
         return FALSE ;
      }
      else if ( 0 == ossStrcmp( FIELD_NAME_CURRENTID, e.fieldName() ) &&
                !_curID.getField( DMS_ID_KEY_NAME ).eoo() )
      {
         return FALSE ;
      }
      else if ( 0 == ossStrcmp( FIELD_NAME_PEERID, e.fieldName() ) &&
                !_peerID.getField( DMS_ID_KEY_NAME ).eoo() )
      {
         return FALSE ;
      }

      if ( 0 == ossStrcmp( FIELD_NAME_INDEXNAME, e.fieldName() ) ||
           0 == ossStrcmp( FIELD_NAME_INDEX, e.fieldName() ) ||
           0 == ossStrcmp( FIELD_NAME_INDEXVALUE, e.fieldName() ) )
      {
         if ( !isMaskEnabled( UTIL_RESULT_MASK_DUP ) || !isDupInfoEmpty() )
         {
            return FALSE ;
         }
      }
      return TRUE ;
   }

   void utilWriteResult::toBSON( BSONObjBuilder &builder ) const
   {
      if ( !_resultObj.isEmpty() )
      {
         try
         {
            BSONObjIterator itr( _resultObj ) ;
            while( itr.more() )
            {
               BSONElement e = itr.next() ;
               if ( _filterResultElement( e ) )
               {
                  builder.append( e ) ;
               }
            }
         }
         catch ( std::exception &e )
         {
            PD_LOG( PDERROR, "Build result information occur exception: %s",
                    e.what() ) ;
         }
      }

      if ( isMaskEnabled( UTIL_RESULT_MASK_DUP ) &&
           !isDupInfoEmpty() )
      {
         try
         {
            utilIdxDupErrAssit assit( _idxKeyPattern, _idxValue ) ;
            BSONObj keyValue ;
            assit.getIdxMatcher( keyValue, FALSE ) ;

            builder.append( FIELD_NAME_INDEXNAME, _idxName ) ;
            builder.append( FIELD_NAME_INDEX, _idxKeyPattern ) ;
            builder.append( FIELD_NAME_INDEXVALUE, keyValue ) ;
         }
         catch( std::exception &e )
         {
            PD_LOG( PDERROR, "Build duplicate result occur exception: %s",
                    e.what() ) ;
         }
      }

      if ( isMaskEnabled( UTIL_RESULT_MASK_ID ) )
      {
         try
         {
            BSONElement e = _curID.getField( DMS_ID_KEY_NAME ) ;
            if ( !e.eoo() )
            {
               builder.appendAs( e, FIELD_NAME_CURRENTID ) ;
            }
            e = _peerID.getField( DMS_ID_KEY_NAME ) ;
            if ( !e.eoo() )
            {
               builder.appendAs( e, FIELD_NAME_PEERID ) ;
            }
         }
         catch( std::exception &e )
         {
            PD_LOG( PDERROR, "Build ID result occur exception: %s",
                    e.what() ) ;
         }
      }
   }

   void utilWriteResult::enableMask( UINT32 mask )
   {
      OSS_BIT_SET( _resultMask, mask ) ;
   }

   void utilWriteResult::disableMask( UINT32 mask )
   {
      OSS_BIT_CLEAR( _resultMask, mask ) ;
   }

   BOOLEAN utilWriteResult::isMaskEnabled( UINT32 mask ) const
   {
      return OSS_BIT_TEST( _resultMask, mask ) ;
   }

   void utilWriteResult::resetDupInfo()
   {
      _idxName.clear() ;
      _idxKeyPattern = BSONObj() ;
      _idxValue = BSONObj() ;
      _curID = BSONObj() ;
      _peerID = BSONObj() ;
      _curRID.reset() ;
      _peerRID.reset() ;
   }

   BOOLEAN utilWriteResult::isDupInfoEmpty() const
   {
      if ( _idxName.empty() || _idxKeyPattern.isEmpty() ||
           _idxValue.isEmpty() )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   ossPoolString utilWriteResult::getIdxName() const
   {
      return _idxName ;
   }

   BSONObj utilWriteResult::getIdxKeyPattern() const
   {
      return _idxKeyPattern ;
   }

   BSONObj utilWriteResult::getIdxValue() const
   {
      return _idxValue ;
   }

   INT32 utilWriteResult::setDupErrInfo( const CHAR *idxName,
                                         const BSONObj& idxKeyPattern,
                                         const BSONObj& idxValue,
                                         const BSONObj& curObj )
   {
      INT32 rc = SDB_OK ;

      if ( !isMaskEnabled( UTIL_RESULT_MASK_DUP ) )
      {
         goto done ;
      }

      try
      {
         _idxName = idxName ;
         _idxKeyPattern = idxKeyPattern.getOwned() ;
         _idxValue = idxValue.getOwned() ;

         BSONObjBuilder curBuilder( 20 ) ;
         BSONElement e = curObj.getField( DMS_ID_KEY_NAME ) ;
         if ( !e.eoo() )
         {
            curBuilder.append( e ) ;
            _curID = curBuilder.obj() ; 
         }
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Build insert error info occur exception: %s",
                 e.what() ) ;
         rc = SDB_OOM ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 utilWriteResult::setCurrentID( const BSONObj &obj )
   {
      INT32 rc = SDB_OK ;

      if ( isMaskEnabled( UTIL_RESULT_MASK_ID ) )
      {
         try
         {
            BSONObjBuilder builder( 20 ) ;
            BSONElement e = obj.getField( DMS_ID_KEY_NAME ) ;
            if ( !e.eoo() )
            {
               builder.append( e ) ;
               _curID = builder.obj() ; 
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Build current _id occur exception: %s",
                    e.what() ) ;
         }
      }
      return rc ;
   }

   INT32 utilWriteResult::setPeerID( const BSONObj &obj )
   {
      INT32 rc = SDB_OK ;

      if ( isMaskEnabled( UTIL_RESULT_MASK_ID ) )
      {
         try
         {
            BSONObjBuilder builder( 20 ) ;
            BSONElement e = obj.getField( DMS_ID_KEY_NAME ) ;
            if ( !e.eoo() )
            {
               builder.append( e ) ;
               _peerID = builder.obj() ; 
            }
         }
         catch ( std::exception &e )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Build peer _id occur exception: %s",
                    e.what() ) ;
         }
      }
      return rc ;
   }

   void utilWriteResult::setCurRID( const dmsRecordID &rid )
   {
      _curRID = rid ;
   }

   void utilWriteResult::setPeerRID( const dmsRecordID &rid )
   {
      _peerRID = rid ;
   }

   BSONObj utilWriteResult::getCurID() const
   {
      return _curID ;
   }

   BSONObj utilWriteResult::getPeerID() const
   {
      return _peerID ;
   }

   const dmsRecordID& utilWriteResult::getCurRID() const
   {
      return _curRID ;
   }

   const dmsRecordID& utilWriteResult::getPeerRID() const
   {
      return _peerRID ;
   }

   void utilWriteResult::setDupErrInfo( const utilWriteResult *pResult )
   {
      if ( isMaskEnabled( UTIL_RESULT_MASK_DUP ) )
      {
         _idxName = pResult->getIdxName() ;
         _idxKeyPattern = pResult->getIdxKeyPattern() ;
         _idxValue = pResult->getIdxValue() ;
         _curID = pResult->getCurID() ;
         _peerID = pResult->getPeerID() ;
         _curRID = pResult->getCurRID() ;
         _peerRID = pResult->getPeerRID() ;
      }
   }

   /*
      utilIdxDupErrAssit implement
   */
   utilIdxDupErrAssit::utilIdxDupErrAssit( const BSONObj &idxKeyPattern,
                                           const BSONObj &idxValue )
   {
      _idxKeyPattern = idxKeyPattern ;
      _idxValue = idxValue ;
   }

   utilIdxDupErrAssit::~utilIdxDupErrAssit()
   {
   }

   INT32 utilIdxDupErrAssit::getIdxMatcher( BSONObj &idxMatcher,
                                            BOOLEAN cvtUndefined )
   {
      INT32 rc = SDB_OK ;

      if ( _idxKeyPattern.isEmpty() || _idxValue.isEmpty() )
      {
         PD_LOG( PDERROR, "Key pattern or value is empty" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      try
      {
         BSONObjBuilder builder( _idxKeyPattern.objsize() +
                                 _idxValue.objsize() ) ;

         BSONObjIterator itrK( _idxKeyPattern ) ;
         BSONObjIterator itrV( _idxValue ) ;

         while( itrK.more() && itrV.more() )
         {
            BSONElement eK = itrK.next() ;
            BSONElement eV = itrV.next() ;

            if ( !cvtUndefined || Undefined != eV.type() )
            {
               builder.appendAs( eV, eK.fieldName() ) ;
            }
            else
            {
               BSONObjBuilder sub( builder.subobjStart( eK.fieldName() ) ) ;
               sub.append( "$exists", 0 ) ;
               sub.done() ;
            }
         }

         if ( itrK.more() || itrV.more() )
         {
            PD_LOG( PDERROR, "Key value[%s] is not match the key pattern[%s]",
                    _idxValue.toString().c_str(),
                    _idxKeyPattern.toString().c_str() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         idxMatcher = builder.obj() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Builder index matcher occur exception: %s",
                 e.what() ) ;
         rc = SDB_OOM ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }


}

