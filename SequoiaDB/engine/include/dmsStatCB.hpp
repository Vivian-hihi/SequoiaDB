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

   Source File Name = dmsStatsCB.hpp

   Descriptive Name = Data Management Service Statistics Control Block Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS Statistics Table Control Block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =

*******************************************************************************/
#ifndef DMSSTATCB_HPP__
#define DMSSTATCB_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossLatch.hpp"
#include "dms.hpp"
#include "dmsSysCB.hpp"
#include "dmsStatObj.hpp"
#include "rtnStatMgr.hpp"

using namespace std ;

namespace engine
{

#define SYSSTAT_SPACE_NAME          "SYSSTAT"
#define SYSSTAT_COLLECTION_CL_NAME  SYSSTAT_SPACE_NAME".SYSCOLLECTIONSTAT"
#define SYSSTAT_INDEX_CL_NAME       SYSSTAT_SPACE_NAME".SYSINDEXSTAT"

#define STAT_CL_IDX_NAME "STATCLIDX"

#define STAT_CL_IDX_DEF \
   "{ "IXM_FIELD_NAME_NAME"      : \""STAT_CL_IDX_NAME"\", \
      "IXM_FIELD_NAME_KEY"       : { "STAT_COLLECTION_SPACE" : 1, \
                                     "STAT_COLLECTION" : 1 } }"

#define STAT_IDX_IDX_NAME "STATIDXIDX"

#define SYSSTAT_IDX_IDX_DEF \
   "{ "IXM_FIELD_NAME_NAME"      : \""STAT_IDX_IDX_NAME"\", \
      "IXM_FIELD_NAME_KEY"       : { "STAT_COLLECTION_SPACE" : 1, \
                                     "STAT_COLLECTION" : 1, \
                                     "STAT_IDX_INDEX" : 1 } }"

   class _pmdEDUCB ;

   /*
      _dmsStatCB define
   */
   class _dmsStatCB : public _dmsSysCB
   {
      public :
         _dmsStatCB ( _SDB_DMSCB *dmsCB ) ;

         INT32 init () ;

         INT32 reloadStats ( _pmdEDUCB *cb ) ;

         INT32 onDropCollectionSpace ( const CHAR *pCSName, _pmdEDUCB *cb,
                                       SDB_DPSCB *dpsCB ) ;

         INT32 onDropCollection ( const CHAR *pCSName, UINT16 mbID,
                                  _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         INT32 onDropIndex ( const CHAR *pCSName, UINT16 mbID,
                             const CHAR *pIndexName, _pmdEDUCB *cb,
                             SDB_DPSCB *dpsCB ) ;

         INT32 onRenameCollectionSpace ( const CHAR *pOldCSName,
                                         const CHAR *pNewCSName,
                                         _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

         INT32 onRenameCollection ( const CHAR *pCSName, UINT16 mbID,
                                    const CHAR *pOldCLName, const CHAR *pNewCLName,
                                    _pmdEDUCB *cb, SDB_DPSCB *dpsCB ) ;

      protected :

         typedef _utilMap< dmsStatObjName, rtnStatMap * > dmsCSStatMap ;

         INT32 _ensureStatMetadata ( _pmdEDUCB *cb ) ;

         INT32 _loadStats ( _pmdEDUCB *cb ) ;

         INT32 _loadCollectionStats ( dmsCSStatMap &csStatMap, _pmdEDUCB *cb ) ;

         INT32 _loadIndexStats ( dmsCSStatMap &csStatMap, _pmdEDUCB *cb ) ;

         INT32 _clearStats () ;

         INT32 _addCollectionStats ( const CHAR *pCSName, rtnStatMap &statMap ) ;

         INT32 _addCollectionStat ( dmsCSStatMap &csStatMap,
                                    dmsCollectionStat *pCollectionStat,
                                    BOOLEAN ignoreVersion ) ;

         INT32 _addIndexStat ( dmsCSStatMap &csStatMap,
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
         BSONObj _collectionHint ;
         BSONObj _indexHint ;
   } ;

   typedef class _dmsStatCB dmsStatCB ;

}

#endif //DMSSTATCB_HPP__

