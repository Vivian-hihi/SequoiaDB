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
      enableMask( UTIL_RESULT_MASK_DUP ) ;
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

      _insertedNum = 0 ;
      _ignoredNum = 0 ;
      _replacedNum = 0 ;
   }

   void utilInsertResult::toBSON( BSONObjBuilder &builder ) const
   {
      try
      {
         utilWriteResult::toBSON( builder ) ;

         /// stat info
         builder.append( FIELD_NAME_INSERT_NUM, (INT64)_insertedNum ) ;
         builder.append( FIELD_NAME_IGNORE_NUM, (INT64)_ignoredNum ) ;
         builder.append( FIELD_NAME_REPLACE_NUM, (INT64)_replacedNum ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Insert result to BSON occur exception: %s",
                 e.what() ) ;
      }
   }

   BOOLEAN utilInsertResult::_filterResultElement( const BSONElement &e ) const
   {
      if ( 0 == ossStrcmp( FIELD_NAME_INSERT_NUM, e.fieldName() ) ||
           0 == ossStrcmp( FIELD_NAME_IGNORE_NUM, e.fieldName() ) ||
           0 == ossStrcmp( FIELD_NAME_REPLACE_NUM, e.fieldName() ) )
      {
         return FALSE ;
      }
      return utilWriteResult::_filterResultElement( e ) ;
   }

   void utilInsertResult::enableDupErrInfo()
   {
      enableMask( UTIL_RESULT_MASK_DUP ) ;
   }

   void utilInsertResult::disableDupErrInfo()
   {
      disableMask( UTIL_RESULT_MASK_DUP ) ;
   }

   BOOLEAN utilInsertResult::isEnaleDupErrInfo() const
   {
      return isMaskEnabled( UTIL_RESULT_MASK_DUP ) ;
   }

   void utilInsertResult::incIngoreOrRepaceNum( BOOLEAN isReplace,
                                                UINT64 step )
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
         utilWriteResult::toBSON( builder ) ;

         builder.append( FIELD_NAME_UPDATE_NUM, (INT64)_updatedNum ) ;
         builder.append( FIELD_NAME_MODIFIED_NUM, (INT64)_modifiedNum ) ;
         builder.append( FIELD_NAME_INSERT_NUM, (INT64)insertedNum() ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Update result to BSON occur exception: %s",
                 e.what() ) ;
      }
   }

   BOOLEAN utilUpdateResult::_filterResultElement( const BSONElement &e ) const
   {
      if ( 0 == ossStrcmp( FIELD_NAME_UPDATE_NUM, e.fieldName() ) ||
           0 == ossStrcmp( FIELD_NAME_MODIFIED_NUM, e.fieldName() ) ||
           0 == ossStrcmp( FIELD_NAME_INSERT_NUM, e.fieldName() ) )
      {
         return FALSE ;
      }
      return utilWriteResult::_filterResultElement( e ) ;
   }

   /*
      utilDeleteResult implement
   */
   utilDeleteResult::utilDeleteResult()
   {
      disableMask( UTIL_RESULT_MASK_DUP ) ;
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
         utilWriteResult::toBSON( builder ) ;

         builder.append( FIELD_NAME_DELETE_NUM, (INT64)_deletedNum ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Delete result to BSON occur exception: %s",
                 e.what() ) ;
      }
   }

   BOOLEAN utilDeleteResult::_filterResultElement( const BSONElement &e ) const
   {
      if ( 0 == ossStrcmp( FIELD_NAME_DELETE_NUM, e.fieldName() ) )
      {
         return FALSE ;
      }
      return utilWriteResult::_filterResultElement( e ) ;
   }

}


