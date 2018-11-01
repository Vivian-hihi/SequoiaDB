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

   Source File Name = clsAutoIncItem.cpp

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

#include "clsAutoIncItem.hpp"
#include "msgCatalogDef.h"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

using namespace bson ;

namespace engine

{
   /*
      implement _clsAutoIncItem
   */
   _clsAutoIncItem::_clsAutoIncItem()
   {
      _fieldName     = NULL ;
      _sequenceName  = NULL ;
      _generatedType = AUTOINC_GEN_DEFAULT ;
      _pSubFieldMap  = NULL ;
   }

   _clsAutoIncItem::~_clsAutoIncItem()
   {
      _clear() ;
   }

   void _clsAutoIncItem::_clear()
   {
      if ( _pSubFieldMap )
      {
         SDB_OSS_DEL _pSubFieldMap ;
         _pSubFieldMap = NULL ;
      }

      _fieldName     = NULL ;
      _sequenceName  = NULL ;
      _fieldStr.clear() ;
   }

   INT32 _clsAutoIncItem::init( const bson::BSONObj &obj )
   {
      INT32             rc = SDB_OK ;
      const CHAR*       pFldName = NULL ;
      const CHAR*       pSeqName = NULL ;
      utilSequenceID    seqID = UTIL_SEQUENCEID_NULL ;
      const CHAR*       pGenStr = NULL ;
      AUTOINC_GEN_TYPE  genType ;
      UINT32            validNum = 0 ;

      BSONObjIterator itr( obj ) ;
      while( itr.more() )
      {
         BSONElement e = itr.next() ;

         if ( 0 == ossStrcmp( e.fieldName(), CAT_AUTOINC_FIELD ) )
         {
            if ( String != e.type() )
            {
               PD_LOG( PDERROR, "Field[%s] in obj[%s] must be string",
                       CAT_AUTOINC_FIELD, obj.toString().c_str() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            pFldName = e.valuestr() ;
            ++validNum ;
         }
         else if ( 0 == ossStrcmp( e.fieldName(), CAT_AUTOINC_SEQ ) )
         {
            if ( String != e.type() )
            {
               PD_LOG( PDERROR, "Field[%s] in obj[%s] must be string",
                       CAT_AUTOINC_SEQ, obj.toString().c_str() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            pSeqName = e.valuestr() ;
            ++validNum ;
         }
         else if ( 0 == ossStrcmp( e.fieldName(), CAT_AUTOINC_SEQ_ID ) )
         {
            if ( !e.isNumber() )
            {
               PD_LOG( PDERROR, "Field[%s] in obj[%s] must be number",
                       CAT_AUTOINC_SEQ_ID, obj.toString().c_str() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            seqID = e.Long() ;
            ++validNum ;
         }
         else if ( 0 == ossStrcmp( e.fieldName(), CAT_AUTOINC_GENERATED ) )
         {
            if ( String != e.type() )
            {
               PD_LOG( PDERROR, "Field[%s] in obj[%s] must be OID",
                       CAT_AUTOINC_GENERATED, obj.toString().c_str() ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            pGenStr = e.valuestr() ;
            ++validNum ;
         }
      }

      /// check valid num
      if ( validNum < 4 )
      {
         PD_LOG( PDERROR, "Object[%s] is not valid", obj.toString().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

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

      rc = _init( pFldName, pSeqName, seqID, genType ) ;
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsAutoIncItem::_init( const CHAR* fieldName,
                                 const CHAR* sequenceName,
                                 const utilSequenceID sequenceID,
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
      const CHAR*       subField = NULL ;

      _clsAutoIncItem    *pSubItem = NULL ;

      subField = ossStrchr( fieldName, '.' ) ;
      if ( NULL == subField )
      {
         _fieldName     = fieldName ;
         _sequenceName  = sequenceName ;
         _sequenceID    = sequenceID ;
         _generatedType = generated ;
      }
      else
      {
         UINT32 strLen = subField - fieldName + 1 ;
         _fieldStr.assign( fieldName, strLen ) ;
         _fieldName = _fieldStr.c_str() ;

         subField++ ;

         pSubItem = SDB_OSS_NEW _clsAutoIncItem() ;
         if ( !pSubItem )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Alloc item failed" ) ;
            goto error ;
         }

         rc = pSubItem->_init( subField, sequenceName,
                               sequenceID, generated ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Init sub item failed, rc: %d", rc ) ;
            goto error ;
         }

         _pSubFieldMap = SDB_OSS_NEW AUTOINC_ITEM_MAP() ;
         if ( !_pSubFieldMap )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Alloc sub field map failed" ) ;
            goto error ;
         }
         /// add to map
         _pSubFieldMap->insert( AUTOINC_ITEM_MAP_VAL( pSubItem->fieldName(),
                                                      pSubItem ) ) ;
         pSubItem = NULL ;
      }

   done:
      if ( pSubItem )
      {
         SDB_OSS_DEL pSubItem ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsAutoIncItem::merge( _clsAutoIncItem *pItem )
   {
      /*
         If field exists, merge their sub fields.
         e.g:
         {field: "a", subFields:[{field: "b"}]} +
         {field: "a", subFields:[{field: "c"}]} =>
         [{field: "a", subFields:[{field: "b"}, {field: "c"}]}]
      */

      INT32 rc = SDB_OK ;

      if ( _pSubFieldMap && pItem->_pSubFieldMap )
      {
         _clsAutoIncItem *pItemSub = NULL ;
         _clsAutoIncItem *pSelfSub = NULL ;
         AUTOINC_ITEM_MAP_IT itSelfSub ;
         AUTOINC_ITEM_MAP_IT itItemSub = pItem->_pSubFieldMap->begin() ;
         while( itItemSub != pItem->_pSubFieldMap->end() )
         {
            pItemSub = itItemSub->second ;
            itSelfSub = _pSubFieldMap->find( pItemSub->fieldName() ) ;

            if ( itSelfSub == _pSubFieldMap->end() )
            {
               /// Insert
               _pSubFieldMap->insert( AUTOINC_ITEM_MAP_VAL( pItemSub->fieldName(),
                                                            pItemSub ) ) ;
               /// remove from pItem's sub item
               itItemSub = pItem->_pSubFieldMap->erase( itItemSub ) ;
            }
            else
            {
               pSelfSub = itSelfSub->second ;

               rc = pSelfSub->merge( pItemSub ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Merge auto-increment item[%s] to [%s] "
                          "failed, rc: %d", pItemSub->fieldName(),
                          pSelfSub->fieldName(), rc ) ;
                  goto error ;
               }

               ++itItemSub ;
            }
         }
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "AutoIncrement fields[%s, %s] conflict.",
                 fieldName(), pItem->fieldName() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   const _clsAutoIncItem* _clsAutoIncItem::findSubItem( const CHAR *pName ) const
   {
      if ( _pSubFieldMap )
      {
         AUTOINC_ITEM_MAP_CONST_IT cit = _pSubFieldMap->find( pName ) ;
         if ( cit != _pSubFieldMap->end() )
         {
            return cit->second ;
         }
      }
      return NULL ;
   }

   /*
      _clsAutoIncSet implement
   */
   _clsAutoIncSet::_clsAutoIncSet()
   {
      _totalCount = 0 ;
   }

   _clsAutoIncSet::~_clsAutoIncSet()
   {
      _clear() ;
   }

   void _clsAutoIncSet::_clear()
   {
      AUTOINC_ITEM_MAP_IT it = _mapItem.begin() ;
      while( it != _mapItem.end() )
      {
         SDB_OSS_DEL it->second ;
         ++it ;
      }
      _mapItem.clear() ;

      _objInfo = BSONObj() ;
      _totalCount = 0 ;

      _vecFields.clear() ;
   }

   void _clsAutoIncSet::clear()
   {
      _clear() ;
   }

   const clsAutoIncItem* _clsAutoIncSet::findItem( const CHAR *pName ) const
   {
      AUTOINC_ITEM_MAP_CONST_IT cit = _mapItem.find( pName ) ;
      if ( cit == _mapItem.end() )
      {
         return NULL ;
      }
      return cit->second ;
   }

   INT32 _clsAutoIncSet::init( const BSONElement &ele )
   {
      INT32 rc = SDB_OK ;

      if ( Array == ele.type() )
      {
         _objInfo = ele.embeddedObject().getOwned() ;

         BSONObjIterator itr( _objInfo ) ;
         while( itr.more() )
         {
            BSONElement e = itr.next() ;
            if ( Object != e.type() )
            {
               rc = SDB_SYS ;
               goto error ;
            }

            rc = _initAItem( e.embeddedObject() ) ;
            if ( rc )
            {
               goto error ;
            }
         }
      }
      else if ( Object == ele.type() )
      {
         _objInfo = ele.embeddedObject().getOwned() ;

         rc = _initAItem( _objInfo ) ;
         if ( rc )
         {
            goto error ;
         }
      }
      else
      {
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsAutoIncSet::_initAItem( const BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      clsAutoIncItem *pItem = NULL ;
      AUTOINC_ITEM_MAP_IT it ;

      pItem = SDB_OSS_NEW clsAutoIncItem() ;
      if ( !pItem )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "Alloc item failed" ) ;
         goto error ;
      }

      rc = pItem->init( obj ) ;
      if ( rc )
      {
         goto error ;
      }

      /// Find first
      it = _mapItem.find( pItem->fieldName() ) ;
      if ( it != _mapItem.end() )
      {
         clsAutoIncItem *pTmpItem = it->second ;

         /// merge
         rc = pTmpItem->merge( pItem ) ;
         if ( rc )
         {
            goto error ;
         }
      }
      else
      {
         _mapItem.insert( AUTOINC_ITEM_MAP_VAL( pItem->fieldName(), pItem ) ) ;
         pItem = NULL ;
      }

      ++_totalCount ;
      _vecFields.push_back( obj ) ;

   done:
      if ( pItem )
      {
         SDB_OSS_DEL pItem ;
      }
      return rc ;
   error:
      goto done ;
   }

}

