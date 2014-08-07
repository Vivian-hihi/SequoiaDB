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

   Source File Name = omagentUtil.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_UTIL_HPP_
#define OMAGENT_UTIL_HPP_

#include "core.hpp"
#include "ossUtil.hpp"
#include "../bson/bson.hpp"
#include "sptApi.hpp"

using namespace bson ;

namespace engine
{

   INT32 checkBuffer ( CHAR **ppBuffer, INT32 *bufferSize,
                       INT32 packetLength ) ;

   INT32 readFile ( const CHAR * name , CHAR ** buf , UINT32 * bufSize,
                    UINT32 * readSize ) ;

   // get spider monkey engine
   INT32 getSptScope ( _sptScope **scope ) ;

   // get bson field
   INT32 omaGetIntElement ( const BSONObj &obj, const CHAR *fieldName,
                            INT32 &value ) ;

   INT32 omaGetStringElement ( const BSONObj &obj, const CHAR *fieldName,
                               const CHAR **value ) ;

   INT32 omaGetObjElement ( const BSONObj &obj, const CHAR *fieldName,
                            BSONObj &value ) ;

   INT32 omaGetBooleanElement ( const BSONObj &obj, const CHAR *fieldName,
                                BOOLEAN &value ) ;

}

#endif // OMAGENT_UTIL_HPP_


