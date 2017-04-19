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

   Source File Name = rtnStatMgr.hpp

   Descriptive Name = Runtime Statistics Manager Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Statistics
   Manager.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/
#ifndef RTNSTATMGR_HPP__
#define RTNSTATMGR_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "dms.hpp"
#include "dmsStatObj.hpp"
#include "dpsLogWrapper.hpp"

using namespace std ;
using namespace bson ;

namespace engine
{

   class _dmsStorageUnit ;
   class _pmdEDUCB ;

   /*
      _rtnStatMap define
    */
   class _rtnStatMap : public SDBObject
   {
      public :
         _rtnStatMap ( const CHAR *pCSName ) ;

         virtual ~_rtnStatMap () ;

         BOOLEAN addCollectionStat ( dmsCollectionStat *pCollectionStat,
                                     BOOLEAN ignoreVersion ) ;

         BOOLEAN addIndexStat ( dmsIndexStat *pIndexStat,
                                BOOLEAN ignoreVersion ) ;

         OSS_INLINE const dmsCollectionStat *getCollectionStat ( UINT16 mbID ) const
         {
            if ( _isValid && mbID < DMS_MME_SLOTS )
            {
               return _collectionStats[ mbID ] ;
            }
            return NULL ;
         }

         OSS_INLINE dmsCollectionStat *getCollectionStat ( UINT16 mbID )
         {
            if ( _isValid && mbID < DMS_MME_SLOTS )
            {
               return _collectionStats[ mbID ] ;
            }
            return NULL ;
         }

         BOOLEAN removeCollectionStat ( UINT16 mbID, BOOLEAN needDelete = TRUE ) ;

         BOOLEAN removeIndexStat ( UINT16 mbID, const CHAR *pIndexName ) ;

         BOOLEAN clearStats () ;

         OSS_INLINE BOOLEAN isValid () const { return _isValid ; }

      protected :
         BOOLEAN              _isValid ;
         dmsCollectionStat *  _collectionStats [ DMS_MME_SLOTS ] ;
   } ;

   typedef class _rtnStatMap rtnStatMap ;

   /*
      _rtnStatMgr define
    */
   class _rtnStatMgr : public _rtnStatMap
   {
      public :
         _rtnStatMgr ( _dmsStorageUnit* su, const CHAR *pSUName ) ;

         virtual ~_rtnStatMgr () {}

         void addCollectionStats ( rtnStatMap &statMap ) ;

         INT32 onDropCollectionSpace ( _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         INT32 onDropCollection ( UINT16 mbID, _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         INT32 onDropIndex ( UINT16 mbID, const CHAR *pIndexName,
                             _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         INT32 onRenameCollectionSpace ( const CHAR *pOldCSName,
                                         const CHAR *pNewCSName,
                                         _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         INT32 onRenameCollection ( UINT16 mbID, const CHAR *pOldCLName,
                                    const CHAR *pNewCLName, _pmdEDUCB *cb,
                                    SDB_DPSCB *dpsCB ) ;

      protected :
         _dmsStorageUnit *    _pSu ;
   } ;

   typedef class _rtnStatMgr rtnStatMgr ;
}

#endif //RTNSTATMGR_HPP__

