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

   Source File Name = utilPooledAutoPtr.hpp

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
#ifndef UTIL_POOLED_AUTO_PTR_HPP__
#define UTIL_POOLED_AUTO_PTR_HPP__

#include "ossTypes.hpp"

namespace engine
{

   /*
      _utilPooledAutoPtr define
   */
   class _utilPooledAutoPtr
   {
      public:
         _utilPooledAutoPtr() ;
         _utilPooledAutoPtr( const _utilPooledAutoPtr &rhs ) ;
         ~_utilPooledAutoPtr() ;

         _utilPooledAutoPtr& operator= ( const _utilPooledAutoPtr &rhs ) ;
         bool operator! () const { return get() ? false : true ; }

         operator bool () { return get() ? true : false ; }
         operator CHAR* () { return get () ; }
         operator BOOLEAN () { return get() ? TRUE : FALSE ; }
         operator const CHAR* () { return get() ; }

         static _utilPooledAutoPtr alloc( UINT32 size,
                                          const CHAR *pFile,
                                          UINT32 line ) ;

      public:
         CHAR*       get() ;
         const CHAR* get() const ;
         INT32       refCount() const ;
         void        release() ;

      private:
         _utilPooledAutoPtr( CHAR *ptr ) ;

      protected:
         INT32*      _refPtr() ;

      private:
         CHAR        *_ptr ;
   } ;
   typedef _utilPooledAutoPtr utilPooledAutoPtr ;

}

#endif // UTIL_POOLED_AUTO_PTR_HPP__

