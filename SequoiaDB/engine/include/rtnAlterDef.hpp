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

   Source File Name = rtnAlterDef.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/05/2015  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_ALTERDEF_HPP_
#define RTN_ALTERDEF_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.hpp"

namespace engine
{
   class _pmdEDUCB ;
   class _dpsLogWrapper ;

   enum RTN_ALTER_TYPE
   {
      RTN_ALTER_INVALID = -1,
      RTN_ALTER_TYPE_DB = 0,
      RTN_ALTER_TYPE_CL,
      RTN_ALTER_TYPE_CS,
      RTN_ALTER_TYPE_DOMAIN,
      RTN_ALTER_TYPE_GROUP,
      RTN_ALTER_TYPE_NODE
   } ;

   struct _rtnAlterOptions
   {
      /// ignore one alter's exception and continue to run next.
      BOOLEAN ignoreException ;

      _rtnAlterOptions()
      :ignoreException( FALSE )
      {

      }

      void reset()
      {
         ignoreException = FALSE ;
      }
   } ;

   typedef INT32 ( *RTN_ALTER_FUNC )( const CHAR *name,
                                      const bson::BSONObj &pubArgs,
                                      const bson::BSONObj &args,
                                      _pmdEDUCB *cb,
                                      _dpsLogWrapper *dpsCB ) ;
}

#endif

