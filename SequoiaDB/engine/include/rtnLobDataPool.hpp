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

   Source File Name = rtnLobDataPool.hpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/08/2014  YW Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_LOBDATAPOOL_HPP_
#define RTN_LOBDATAPOOL_HPP_

#include "core.hpp"
#include "oss.hpp"
#include <vector>
#include <list>

namespace engine
{
   class _rtnLobDataPool : public SDBObject
   {
   public:
      _rtnLobDataPool() ;
      virtual ~_rtnLobDataPool() ;

   public:
      struct tuple
      {
         SINT64 offset ;
         UINT32 len ;
         const CHAR *data ;

         tuple( SINT64 o,
                UINT32 l,
                const CHAR *d )
         :offset( o ),
          len( l ),
          data( d )
         {

         }
      } ;

   public:
      /// buf will be freed by pool
      /// the last buf allocated will be invalid when allocate mem.
      INT32 allocate( UINT32 len, CHAR **buf ) ;

      /// realBuf will be freed when do clear
      INT32 push( const CHAR *data, UINT32 len,
                  SINT64 offset ) ;

      void pushDone() ;

      void entrust( CHAR *buf ) ;

      BOOLEAN empty() const
      {
         return 0 == _dataSz ;
      }

      UINT32 getDataSize() const
      {
         return _dataSz ;
      }

      BOOLEAN match( SINT64 offset ) const
      {
         if ( !_pool.empty() )
         {
            return offset == _pool[_current].offset ;
         }
         else
         {
            return FALSE ;
         }
      }

      BOOLEAN next( UINT32 len, const CHAR **data, UINT32 &read ) ;

      void clear() ;

   private:
      struct compare
      {
         BOOLEAN operator()( const tuple &l, const tuple &r )
         {
            return l.offset < r.offset ;
         }
      } ;

      CHAR *_buf ;
      UINT32 _bufSz ;
      std::vector<tuple> _pool ;
      std::list<CHAR *> _toBeFreed ;
      UINT32 _dataSz ;
      SINT32 _current ;
      friend class _rtnLobStream ;
   } ;
   typedef class _rtnLobDataPool rtnLobDataPool ;
}

#endif

