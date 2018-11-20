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

#include <utilStr.hpp>
#include "clsAutoIncItem.hpp"
#include "msgCatalogDef.h"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "clsTrace.hpp"

using namespace bson ;

namespace engine
{
   /*
      implement _clsAutoIncIterator
   */
   _clsAutoIncIterator::_clsAutoIncIterator( const _clsAutoIncSet &set,
                                             const MODE mode )
   {
      _pMap = &set._mapItem ;
      _mode = mode ;
      _it = _pMap->begin() ;
   }

   _clsAutoIncIterator::_clsAutoIncIterator( const _clsAutoIncItem &item,
                                             const MODE mode )
   {
      _pMap = item._pSubFieldMap ;
      _mode = mode ;
      if ( _pMap )
      {
         _it = _pMap->begin();
      }
   }

   BOOLEAN _clsAutoIncIterator::more()
   {
      if ( !_pMap || _it == _pMap->end() )
         return FALSE ;
      else
         return TRUE ;
   }

   const clsAutoIncItem* _clsAutoIncIterator::next()
   {
      const clsAutoIncItem* pItem = NULL ;

      if ( !_pMap || _it == _pMap->end() )
      {
         return NULL ;
      }

      if ( NON_RECURS == _mode )
      {
         pItem = _it->second ;
         ++_it ;
      }
      else if ( RECURS == _mode )
      {
         // if is not leaf, go forward until the leaf.
         pItem = _it->second ;
         while ( NULL != pItem->_pSubFieldMap )
         {
            _mapTrace.push_back( _pMap ) ;
            _itTrace.push_back( _it ) ;

            _pMap = pItem->_pSubFieldMap ;
            _it = _pMap->begin() ;
            pItem = _it->second ;
         }
         ++_it ;

         // if arrive the leaf, go back to the last forky node.
         while ( _it == _pMap->end() && !_mapTrace.empty() && !_itTrace.empty() )
         {
            _pMap = _mapTrace.back() ;
            _mapTrace.pop_back() ;
            _it = _itTrace.back() ;
            _itTrace.pop_back() ;

            ++_it ;
         }
      }

      return pItem ;
   }

   /*
      implement _clsAutoIncItem
   */
   _clsAutoIncItem::_clsAutoIncItem()
   {
      _fieldName     = NULL ;
      _fullName      = NULL ;
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
         AUTOINC_ITEM_MAP_IT it = _pSubFieldMap->begin() ;
         while( it != _pSubFieldMap->end() )
         {
            SDB_OSS_DEL it->second ;
            ++it ;
         }
         _pSubFieldMap->clear() ;
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
               rc = SDB_INVALIDARG ;
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
               rc = SDB_INVALIDARG ;
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
               rc = SDB_INVALIDARG ;
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
               rc = SDB_INVALIDARG ;
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
         rc = SDB_INVALIDARG ;
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
                                 const AUTOINC_GEN_TYPE generated,
                                 const CHAR* fullName )
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
      _clsAutoIncItem   *pSubItem = NULL ;

      _fullName = fullName ? fullName : fieldName ;
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
         UINT32 strLen = subField - fieldName ;
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

         rc = pSubItem->_init( subField, sequenceName, sequenceID,
                               generated, _fullName ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Init sub item failed, rc: %d", rc ) ;
            goto error ;
         }

         _pSubFieldMap = new(std::nothrow) AUTOINC_ITEM_MAP ;
         if ( !_pSubFieldMap )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Alloc sub field map failed" ) ;
            goto error ;
         }
         /// add to map
         _pSubFieldMap->insert(
               AUTOINC_ITEM_MAP_VAL( pSubItem->fieldName(), pSubItem ) ) ;
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
         AUTOINC_ITEM_MAP *pItemMap = pItem->_pSubFieldMap ;
         AUTOINC_ITEM_MAP_IT itemIt = pItemMap->begin() ;
         AUTOINC_ITEM_MAP_IT selfIt ;

         while( itemIt != pItemMap->end() )
         {
            pItemSub = itemIt->second ;
            selfIt = _pSubFieldMap->find( pItemSub->fieldName() ) ;

            if ( selfIt == _pSubFieldMap->end() )
            {
               /// Insert
               _pSubFieldMap->insert( AUTOINC_ITEM_MAP_VAL( pItemSub->fieldName(),
                                                            pItemSub ) ) ;
               /// remove from pItem's sub item
               itemIt = pItemMap->erase( itemIt ) ;
            }
            else
            {
               pSelfSub = selfIt->second ;
               rc = pSelfSub->merge( pItemSub ) ;
               if ( rc )
               {
                  PD_LOG( PDERROR, "Merge auto-increment item[%s] to [%s] "
                          "failed, rc: %d", pItemSub->fieldName(),
                          pSelfSub->fieldName(), rc ) ;
                  goto error ;
               }

               ++itemIt ;
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

   const _clsAutoIncItem* _clsAutoIncItem::findItem( const CHAR *pName ) const
   {
      if ( _pSubFieldMap )
      {
         AUTOINC_ITEM_MAP_CONST_IT it = _pSubFieldMap->find( pName ) ;
         if ( it == _pSubFieldMap->end() )
            return NULL ;
         else
            return it->second ;
      }
      return NULL ;
   }

   const clsAIID _clsAutoIncItem::AIID() const
   {
      clsAIID aiid ;
      aiid.seqID = _sequenceID ;
      aiid.genType = _generatedType ;
      return aiid ;
   }

   /*
      _clsAutoIncSet implement
   */
   _clsAutoIncSet::_clsAutoIncSet()
   {
      _fieldCount = 0 ;
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
      _fieldCount = 0 ;

      _vecFields.clear() ;
   }

   UINT32 _clsAutoIncSet::_calcEleSize( const AUTOINC_ITEM_MAP &map )
   {
      UINT32 eleSize= 0 ;
      INT32 fieldLen = 0 ;
      const clsAutoIncItem *pItem = NULL ;

      AUTOINC_ITEM_MAP_CONST_IT cit = map.begin() ;
      while ( cit != map.end() )
      {
         pItem = cit->second ;
         fieldLen = ossStrlen( pItem->fieldName() ) + 1 ;
         if ( !pItem->hasSubField() )
         {
            // |type(CHAR) |field(CHAR*)  |sequenceValue(INT64) |
            eleSize += ( 1 + fieldLen + 8 ) ;
         }
         else
         {
            AUTOINC_ITEM_MAP *pSubMap = pItem->_pSubFieldMap ;
            // |type(CHAR) |field(CHAR*)  |subObj(BSONObj) |
            // BSONObj: |length(UINT32)  |elements(...)  |EOO(CHAR) |
            eleSize += ( 1 + fieldLen + 4 +
                             _calcEleSize( *pSubMap ) + 1 ) ;
         }
         ++cit ;
      }
      return eleSize ;
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

      _eleSize = _calcEleSize( _mapItem ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _clsAutoIncSet::insert( const BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      rc = _initAItem( obj.getOwned() ) ;
      if ( SDB_OK == rc )
      {
         _eleSize = _calcEleSize( _mapItem );
      }
      return rc ;
   }

   INT32 _clsAutoIncSet::_initAItem( const BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      clsAutoIncItem *pItem = NULL ;
      clsAutoIncItem *pFirstItem = NULL ;
      AUTOINC_ITEM_MAP_IT it ;
      clsAIID aiid ;

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
         PD_LOG( PDERROR, "Failed to init item, rc: %d", rc ) ;
         goto error ;
      }

      // get AIID
      pFirstItem = pItem ;
      while ( pItem->hasSubField() )
      {
         pItem = pItem->_pSubFieldMap->begin()->second ;
      }
      aiid = pItem->AIID() ;
      pItem = pFirstItem ;

      /// Find first
      it = _mapItem.find( pItem->fieldName() ) ;
      if ( it != _mapItem.end() )
      {
         clsAutoIncItem *pTmpItem = it->second ;

         /// merge
         rc = pTmpItem->merge( pItem ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to merge item, rc: %d", rc ) ;
            goto error ;
         }
      }
      else
      {
         _mapItem.insert( AUTOINC_ITEM_MAP_VAL( pItem->fieldName(),
                                                pItem ) ) ;
         pItem = NULL ;
      }

      ++_fieldCount ;
      _aiidSet.insert( aiid ) ;
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

