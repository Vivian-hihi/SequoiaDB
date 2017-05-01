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

   Source File Name = dmsStatSUMgr.hpp

   Descriptive Name = DMS Statistics Storage Unit Management Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS Statistics Storage Unit Management.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/
#ifndef DMSSTATSUMGR_HPP__
#define DMSSTATSUMGR_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossLatch.hpp"
#include "dms.hpp"
#include "dmsSysSUMgr.hpp"
#include "dmsEventHandler.hpp"
#include "dmsStatUnit.hpp"

using namespace std ;

namespace engine
{

   /*
      _dmsStatSUMgr define
   */
   class _dmsStatSUMgr : public _dmsSysSUMgr,
                         public _IDmsEventHandler
   {
      public :
         _dmsStatSUMgr ( _SDB_DMSCB *dmsCB ) ;

         INT32 init () ;

         INT32 reloadStats ( pmdEDUCB *cb ) ;

      public :
         // dmsEventHandler
         virtual INT32 onLoadCS ( _IDmsEventHolder *pEventHolder,
                                  _IUtilSUCacheHolder *pCacheHolder,
                                  pmdEDUCB *cb,
                                  SDB_DPSCB *dpsCB ) ;

         virtual INT32 onUnloadCS ( _IDmsEventHolder *pEventHolder,
                                    _IUtilSUCacheHolder *pCacheHolder,
                                    pmdEDUCB *cb,
                                    SDB_DPSCB *dpsCB ) ;

         virtual INT32 onRenameCS ( _IDmsEventHolder *pEventHolder,
                                    _IUtilSUCacheHolder *pCacheHolder,
                                    const CHAR *pOldCSName,
                                    const CHAR *pNewCSName,
                                    pmdEDUCB *cb,
                                    SDB_DPSCB *dpsCB ) ;

         virtual INT32 onDropCS ( _IDmsEventHolder *pEventHolder,
                                  _IUtilSUCacheHolder *pCacheHolder,
                                  pmdEDUCB *cb,
                                  SDB_DPSCB *dpsCB ) ;

         virtual INT32 onRenameCL ( _IDmsEventHolder *pEventHolder,
                                    _IUtilSUCacheHolder *pCacheHolder,
                                    const dmsCLItem &clItem,
                                    const CHAR *pNewCLName,
                                    pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         virtual INT32 onTruncateCL ( _IDmsEventHolder *pEventHolder,
                                      _IUtilSUCacheHolder *pCacheHolder,
                                      const dmsCLItem &clItem,
                                      pmdEDUCB *cb,
                                      SDB_DPSCB *dpsCB ) ;

         virtual INT32 onDropCL ( _IDmsEventHolder *pEventHolder,
                                  _IUtilSUCacheHolder *pCacheHolder,
                                  const dmsCLItem &clItem,
                                  pmdEDUCB *cb,
                                  SDB_DPSCB *dpsCB ) ;

         virtual INT32 onDropIndex ( _IDmsEventHolder *pEventHolder,
                                     _IUtilSUCacheHolder *pCacheHolder,
                                     const dmsCLItem &clItem,
                                     const dmsIdxItem &idxItem,
                                     pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         OSS_INLINE virtual UINT32 getMask ()
         {
            return DMS_EVENT_MASK_STAT ;
         }

      protected :

         typedef _utilMap< dmsStatSubUnitKey, utilSUCache * > CS_STAT_MAP ;

         INT32 _ensureStatMetadata ( pmdEDUCB *cb ) ;

         INT32 _loadStats ( pmdEDUCB *cb ) ;

         INT32 _loadCollectionStats ( CS_STAT_MAP &csStatMap, pmdEDUCB *cb ) ;

         INT32 _loadIndexStats ( CS_STAT_MAP &csStatMap, pmdEDUCB *cb ) ;

         INT32 _loadCollectionStatsByCS ( const CHAR *pCSName,
                                          utilSUCache &statMap,
                                          pmdEDUCB *cb ) ;

         INT32 _loadIndexStatsByCS ( const CHAR *pCSName, utilSUCache &statMap,
                                     pmdEDUCB *cb ) ;

         INT32 _clearStats () ;

         INT32 _replaceCollectionStats ( const CHAR *pCSName,
                                           utilSUCache *pStatMap ) ;

         INT32 _removeCollectionStats ( const CHAR *pCSName ) ;

         INT32 _addCollectionStat ( CS_STAT_MAP &csStatMap,
                                    dmsCollectionStat *pCollectionStat,
                                    BOOLEAN ignoreVersion ) ;

         INT32 _addIndexStat ( CS_STAT_MAP &csStatMap,
                               dmsIndexStat *pIndexStat,
                               BOOLEAN ignoreVersion ) ;

         INT32 _deleteCollectionStat ( const BSONObj &boMatcher, _pmdEDUCB *cb,
                                       SDB_DPSCB *dpsCB ) ;

         INT32 _deleteIndexStat ( const BSONObj &boMatcher, _pmdEDUCB *cb,
                                  SDB_DPSCB *dpsCB ) ;

         INT32 _updateCollectionStat ( const BSONObj &boMatcher,
                                       const BSONObj &boUpdator,
                                       _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         INT32 _updateIndexStat ( const BSONObj &boMatcher,
                                  const BSONObj &boUpdator,
                                  _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

      protected :
         BOOLEAN _initialized ;
         BSONObj _collectionHint ;
         BSONObj _indexHint ;
   } ;

   typedef class _dmsStatSUMgr dmsStatSUMgr ;

}

#endif //DMSSTATSUMGR_HPP__

