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

   Source File Name = utilInsertResult.cpp

   Descriptive Name = util insert error info

   When/how to use: N/A

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          02/13/2019  LYB Initial Draft

   Last Changed =

*******************************************************************************/

#include "utilInsertResult.hpp"
#include "msgDef.hpp"
#include "pd.hpp"

using namespace bson ;

namespace engine
{

   /*
      utilInsertResult implement
   */
   utilInsertResult::utilInsertResult()
   {
      _isEnableDupErrInfo = TRUE ;
      _insertedNum = 0 ;
      _ignoredNum = 0 ;
      _replacedNum = 0 ;
   }

   utilInsertResult::~utilInsertResult()
   {
   }

   void utilInsertResult::reset()
   {
      utilWriteResult::reset() ;
      resetDupInfo() ;

      _insertedNum = 0 ;
      _ignoredNum = 0 ;
      _replacedNum = 0 ;
   }

   void utilInsertResult::toBSON( BSONObjBuilder &builder ) const
   {
      try
      {
         /// stat info
         builder.append( FIELD_NAME_INSERT_NUM, (INT32)_insertedNum ) ;
         builder.append( FIELD_NAME_IGNORE_NUM, (INT32)_ignoredNum ) ;
         builder.append( FIELD_NAME_REPLACE_NUM, (INT32)_replacedNum ) ;

         /*
         /// dup error info
         if ( !_idxValue.isEmpty() && !_idxName.empty() &&
              !_idxKeyPattern.isEmpty() )
         {
            builder.append( FIELD_NAME_INDEXNAME, _idxName ) ;
            builder.append( FIELD_NAME_INDEX, _idxKeyPattern ) ;
         } */
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Insert result to BSON occur exception: %s",
                 e.what() ) ;
      }
   }

   void utilInsertResult::resetDupInfo()
   {
      _idxName.clear() ;
      _idxKeyPattern = BSONObj() ;
      _idxValue = BSONObj() ;
   }

   void utilInsertResult::enableDupErrInfo()
   {
      _isEnableDupErrInfo = TRUE ;
   }

   void utilInsertResult::disableDupErrInfo()
   {
      _isEnableDupErrInfo = FALSE ;
   }

   BOOLEAN utilInsertResult::isEnaleDupErrInfo() const
   {
      return _isEnableDupErrInfo ;
   }

   ossPoolString utilInsertResult::getIdxName() const
   {
      return _idxName ;
   }

   BSONObj utilInsertResult::getIdxKeyPattern() const
   {
      return _idxKeyPattern ;
   }

   BSONObj utilInsertResult::getIdxValue() const
   {
      return _idxValue ;
   }

   INT32 utilInsertResult::setDupErrInfo( const CHAR *idxName,
                                          const BSONObj& idxKeyPattern,
                                          const BSONObj& idxValueWithoutKey )
   {
      INT32 rc = SDB_OK ;

      if ( !isEnaleDupErrInfo() )
      {
         goto done ;
      }

      try
      {
         _idxName = idxName ;
         _idxKeyPattern = idxKeyPattern.getOwned() ;
         _idxValue = idxValueWithoutKey.getOwned() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Build insert error info occur exception: %s",
                 e.what() ) ;
         rc = SDB_OOM ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void utilInsertResult::setDupErrInfo( const utilInsertResult *pResult )
   {
      _idxName = pResult->getIdxName() ;
      _idxKeyPattern = pResult->getIdxKeyPattern() ;
      _idxValue = pResult->getIdxValue() ;
   }

   void utilInsertResult::incIngoreOrRepaceNum( BOOLEAN isReplace,
                                                UINT32 step )
   {
      if ( isReplace )
      {
         _replacedNum += step ;
      }
      else
      {
         _ignoredNum += step ;
      }
   }

   /*
      utilUpdateResult implement
   */
   utilUpdateResult::utilUpdateResult()
   {
      _updatedNum = 0 ;
      _modifiedNum = 0 ;
   }

   utilUpdateResult::~utilUpdateResult()
   {
   }

   void utilUpdateResult::reset()
   {
      utilInsertResult::reset() ;
      _updatedNum = 0 ;
      _modifiedNum = 0 ;
   }

   void utilUpdateResult::toBSON( BSONObjBuilder &builder ) const
   {
      try
      {
         builder.append( FIELD_NAME_UPDATE_NUM, (INT64)_updatedNum ) ;
         builder.append( FIELD_NAME_MODIFIED_NUM, (INT64)_modifiedNum ) ;
         builder.append( FIELD_NAME_INSERT_NUM, (INT32)insertedNum() ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Update result to BSON occur exception: %s",
                 e.what() ) ;
      }
   }

   /*
      utilDeleteResult implement
   */
   utilDeleteResult::utilDeleteResult()
   {
      _deletedNum = 0 ;
   }

   utilDeleteResult::~utilDeleteResult()
   {
   }

   void utilDeleteResult::reset()
   {
      utilWriteResult::reset() ;
      _deletedNum = 0 ;
   }

   void utilDeleteResult::toBSON( BSONObjBuilder &builder ) const
   {
      try
      {
         builder.append( FIELD_NAME_DELETE_NUM, (INT64)_deletedNum ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Delete result to BSON occur exception: %s",
                 e.what() ) ;
      }
   }

   /*
      utilIdxDupErrAssit implement
   */
   utilIdxDupErrAssit::utilIdxDupErrAssit( const BSONObj &idxKeyPattern,
                                           const BSONObj &idxValue )
   {
      _idxKeyPattern = idxKeyPattern ;
      _idxValue = idxValue ;
   }

   utilIdxDupErrAssit::~utilIdxDupErrAssit()
   {
   }

   INT32 utilIdxDupErrAssit::getIdxMatcher( BSONObj &idxMatcher )
   {
      INT32 rc = SDB_OK ;

      if ( _idxKeyPattern.isEmpty() || _idxValue.isEmpty() )
      {
         PD_LOG( PDERROR, "Key pattern or value is empty" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      try
      {
         BSONObjBuilder builder( _idxKeyPattern.objsize() +
                                 _idxValue.objsize() ) ;

         BSONObjIterator itrK( _idxKeyPattern ) ;
         BSONObjIterator itrV( _idxValue ) ;

         while( itrK.more() && itrV.more() )
         {
            BSONElement eK = itrK.next() ;
            BSONElement eV = itrV.next() ;

            if ( Undefined != eV.type() )
            {
               builder.appendAs( eV, eK.fieldName() ) ;
            }
            else
            {
               BSONObjBuilder sub( builder.subobjStart( eK.fieldName() ) ) ;
               sub.append( "$exists", 0 ) ;
               sub.done() ;
            }
         }

         if ( itrK.more() || itrV.more() )
         {
            PD_LOG( PDERROR, "Key value[%s] is not match the key pattern[%s]",
                    _idxValue.toString().c_str(),
                    _idxKeyPattern.toString().c_str() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         idxMatcher = builder.obj() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Builder index matcher occur exception: %s",
                 e.what() ) ;
         rc = SDB_OOM ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

}


