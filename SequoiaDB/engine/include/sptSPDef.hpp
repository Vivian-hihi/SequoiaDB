/******************************************************************************


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

   Source File Name = sptSPDef.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_SPDEF_HPP_
#define SPT_SPDEF_HPP_

#include "core.hpp"
#include "jsapi.h"

namespace engine
{
   #define SAFE_JS_FREE( cx, p ) \
      do { if ( p ) { JS_free( ( cx ), ( p ) ) ; ( p ) = NULL ; } } while ( 0 )

   #define SPT_EVAL_FLAG_NONE    0
   #define SPT_EVAL_FLAG_PRINT   0x00000001

   #define SPT_ERR               "ErrMsg"

   /*
      SPT_PROP_ATTR define
   */
   /// visible to enumerate. for...in, for...each, JS_Enumerate
   #define SPT_PROP_ENUMERATE                JSPROP_ENUMERATE
   /// can't be set
   #define SPT_PROP_READONLY                 JSPROP_READONLY
   /// can't be deleted
   #define SPT_PROP_PERMANENT                JSPROP_PERMANENT

   #define SPT_PROP_DEFAULT                  (SPT_PROP_ENUMERATE|SPT_PROP_READONLY|SPT_PROP_PERMANENT)
   #define SPT_FUNC_DEFAULT                  (SPT_PROP_ENUMERATE)

}

#endif // SPT_SPDEF_HPP_

