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

   Source File Name = utilPooledAutoPtr.cpp

   Descriptive Name = Operating System Services Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for OSS operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/13/2019  XJH  Initial Draft

   Last Changed =

*******************************************************************************/

#include "utilPooledAutoPtr.hpp"
#include "utilMemBlockPool.hpp"
#include "ossAtomicBase.hpp"
#include "ossMem.hpp"

namespace engine
{

   /*
      _utilPooledAutoPtr implement
   */
   _utilPooledAutoPtr::_utilPooledAutoPtr()
   {
      _ptr = NULL ;
   }

   _utilPooledAutoPtr::_utilPooledAutoPtr( CHAR *ptr )
   {
      _ptr = ptr ;
      if ( _ptr )
      {
         INT32 orgRef = ossFetchAndIncrement32( _refPtr() ) ;
         SDB_ASSERT( orgRef >= 0, "Ref is invlaid" ) ;
         SDB_UNUSED( orgRef ) ;
      }
   }

   _utilPooledAutoPtr::_utilPooledAutoPtr( const _utilPooledAutoPtr &rhs )
   {
      _ptr = rhs._ptr ;
      if ( _ptr )
      {
         INT32 orgRef = ossFetchAndIncrement32( _refPtr() ) ;
         SDB_ASSERT( orgRef >= 0, "Ref is invlaid" ) ;
         SDB_UNUSED( orgRef ) ;
      }
   }

   _utilPooledAutoPtr::~_utilPooledAutoPtr()
   {
      release() ;
   }

   _utilPooledAutoPtr& _utilPooledAutoPtr::operator= ( const _utilPooledAutoPtr &rhs )
   {
      release() ;
      _ptr = rhs._ptr ;
      if ( _ptr )
      {
         INT32 orgRef = ossFetchAndIncrement32( _refPtr() ) ;
         SDB_ASSERT( orgRef >= 0, "Ref is invlaid" ) ;
         SDB_UNUSED( orgRef ) ;
      }
      return *this ;
   }

   _utilPooledAutoPtr _utilPooledAutoPtr::alloc( UINT32 size )
   {
      _utilPooledAutoPtr recordPtr ;
      if ( size > 0 )
      {
         UINT32 realSZ = size + sizeof( INT32 ) ;
         CHAR *ptr = NULL ;

         ptr = ( CHAR* )utilPoolAlloc( realSZ ) ;
         if ( ptr )
         {
            *(INT32*)ptr = 1 ;
            recordPtr._ptr = ptr ;
         }
      }
      return recordPtr ;
   }

   CHAR* _utilPooledAutoPtr::get()
   {
      return _ptr ? _ptr + sizeof( INT32 ) : NULL ;
   }

   const CHAR* _utilPooledAutoPtr::get() const
   {
      return _ptr ? _ptr + sizeof( INT32 ) : NULL ;
   }

   INT32 _utilPooledAutoPtr::refCount() const
   {
      return _ptr ? *((INT32*)_ptr) : 0 ;
   }

   INT32* _utilPooledAutoPtr::_refPtr()
   {
      return _ptr ? (INT32*)_ptr : NULL ;
   }

   void _utilPooledAutoPtr::release()
   {
      if ( _ptr )
      {
         INT32 orgRef = ossFetchAndDecrement32( _refPtr() ) ;
         SDB_ASSERT( orgRef >= 1, "Ref is invlaid" ) ;
         if ( 1 == orgRef )
         {
            utilPoolRelease( (void*&)_ptr ) ;
            _ptr = NULL ;
         }
      }
   }

}

