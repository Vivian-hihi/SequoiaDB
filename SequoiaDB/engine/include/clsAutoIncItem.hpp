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

   Source File Name = clsAutoIncItem.hpp

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

#ifndef CLS_AUTOINC_ITEM_HPP__
#define CLS_AUTOINC_ITEM_HPP__

#include "oss.hpp"
#include "ossUtil.hpp"
#include "../bson/bson.h"
#include "utilMap.hpp"
#include "utilList.hpp"
#include "utilUniqueID.hpp"

using namespace bson ;

namespace engine
{
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
      define _clsAutoIncItem
   */
   class _clsAutoIncItem : public SDBObject
   {
   friend class _clsAutoIncSet ;

   public:
      typedef _utilStringMap<_clsAutoIncItem*, 1>        AUTOINC_ITEM_MAP ;

   public:
      _clsAutoIncItem() ;
      ~_clsAutoIncItem() ;

      const CHAR*          fieldName() const { return _fieldName ; }
      const CHAR*          sequenceName() const { return _sequenceName ; }
      const utilSequenceID sequenceID() const { return _sequenceID ; }
      AUTOINC_GEN_TYPE     generatedType() const { return _generatedType ; }
      BOOLEAN              hasSubField() const { return _pSubFieldMap ? TRUE :FALSE ; }

      const _clsAutoIncItem*  findSubItem( const CHAR *pName ) const ;

      const _clsAutoIncSet* subFieldSet() const { return _pSubFieldSet ; }

   protected:

      INT32             init( const BSONObj &obj ) ;

      INT32             merge( _clsAutoIncItem *pItem ) ;

   protected:
      void              _clear() ;
      INT32             _init( const CHAR* fieldName,
                               const CHAR* sequenceName,
                               const utilSequenceID sequenceID,
                               const AUTOINC_GEN_TYPE generated ) ;

   private:
      const CHAR*       _fieldName ;
      const CHAR*       _sequenceName ;
      utilSequenceID    _sequenceID ;
      AUTOINC_GEN_TYPE  _generatedType ;

      _clsAutoIncSet*   _pSubFieldSet ;
      string            _fieldStr ;

   } ;
   typedef _clsAutoIncItem clsAutoIncItem ;

   /*
      define container of clsAutoIncItem
   */
   typedef clsAutoIncItem::AUTOINC_ITEM_MAP     AUTOINC_ITEM_MAP ;
   typedef AUTOINC_ITEM_MAP::iterator           AUTOINC_ITEM_MAP_IT ;
   typedef AUTOINC_ITEM_MAP::const_iterator     AUTOINC_ITEM_MAP_CONST_IT ;
   typedef AUTOINC_ITEM_MAP::value_type         AUTOINC_ITEM_MAP_VAL ;

   /*
      _clsAutoIncSet define
   */
   class _clsAutoIncSet : public SDBObject
   {
      public:
         _clsAutoIncSet() ;
         ~_clsAutoIncSet() ;

      public:
         INT32    init( const BSONElement &ele ) ;
         void     clear() ;

         UINT32   totalCount() const { return _totalCount ; }
         UINT32   size() const { return _mapItem.size() ; }

         const clsAutoIncItem*      findItem( const CHAR *pName ) const ;

         const vector<BSONObj>&     getFields() const { return _vecFields ; }

      protected:

         const AUTOINC_ITEM_MAP&    getMap() const { return _mapItem ; }
         INT32    _initAItem( const BSONObj &obj ) ;
         void     _clear() ;

      private:
         BSONObj              _objInfo ;
         AUTOINC_ITEM_MAP     _mapItem ;
         UINT32               _totalCount ;

         vector<BSONObj>      _vecFields ;

   } ;
   typedef _clsAutoIncSet clsAutoIncSet ;

   /*
      define iterator of clsAutoIncItem
   */
   class _clsAutoIncIterator : public SDBObject
   {
      friend class _clsAutoIncSet ;
      friend class _clsAutoIncItem ;
   public:
      enum _MODE
      {
         nonRecur = 0,
         recursive
      };
      typedef enum _MODE MODE ;
      _clsAutoIncIterator( const _clsAutoIncSet &set, MODE mode = nonRecur ) ;
      BOOLEAN more() ;
      const _clsAutoIncItem* next() ;

   private:
      MODE _mode ;
      const AUTOINC_ITEM_MAP*    _pMap ;
      AUTOINC_ITEM_MAP_CONST_IT  _it ;
      _utilList< const AUTOINC_ITEM_MAP*, 1 >   _mapTrace ;
      _utilList< AUTOINC_ITEM_MAP_CONST_IT, 1 > _itTrace ;
   } ;
   typedef _clsAutoIncIterator clsAutoIncIterator ;

}

#endif //CLS_AUTOINC_ITEM_HPP__
