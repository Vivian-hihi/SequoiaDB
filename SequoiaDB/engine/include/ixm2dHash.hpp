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

   Source File Name = ixm2dHash.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Index Manager component. This file contains functions for index
   key generator, which is used to create key pairs from data record and index
   definition.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef IXM2DHASH_HPP_
#define IXM2DHASH_HPP_

#include "core.hpp"
#include "oss.hpp"
namespace engine
{
   class _ixm2dHash : public SDBObject
   {
   public:
      _ixm2dHash( UINT32 x,
                  UINT32 y ):
      _x( x ),
      _y( y ),
      _hash( 0 )
      {
         _fill() ;
      }

      _ixm2dHash( const _ixm2dHash &hash ):
      _x( hash._x ),
      _y( hash._y ),
      _hash( hash._hash )
      {

      }

   const _ixm2dHash &operator=( const _ixm2dHash &hash )
   {
      _x = hash._x ;
      _y = hash._y ;
      _hash = hash._hash ;
      return *this ;
   }

   const UINT64 &hash()
   {
      return _hash ;
   }

   private:
      void _fill() ;

   private:
      UINT32 _x ;
      UINT32 _y ;
      UINT64 _hash ;
   } ;

   typedef class _ixm2dHash geoHash ;
}

#endif

