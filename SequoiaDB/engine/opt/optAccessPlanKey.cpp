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

   Source File Name = optAccessPlanKey.cpp

   Descriptive Name = Optimizer Access Plan Key

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains functions for key of
   access plan.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/17/2017  HGM Initial Draft

   Last Changed =

*******************************************************************************/

#include "optAccessPlanKey.hpp"
#include "pdTrace.hpp"
#include "optTrace.hpp"
#include "pmd.hpp"
#include "utilBsonHash.hpp"

using namespace bson ;

namespace engine
{

   /*
      _optAccessPlanKey implement
    */
   _optAccessPlanKey::_optAccessPlanKey ( const CHAR *pCLFullName,
                                          const BSONObj &selector,
                                          const BSONObj &matcher,
                                          const BSONObj &orderBy,
                                          const BSONObj &hint,
                                          SINT32 flags,
                                          SINT64 numToSkip,
                                          SINT64 numToReturn,
                                          BOOLEAN needGetOwned )
   : _rtnQueryOptions( matcher, selector, orderBy, hint, pCLFullName,
                       numToSkip, numToReturn, flags )
   {
      SDB_ASSERT( pCLFullName, "pCLFullName is invalid" ) ;

      if ( needGetOwned )
      {
         getOwned() ;
      }

      // Note: only the cached plan needs these properties
      _suID = DMS_INVALID_SUID ;
      _suLID = DMS_INVALID_LOGICCSID ;
      _clLID = DMS_INVALID_CLID ;
      _mbID = DMS_INVALID_MBID ;

      _isValid = FALSE ;
   }

   _optAccessPlanKey::_optAccessPlanKey ( const _optAccessPlanKey &planKey,
                                          BOOLEAN needGetOwned )
   : _rtnQueryOptions( planKey )
   {
      if ( needGetOwned )
      {
         getOwned() ;
      }

      _suID = planKey._suID ;
      _suLID = planKey._suLID ;
      _clLID = planKey._clLID ;
      _mbID = planKey._mbID ;

      // No need to re-calculate
      _keyCode = planKey._keyCode ;

      _isValid = FALSE ;
   }

   BOOLEAN _optAccessPlanKey::isEqual ( const _optAccessPlanKey &planKey ) const
   {
      if ( !_isValid || !planKey._isValid )
      {
         return FALSE ;
      }

      if ( _keyCode != planKey._keyCode )
      {
         return FALSE ;
      }

      // Check the IDs of Collection Space and Collection
      if ( _suID != planKey._suID || _suLID != planKey._suLID ||
           _mbID != planKey._mbID || _clLID != planKey._clLID )
      {
         return FALSE ;
      }

      // User query must be identical
      if ( !_query.shallowEqual( planKey._query ) )
      {
         return FALSE ;
      }

      // Order by must be identical
      if ( !_orderBy.shallowEqual( planKey._orderBy ) )
      {
         return FALSE ;
      }

      // Query with modifier should use index to sort
      if ( ( OSS_BIT_TEST( _flag, FLG_QUERY_MODIFY ) ||
             OSS_BIT_TEST( _flag, FLG_QUERY_FORCE_IDX_BY_SORT ) ) &&
           ( OSS_BIT_TEST( planKey._flag, FLG_QUERY_MODIFY ) ||
             OSS_BIT_TEST( planKey._flag, FLG_QUERY_FORCE_IDX_BY_SORT ) ) )
      {
         return FALSE ;
      }

      /// Hint must compare field by field, and need ignore object field and
      /// field name
      BSONObjIterator itr( planKey._hint ) ;
      BSONObjIterator itrSelf( _hint ) ;
      while( itr.more() )
      {
         BSONElement e2 ;
         BSONElement e1 = itr.next() ;
         if ( e1.isABSONObj() )
         {
            continue ;
         }

         while( itrSelf.more() )
         {
            e2 = itrSelf.next() ;
            if ( e2.isABSONObj() )
            {
               continue ;
            }
            break ;
         }

         if ( 0 != e1.woCompare( e2, false ) )
         {
            return FALSE ;
         }
      }

      /// If _hint has other hint field, not the same
      while( itrSelf.more() )
      {
         BSONElement e = itrSelf.next() ;
         if ( !e.isABSONObj() )
         {
            return FALSE ;
         }
      }

      return TRUE ;
   }

   void _optAccessPlanKey::_generateKeyCodeInternal ()
   {
      setKeyCode( _generateKeyCodeHash() ) ;
   }

   UINT32 _optAccessPlanKey::_generateKeyCodeHash ()
   {
      UINT32 keyCode = 0 ;

      // Information of collection space and collection
      keyCode = ossHash( (CHAR *)&_suID, sizeof( _suID ), 5 ) ;
      keyCode ^= ossHash( (CHAR *)&_suLID, sizeof( _suLID ), 5 ) ;
      keyCode ^= ossHash( (CHAR *)&_mbID, sizeof( _mbID ), 5 ) ;
      keyCode ^= ossHash( (CHAR *)&_clLID, sizeof( _clLID ), 5 ) ;

      // Query
      if ( !_query.isEmpty() )
      {
         keyCode ^= ossHash( _query.objdata(), _query.objsize() ) ;
      }

      // Order-By
      if ( !_orderBy.isEmpty() )
      {
         keyCode ^= ossHash( _orderBy.objdata(), _orderBy.objsize() ) ;
      }

      // Hint
      BSONObjIterator itr( _hint ) ;
      while( itr.more() )
      {
         BSONElement e = itr.next() ;
         if ( e.isABSONObj() )
            continue ;
         keyCode ^= ossHash( e.value(), e.valuesize() ) ;
      }

      return keyCode ;
   }

   UINT32 _optAccessPlanKey::_generateKeyCodeMD5 ()
   {
      UINT32 keyCode = _utilBSONHasher::hashStr( _fullName ) ;
      UINT32 matcherCode = _utilBSONHasher::hashObj( _query ) ;

      keyCode = _utilBSONHasher::hashCombine( keyCode, matcherCode ) ;

      if ( !_orderBy.isEmpty() )
      {
         UINT32 orderByCode = _utilBSONHasher::hashObj( _orderBy ) ;
         keyCode = _utilBSONHasher::hashCombine( keyCode, orderByCode ) ;
      }

      if ( !_hint.isEmpty() )
      {
         BSONObjIterator itr( _hint ) ;
         while( itr.more() )
         {
            BSONElement e = itr.next() ;
            UINT32 hintCode = 0 ;
            if ( e.isABSONObj() )
            {
               continue ;
            }
            hintCode = _utilBSONHasher::hashElement( e ) ;
            keyCode = _utilBSONHasher::hashCombine( keyCode, hintCode ) ;
         }
      }

      return keyCode ;
   }

}

