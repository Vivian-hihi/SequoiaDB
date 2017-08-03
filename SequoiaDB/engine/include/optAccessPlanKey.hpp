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

   Source File Name = optAccessPlanKey.hpp

   Descriptive Name = Optimizer Access Plan Key Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Optimizer component. This file contains structure for key of
   access plan.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/17/2017  HGM  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OPTACCESSPLANKEY_HPP__
#define OPTACCESSPLANKEY_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "../bson/oid.h"
#include "../bson/bson.h"
#include "ossUtil.hpp"
#include "utilHashTable.hpp"
#include "rtnQueryOptions.hpp"
#include "dms.hpp"
#include "dmsStorageUnit.hpp"

using namespace bson ;

namespace engine
{

   /*
      _optAccessPlanKey define
    */
   class _optAccessPlanKey : public _utilHashTableKey,
                             public _rtnQueryOptions
   {
      friend class _optAccessPlan ;

      public :
         _optAccessPlanKey ( const CHAR *pCLFullName,
                             const BSONObj &selector,
                             const BSONObj &matcher,
                             const BSONObj &orderBy,
                             const BSONObj &hint,
                             SINT32 flags,
                             SINT64 numToSkip,
                             SINT64 numToReturn,
                             BOOLEAN needGetOwned ) ;

         _optAccessPlanKey ( const _optAccessPlanKey &planKey,
                             BOOLEAN needGetOwned ) ;

         virtual ~_optAccessPlanKey () {}

         OSS_INLINE virtual UINT32 getKeyCode () const
         {
            return _keyCode ;
         }

         OSS_INLINE void setValid ( BOOLEAN valid )
         {
            _isValid = valid ;
         }

         OSS_INLINE BOOLEAN getValid () const
         {
            return _isValid ;
         }

         virtual BOOLEAN isEqual ( const _optAccessPlanKey &key ) const ;

         OSS_INLINE dmsStorageUnitID getSUID () const
         {
            return _suID ;
         }

         OSS_INLINE UINT32 getSULID () const
         {
            return _suLID ;
         }

         OSS_INLINE UINT16 getCLMBID () const
         {
            return _mbID ;
         }

         OSS_INLINE UINT32 getCLLID () const
         {
            return _clLID ;
         }

         void generateKeyCode ( dmsStorageUnit *su, dmsMBContext *mbContext )
         {
            _setCSInfo( su ) ;
            _setCLInfo( mbContext ) ;

            _generateKeyCodeInternal() ;

            _isValid = TRUE ;
         }

      protected :
         OSS_INLINE void _setCSInfo ( dmsStorageUnit *su )
         {
            SDB_ASSERT( su, "su is invalid" ) ;
            _suID = su->CSID() ;
            _suLID = su->LogicalCSID() ;
         }

         OSS_INLINE void _setCLInfo ( dmsMBContext *mbContext )
         {
            SDB_ASSERT( mbContext, "su is invalid" ) ;
            _mbID = mbContext->mbID() ;
            _clLID = mbContext->clLID() ;
         }

         void _generateKeyCodeInternal () ;

         UINT32 _generateKeyCodeHash () ;

         UINT32 _generateKeyCodeMD5 () ;

      protected :
         BOOLEAN           _isValid ;
         dmsStorageUnitID  _suID ;
         UINT32            _suLID ;
         UINT32            _clLID ;
         UINT16            _mbID ;
   } ;

   typedef class _optAccessPlanKey optAccessPlanKey ;

}

#endif //OPTACCESSPLANKEY_HPP__

