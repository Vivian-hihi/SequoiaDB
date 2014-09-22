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

   Source File Name = rtnLobDataPool.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/08/2014  YW Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnLobDataPool.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include <algorithm>

using namespace std ;

namespace engine
{
   _rtnLobDataPool::_rtnLobDataPool()
   :_buf( NULL ),
    _bufSz( 0 ),
    _dataSz( 0 ),
    _current( -1 )
   {

   }

   _rtnLobDataPool::~_rtnLobDataPool()
   {
      clear() ;

      if ( NULL != _buf )
      {
         SDB_OSS_FREE( _buf ) ;
         _buf = NULL ;
         _bufSz = 0 ;
      }
   }

   INT32 _rtnLobDataPool::allocate( UINT32 len, CHAR **buf )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != buf, "can not be null" ) ;
      
      if ( len <= _bufSz )
      {
         *buf = _buf ;
         goto done ;
      }
      else if ( NULL != _buf )
      {
         SDB_OSS_FREE( _buf ) ;
         _bufSz = 0 ;
      }

      _buf = ( CHAR * )SDB_OSS_MALLOC( len ) ;
      if ( NULL == _buf )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ; 
         rc = SDB_OOM ;
         goto error ;
      }
      _bufSz = len ;
      *buf = _buf ;
   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnLobDataPool::entrust( CHAR *buf )
   {
      _toBeFreed.push_back( buf ) ;
      return ;
   }

   BOOLEAN _rtnLobDataPool::next( UINT32 len, const CHAR **buf, UINT32 &read )
   {
      BOOLEAN res = FALSE ;

      if ( 0 <= _current )
      {
         SDB_ASSERT( _pool.size() > ( UINT32 )_current, "impossible" ) ;
         tuple &t = _pool[_current] ;
         UINT32 realLen = len <= t.len ? len : t.len ;
         *buf = t.data ;
         read = realLen ;
         t.len -= realLen ;
         _dataSz -= realLen ;
         res = TRUE ;

         if ( 0 == t.len )
         {
            ++_current ;
            if ( _pool.size() == ( UINT32 )_current )
            {
               _current = -1 ;
               SDB_ASSERT( 0 == _dataSz, "must be zero" ) ;
            }
         }
         else
         {
            t.data += realLen ;
            t.offset += realLen ;
         }
      }

      return res ;
   }

   INT32 _rtnLobDataPool::push( const CHAR *data, UINT32 len,
                                SINT64 offset )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != data, "can not be null" ) ;
      _pool.push_back( tuple( offset, len, data ) ) ;
      _dataSz += len ;
   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnLobDataPool::pushDone()
   {
      if ( 1 < _pool.size() )
      {
         std::sort( _pool.begin(), _pool.end(), compare() ) ;
      }
#if defined (_DEBUG)
      SINT64 offset = -1 ;
      vector<tuple>::const_iterator itr = _pool.begin() ;
      for ( ; itr != _pool.end(); ++itr )
      {
         if ( -1 == offset )
         {
            offset = itr->offset + itr->len ;
         }
         else
         {
            SDB_ASSERT( offset == itr->offset, "impossible" ) ;
            offset += itr->len ;
         }
      }
#endif
      _current = 0 ;
      return ;
   }

   void _rtnLobDataPool::clear()
   {
      _pool.clear() ;
      _dataSz = 0 ;
      _current = -1 ;
      std::list<CHAR *>::iterator itr = _toBeFreed.begin() ;
      for ( ; itr != _toBeFreed.end(); ++itr )
      {
         SDB_OSS_FREE( *itr ) ;
      }
      _toBeFreed.clear() ;
      return ;
   }
}

