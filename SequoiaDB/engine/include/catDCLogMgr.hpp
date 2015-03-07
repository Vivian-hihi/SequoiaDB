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

   Source File Name = catDCLogMgr.hpp

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
#ifndef CAT_DCLOGMGR_HPP__
#define CAT_DCLOGMGR_HPP__

#include "pmd.hpp"
#include "netDef.hpp"
#include "dpsLogRecord.hpp"
#include "dpsLogDef.hpp"
#include "pmdEDU.hpp"
#include <vector>
#include <string>

using namespace bson ;
using namespace std ;

namespace engine
{
   class sdbCatalogueCB ;
   class _SDB_RTNCB ;
   class _dpsLogWrapper ;
   class _SDB_DMSCB ;

   /*
      _catDCLogItem define
   */
   class _catDCLogItem : public SDBObject
   {
      public:
         _catDCLogItem( UINT32 pos, const string &clname ) ;
         ~_catDCLogItem() ;

         string         toString() const ;

         const CHAR*    getCLName() const { return _clName.c_str() ; }
         UINT32         getPos() const { return _pos ; }
         UINT64         getCount() const ;
         DPS_LSN        getFirstLSN() const ;
         DPS_LSN        getLastLSN() const ;
         DPS_LSN        getComingLSN() const ;
         UINT32         getLID() const ;

         INT32          truncate( _pmdEDUCB *cb ) ;
         INT32          restore( _pmdEDUCB *cb ) ;

      protected:
         void           _reset() ;

      private:
         _SDB_DMSCB                 *_pDmsCB;
         _dpsLogWrapper             *_pDpsCB;
         _SDB_RTNCB                 *_pRtnCB;
         sdbCatalogueCB             *_pCatCB;

         string                     _clName ;
         UINT32                     _pos ;

   } ;
   typedef _catDCLogItem catDCLogItem ;

   /*
      _catDCLogMgr define
   */
   class _catDCLogMgr : public SDBObject
   {
      public:
         _catDCLogMgr() ;
         ~_catDCLogMgr() ;

         void  attachCB( _pmdEDUCB *cb ) ;
         void  detachCB( _pmdEDUCB *cb ) ;

         INT32 init() ;
         INT32 restore() ;

         INT32 saveSysLog( dpsLogRecordHeader *pHeader,
                           const CHAR *pData,
                           UINT32 length,
                           DPS_LSN *pRetLSN = NULL ) ;

      protected:
         UINT32 _incFileID ( UINT32 fileID ) ;
         UINT32 _decFileID ( UINT32 fileID ) ;

      private:
         _pmdEDUCB                  *_pEduCB;
         vector< catDCLogItem* >    _vecLogCL ;

         UINT32                     _begin ;
         UINT32                     _work ;

   } ;
   typedef _catDCLogMgr catDCLogMgr ;


}

#endif // CAT_DCLOGMGR_HPP__

