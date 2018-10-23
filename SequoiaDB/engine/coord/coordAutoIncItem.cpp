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

   Source File Name = coordAutoIncItem.cpp

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

#include "coordAutoIncItem.hpp"
#include "pd.hpp"
#include "ossTypes.h"
#include "ossMem.hpp"
#include "msgCatalogDef.h"
#include <map>

namespace engine

{
   /*
      implement coordAutoIncItem
   */
   _coordAutoIncItem::_coordAutoIncItem()
   {
      _fieldName = NULL ;
      _sequenceName = NULL ;
      _generatedType = AUTOINC_GEN_DEFAULT ;
      _pSubFieldMap = NULL ;
   }

   _coordAutoIncItem::~_coordAutoIncItem()
   {
      if ( _pSubFieldMap )
      {
         delete _pSubFieldMap ;
         _pSubFieldMap = NULL ;
      }
   }

   _coordAutoIncItem::_coordAutoIncItem( const _coordAutoIncItem &that )
   {
      _pSubFieldMap = NULL ;
      *this = that ;
   }

   _coordAutoIncItem& _coordAutoIncItem::operator=( const _coordAutoIncItem &that )
   {
      _fieldName     = that._fieldName ;
      _sequenceName  = that._sequenceName ;
      _sequenceID    = that._sequenceID ;
      _generatedType = that._generatedType ;
      _fldNameBuf    = that._fldNameBuf ;

      if ( _pSubFieldMap )
      {
         delete _pSubFieldMap ;
         _pSubFieldMap = NULL ;
      }

      if ( that._pSubFieldMap )
      {
         _pSubFieldMap = new AUTOINC_ITEM_MAP ;
         *_pSubFieldMap = *(that._pSubFieldMap) ;
      }

      return *this ;
   }

   INT32 _coordAutoIncItem::init( const bson::BSONObj &obj )
   {
      INT32             rc = SDB_OK ;
      const CHAR*       pFldName = NULL ;
      const CHAR*       pSeqName = NULL ;
      bson::OID         seqID ;
      const CHAR*       pGenStr = NULL ;
      AUTOINC_GEN_TYPE  genType ;

      pFldName = obj.getField( CAT_AUTOINC_FIELD ).valuestr() ;
      pSeqName = obj.getField( CAT_AUTOINC_SEQ ).valuestr() ;
      seqID = obj.getField( CAT_AUTOINC_SEQ_ID ).OID() ;
      pGenStr = obj.getField( CAT_AUTOINC_GENERATED ).valuestr() ;

      if ( 0 == ossStrcmp( CAT_GENERATED_ALWAYS, pGenStr ) )
      {
         genType = AUTOINC_GEN_ALWAYS ;
      }
      else if ( 0 == ossStrcmp( CAT_GENERATED_STRICT, pGenStr ) )
      {
         genType = AUTOINC_GEN_STRICT ;
      }
      else if ( 0 == ossStrcmp( CAT_GENERATED_DEFAULT, pGenStr ) )
      {
         genType = AUTOINC_GEN_DEFAULT ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Unknown generated type[%s]", pGenStr ) ;
         goto error ;
      }

      rc = init( pFldName, pSeqName, seqID, genType ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _coordAutoIncItem::init( const CHAR* fieldName,
                                  const CHAR* sequenceName,
                                  const bson::OID &sequenceID,
                                  const AUTOINC_GEN_TYPE generated )
   {
      /*
         If fieldName is simple, result will be like
         {
            field: "a",
            sequence: "SYS_xxx_a_SEQ",
            generated: "always",
            subFields: <unused>
         }.
         If fieldName is nested, like "a.b", result will be like
         {
            field: "a",
            sequence: <unused>,
            generated: <unused>,
            subFields: [
               {
                  field: "b",
                  sequence: "SYS_xxx_a.b_SEQ",
                  generated: "always",
                  subFields: <unused>
               }
            ]
         }
      */

      INT32             rc = SDB_OK ;
      const CHAR*       pos = NULL ;
      INT32             strLen = 0 ;
      const CHAR*       subField = NULL ;
      _coordAutoIncItem subItem ;

      pos = ossStrchr( fieldName, '.' ) ;
      if ( NULL == pos )
      {
         _fieldName = fieldName ;
         _sequenceName = sequenceName ;
         _sequenceID = sequenceID ;
         _generatedType = generated ;
      }
      else
      {
         strLen = pos - fieldName + 1 ;
         _fldNameBuf = boost::shared_array<CHAR>( new(std::nothrow) CHAR[strLen] );
         PD_CHECK( _fldNameBuf.get(), SDB_OOM, error, PDERROR,
                   "Failed to malloc for field name, rc: %d", rc ) ;
         ossStrncpy( _fldNameBuf.get(), fieldName, strLen ) ;
         _fldNameBuf[ strLen - 1 ] = 0 ;

         _fieldName = _fldNameBuf.get() ;
         subField = pos + 1 ;
         rc = subItem.init( subField, sequenceName, sequenceID, generated ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to init sub auto-increment item, rc: %d", rc ) ;

         _pSubFieldMap = new(std::nothrow) AUTOINC_ITEM_MAP ;
         PD_CHECK( _pSubFieldMap, SDB_OOM, error, PDERROR,
                   "Failed to new sub field map, rc: %d", rc ) ;
         _pSubFieldMap->insert( AUTOINC_ITEM_MAP_VAL( subItem.fieldName(), subItem ) ) ;
      }

   done:
      return rc ;

   error:
      if ( _pSubFieldMap )
      {
         delete _pSubFieldMap ;
         _pSubFieldMap = NULL ;
      }
      goto done ;
   }

}

