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

   Source File Name = catDCLogMgr.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of runtime component. This file contains code logic for
   common functions for coordinator node.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================

   Last Changed =     XJH Opt

*******************************************************************************/


#include "catCommon.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
#include "catDCLogMgr.hpp"
#include "dmsCB.hpp"
#include "rtnCB.hpp"
#include "dpsLogWrapper.hpp"
#include "catalogueCB.hpp"
#include "pdTrace.hpp"
#include "catTrace.hpp"

using namespace bson ;

namespace engine
{

   /*
      _catDCLogItem implement
   */
   _catDCLogItem::_catDCLogItem( UINT32 pos, const string &clname )
   {
      pmdKRCB *krcb     = pmdGetKRCB() ;
      _pDmsCB           = krcb->getDMSCB() ;
      _pDpsCB           = krcb->getDPSCB() ;
      _pRtnCB           = krcb->getRTNCB() ;
      _pCatCB           = krcb->getCATLOGUECB() ;

      _pos              = pos ;
      _clName           = clname ;

      _reset() ;
   }

   _catDCLogItem::~_catDCLogItem()
   {
   }

   string _catDCLogItem::toString() const
   {
      // name + count + first lsn + last lsn
      return "" ;
   }

   void _catDCLogItem::_reset()
   {
   }

   INT32 _catDCLogItem::restore( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      // first reset
      _reset() ;

      // if the lsn is not continuous, need to drop the collection

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _catDCLogItem::truncate( _pmdEDUCB *cb )
   {
      return SDB_OK ;
   }

   UINT64 _catDCLogItem::getCount() const
   {
      return 0 ;
   }

   UINT32 _catDCLogItem::getLID() const
   {
      return 0 ;
   }

   DPS_LSN _catDCLogItem::getFirstLSN() const
   {
      return DPS_LSN() ;
   }

   DPS_LSN _catDCLogItem::getLastLSN() const
   {
      return DPS_LSN() ;
   }

   DPS_LSN _catDCLogItem::getComingLSN() const
   {
      return DPS_LSN() ;
   }

   /*
      _catDCLogMgr implement
   */
   _catDCLogMgr::_catDCLogMgr()
   {
      _pEduCB     = NULL ;
      _begin      = 0 ;
      _work       = 0 ;
   }

   _catDCLogMgr::~_catDCLogMgr()
   {
      for ( UINT32 i = 0 ; i < _vecLogCL.size() ; ++i )
      {
         SDB_OSS_DEL _vecLogCL[ i ] ;
      }
      _vecLogCL.clear() ;
   }

   void _catDCLogMgr::attachCB( pmdEDUCB * cb )
   {
      _pEduCB = cb ;
   }

   void _catDCLogMgr::detachCB( pmdEDUCB * cb )
   {
      _pEduCB = NULL ;
   }

   INT32 _catDCLogMgr::init()
   {
      INT32 rc          = SDB_OK ;
      catDCLogItem *pLog= NULL ;
      CHAR clName[ DMS_COLLECTION_FULL_NAME_SZ + 1 ] = { 0 } ;

      for ( UINT32 i = 0 ; i < CAT_SYSLOG_CL_NUM ; ++i )
      {
         ossSnprintf( clName, DMS_COLLECTION_FULL_NAME_SZ, "%s%d",
                      CAT_SYSLOG_COLLECTION_NAME, i ) ;
         pLog = SDB_OSS_NEW catDCLogItem( i, clName ) ;
         if ( !pLog )
         {
            PD_LOG( PDERROR, "Alloc dc log item[%s] failed", clName ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         _vecLogCL.push_back( pLog ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _catDCLogMgr::restore()
   {
      INT32 rc = SDB_OK ;
      catDCLogItem *pLog = NULL ;
      UINT32 i = 0 ;
      UINT32 tmpWork = 0 ;
      DPS_LSN minLsn ;
      DPS_LSN firstLsn ;
      DPS_LSN comingLsn ;

      for ( i = 0 ; i < _vecLogCL.size() ; ++i )
      {
         pLog = _vecLogCL[ i ] ;
         rc = pLog->restore( _pEduCB ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Analysis collection[%s] failed, rc: %d",
                    pLog->getCLName(), rc ) ;
            goto error ;
         }
      }

      // analyse the current work position
      // 1. find the begin
      for ( i = 0 ; i < _vecLogCL.size() ; ++i )
      {
         pLog = _vecLogCL[ i ] ;
         firstLsn = pLog->getFirstLSN() ;
         if ( !firstLsn.invalid() && ( minLsn.invalid() ||
              firstLsn.compareOffset( minLsn.offset ) < 0 ) )
         {
            _begin = i ;
            minLsn = firstLsn ;
         }
      }
      // 2. find the work
      tmpWork = _begin ;
      comingLsn = minLsn ;
      for ( i = 0 ; i < _vecLogCL.size() ; ++i )
      {
         pLog = _vecLogCL[ tmpWork ] ;
         firstLsn = pLog->getFirstLSN() ;
         if ( 0 != firstLsn.compareOffset( comingLsn.offset ) )
         {
            break ;
         }
         comingLsn = pLog->getComingLSN() ;
         tmpWork = _incFileID( tmpWork ) ;
      }
      _work = tmpWork ;
      // 3. reset others
      for ( ; i < _vecLogCL.size() ; ++i )
      {
         tmpWork = _incFileID( _work ) ;
         pLog = _vecLogCL[ tmpWork ] ;
         firstLsn = pLog->getFirstLSN() ;
         if ( !firstLsn.invalid() )
         {
            PD_LOG( PDWARNING, "Truncate system log[%s]",
                    pLog->toString().c_str() ) ;
            rc = pLog->truncate( _pEduCB ) ;
            PD_LOG( PDERROR, "Truncate system log[%s] failed, rc: %d",
                    pLog->toString().c_str(), rc ) ;
         }
      }

      PD_LOG( PDEVENT, "Analysis system log[ begin: %s, work: %s ]",
              _vecLogCL[ _begin ]->toString().c_str(),
              _vecLogCL[ _work ]->toString().c_str() ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _catDCLogMgr::saveSysLog( dpsLogRecordHeader *pHeader,
                                   const CHAR *pData,
                                   UINT32 length,
                                   DPS_LSN *pRetLSN )
   {
      INT32 rc = SDB_OK ;

   done:
      return rc ;
   error:
      goto done ;
   }

   UINT32 _catDCLogMgr::_incFileID ( UINT32 fileID )
   {
      ++fileID ;
      if ( fileID >= CAT_SYSLOG_CL_NUM )
      {
         fileID = 0 ;
      }

      return fileID ;
   }

   UINT32 _decFileID ( UINT32 fileID )
   {
      if ( 0 == fileID )
      {
         fileID = CAT_SYSLOG_CL_NUM - 1 ;
      }
      else
      {
         --fileID ;
      }

      return fileID ;
   }

}


