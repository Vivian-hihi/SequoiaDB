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

   Source File Name = utilResult.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          02/18/2019   LYB Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTIL_RESULT_HPP_
#define UTIL_RESULT_HPP_

#include "oss.hpp"
#include "../bson/bson.hpp"
#include "ossMemPool.hpp"
#include "dms.hpp"

using namespace bson ;

namespace engine
{
   class utilResult : public SDBObject
   {
   public:
      utilResult() ;
      virtual ~utilResult() ;

      virtual void reset() {}
      virtual void toBSON( BSONObjBuilder &builder ) const = 0 ;
   } ;

   /*
      utilIdxDupErrAssit define
   */
   class utilIdxDupErrAssit : public SDBObject
   {
   public:
      utilIdxDupErrAssit( const BSONObj &idxKeyPattern,
                          const BSONObj &idxValue ) ;
      ~utilIdxDupErrAssit() ;

      INT32    getIdxMatcher( BSONObj &idxMatcher,
                              BOOLEAN cvtUndefined = TRUE ) ;

   private:
      BSONObj        _idxKeyPattern ;
      BSONObj        _idxValue ;

   } ;

   #define UTIL_RESULT_MASK_DUP              ( 0x00000001 )
   #define UTIL_RESULT_MASK_ID               ( 0x00000002 )
   #define UTIL_RESULT_MASK_ALL              ( 0xFFFFFFFF )
   #define UTIL_RESULT_MASK_DFT              ( UTIL_RESULT_MASK_DUP | \
                                               UTIL_RESULT_MASK_ID )

   /*
      utilWriteResult define
   */
   class utilWriteResult : public utilResult
   {
   public:
      utilWriteResult( UINT32 mask = UTIL_RESULT_MASK_DFT ) ;
      virtual ~utilWriteResult() ;

      BSONObj           toBSON() const ;
      void              setResultObj( const BSONObj &obj ) ;
      void              resultResultInfo() ;
      BSONObj           getResultInfo() const { return _resultObj ; }

      void              enableMask( UINT32 mask ) ;
      void              disableMask( UINT32 mask ) ;
      BOOLEAN           isMaskEnabled( UINT32 mask ) const ;

   public:
      virtual void      reset() ;
      virtual void      toBSON( BSONObjBuilder &builder ) const ;

   protected:
      virtual BOOLEAN   _filterResultElement( const BSONElement &e ) const ;

   public:
      void     resetDupInfo() ;
      BOOLEAN  isDupInfoEmpty() const ;

      INT32    setDupErrInfo( const CHAR *idxName,
                              const BSONObj& idxKeyPattern,
                              const BSONObj& idxValue,
                              const BSONObj& curObj= BSONObj() ) ;

      INT32    setCurrentID( const BSONObj &obj ) ;
      INT32    setPeerID( const BSONObj &obj ) ;
      void     setCurRID( const dmsRecordID &rid ) ;
      void     setPeerRID( const dmsRecordID &rid ) ;

      void     setDupErrInfo( const utilWriteResult *pResult ) ;

      ossPoolString        getIdxName() const ;
      BSONObj              getIdxKeyPattern() const ;
      BSONObj              getIdxValue() const ;
      BSONObj              getCurID() const ;
      BSONObj              getPeerID() const ;
      const dmsRecordID&   getCurRID() const ;
      const dmsRecordID&   getPeerRID() const ;

   protected:
      ossPoolString        _idxName ;
      BSONObj              _idxKeyPattern ;
      BSONObj              _idxValue ;
      BSONObj              _curID ;
      BSONObj              _peerID ;
      dmsRecordID          _curRID ;
      dmsRecordID          _peerRID ;

      BSONObj              _resultObj ;

      UINT32               _resultMask ;

   } ;
}

#endif /* UTIL_RESULT_HPP_ */



