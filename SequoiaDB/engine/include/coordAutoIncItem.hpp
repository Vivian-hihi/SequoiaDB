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

   Source File Name = coordAutoIncItem.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          27/08/2018  LSQ Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef COORD_AUTOINC_ITEM_HPP__
#define COORD_AUTOINC_ITEM_HPP__

#include "ossTypes.h"
#include "oss.hpp"
#include "ossUtil.hpp"
#include <map>

namespace engine
{
   /*
      define container of coordAutoIncItem
   */
   // TODO: common?
   struct cmpStr
   {
      BOOLEAN operator()( CHAR const *a, CHAR const *b ) const
      {
         return ossStrcmp( a, b ) < 0 ;
      }
   } ;
   class _coordAutoIncItem ;
   typedef std::map<const CHAR*, _coordAutoIncItem*, cmpStr>   AUTOINC_ITEM_MAP ;
   typedef AUTOINC_ITEM_MAP::iterator        AUTOINC_ITEM_MAP_IT ;
   typedef AUTOINC_ITEM_MAP::const_iterator  AUTOINC_ITEM_MAP_CONST_IT ;
   typedef AUTOINC_ITEM_MAP::value_type      AUTOINC_ITEM_MAP_VAL ;



   /*
      define generated type enum
   */
   enum _AUTOINC_GEN_TYPE
   {
      AUTOINC_GEN_ALWAYS = 0,
      AUTOINC_GEN_STRICT,
      AUTOINC_GEN_DEFAULT
   } ;
   typedef enum _AUTOINC_GEN_TYPE AUTOINC_GEN_TYPE ;



   /*
      define coordAutoIncItem
   */
   class _coordAutoIncItem : public SDBObject
   {
   public:
      _coordAutoIncItem() ;

      ~_coordAutoIncItem() ;

      INT32             init( const CHAR* fieldName,
                              const CHAR* sequenceName,
                              const AUTOINC_GEN_TYPE generated ) ;

      const CHAR*       fieldName() const { return _fieldName ; }

      const CHAR*       sequenceName() const { return _sequenceName ; }

      AUTOINC_GEN_TYPE  generatedType() const { return _generatedType ; }

      BOOLEAN           hasSubField() const { return (NULL != _pSubFieldMap) ; }

      AUTOINC_ITEM_MAP* subFieldMap() const { return _pSubFieldMap ; }

   private:
      CHAR*             _fieldName ;
      CHAR*             _sequenceName ;
      AUTOINC_GEN_TYPE  _generatedType ;
      AUTOINC_ITEM_MAP* _pSubFieldMap ;

   } ;
   typedef _coordAutoIncItem coordAutoIncItem ;

}

#endif //COORD_AUTOINC_ITEM_HPP__
