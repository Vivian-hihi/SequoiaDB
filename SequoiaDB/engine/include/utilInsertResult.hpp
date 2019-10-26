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

   Source File Name = utilInsertResult.hpp

   Dependencies: N/A

   Restrictions: N/AdmsStorageDataCommon

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          02/13/2019   LYB Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTIL_INSERT_RESULT_HPP_
#define UTIL_INSERT_RESULT_HPP_

#include "oss.hpp"
#include "utilResult.hpp"
#include "ossMemPool.hpp"

using namespace bson ;

namespace engine
{
   /*
      utilIdxDupErrAssit define
   */
   class utilIdxDupErrAssit : public SDBObject
   {
   public:
      utilIdxDupErrAssit( const BSONObj &idxKeyPattern,
                          const BSONObj &idxValue ) ;
      ~utilIdxDupErrAssit() ;

      INT32    getIdxMatcher( BSONObj &idxMatcher ) ;

   private:
      BSONObj        _idxKeyPattern ;
      BSONObj        _idxValue ;

   } ;

   /*
      utilInsertResult define
   */
   class utilInsertResult : public utilWriteResult
   {
   public:
      utilInsertResult() ;
      virtual ~utilInsertResult() ;

      virtual void      reset() ;
      virtual void      toBSON( BSONObjBuilder &builder ) const ;

      void              resetDupInfo() ;

   public:
      void     enableDupErrInfo() ;
      void     disableDupErrInfo() ;
      BOOLEAN  isEnaleDupErrInfo() const ;

      INT32    setDupErrInfo( const CHAR *idxName,
                              const BSONObj& idxKeyPattern,
                              const BSONObj& idxValueWithoutKey ) ;

      void     setDupErrInfo( const utilInsertResult *pResult ) ;

      ossPoolString        getIdxName() const ;
      BSONObj              getIdxKeyPattern() const ;
      BSONObj              getIdxValue() const ;

      UINT32               insertedNum() const { return _insertedNum ; }
      UINT32               ignoredNum() const { return _ignoredNum ; }
      UINT32               replacedNum() const { return _replacedNum ; }

      void                 incInsertedNum() { ++_insertedNum ; }
      void                 incIngoreOrRepaceNum( BOOLEAN isReplace = FALSE,
                                                 UINT32 step = 1 ) ;

   private:
      BOOLEAN              _isEnableDupErrInfo ;

      ossPoolString        _idxName ;
      BSONObj              _idxKeyPattern ;
      BSONObj              _idxValue ;

      UINT32               _insertedNum ;
      UINT32               _ignoredNum ;
      UINT32               _replacedNum ;
   } ;

   /*
      utilUpdateResult define
   */
   class utilUpdateResult : public utilInsertResult
   {
   public:
      utilUpdateResult() ;
      virtual ~utilUpdateResult() ;

      virtual void      reset() ;
      virtual void      toBSON( BSONObjBuilder &builder ) const ;

   public:
      UINT64               updateNum() const { return _updatedNum ; }
      UINT64               modifiedNum() const { return _modifiedNum ; }

      void                 incUpdatedNum() { ++_updatedNum ; }
      void                 incModifiedNum() { ++_modifiedNum ; }

   private:
      UINT64               _updatedNum ;
      UINT64               _modifiedNum ;

   } ;

   /*
      utilDeleteResult define
   */
   class utilDeleteResult : public utilWriteResult
   {
   public:
      utilDeleteResult() ;
      virtual ~utilDeleteResult() ;

      virtual void      reset() ;
      virtual void      toBSON( BSONObjBuilder &builder ) const ;

      UINT64            deletedNum() const { return _deletedNum ; }

      void              incDeletedNum() { ++_deletedNum ; }

   private:
      UINT64               _deletedNum ;

   } ;

}

#endif /* UTIL_INSERT_RESULT_HPP_ */

