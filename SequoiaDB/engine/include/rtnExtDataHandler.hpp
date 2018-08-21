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

   Source File Name = rtnExtDataHandler.hpp

   Descriptive Name = External data process handler for rtn.

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          14/04/2017  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTN_EXTDATAHANDLER_HPP__
#define RTN_EXTDATAHANDLER_HPP__

#include "pmdEDU.hpp"
#include "dpsLogWrapper.hpp"
#include "dmsExtDataHandler.hpp"
#include "rtnExtDataProcessor.hpp"
#include "rtnExtContext.hpp"

namespace engine
{
   class _rtnExtDataHandler : public _IDmsExtDataHandler
   {
   public:
      _rtnExtDataHandler( rtnExtDataProcessorMgr *edpMgr ) ;
      virtual ~_rtnExtDataHandler() ;

   public:
      virtual INT32 getExtDataName( utilCLUniqueID clUniqID,
                                    const CHAR *idxName,
                                    CHAR *extName,
                                    UINT32 buffSize ) ;

      virtual INT32 check( DMS_EXTOPR_TYPE type, const CHAR *csName,
                           const CHAR *clName, const CHAR *idxName,
                           const BSONObj *object, const BSONObj *objNew,
                           pmdEDUCB *cb ) ;

      virtual INT32 onOpenTextIdx( const CHAR *csName, const CHAR *clName,
                                   ixmIndexCB &indexCB ) ;

      // Always called after the cs name have been removed from cscbVec.(In 2P
      // dropping, it may be in the delCscbVec. Any way, other sessions are not
      // able to see this cs now.
      virtual INT32 onDelCS( const CHAR *csName, pmdEDUCB *cb,
                             BOOLEAN removeFiles, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onDelCL( const CHAR *csName, const CHAR *clName,
                             pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onCrtTextIdx( utilCLUniqueID clUniqID,
                                  const BSONObj &index, BSONObj &newIndex,
                                  _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onDropTextIdx( const CHAR *extName, _pmdEDUCB *cb,
                                   SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onRebuildTextIdx( const CHAR *csName, const CHAR *clName,
                                      const CHAR *idxName, const CHAR *extName,
                                      const BSONObj &indexDef, _pmdEDUCB *cb,
                                      SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onInsert( const CHAR *extName, const BSONObj &object,
                              _pmdEDUCB* cb, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onDelete( const CHAR *extName, const BSONObj &object,
                              _pmdEDUCB* cb, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onUpdate( const CHAR *extName, const BSONObj &orignalObj,
                              const BSONObj &newObj, _pmdEDUCB* cb,
                              SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onTruncateCL( const CHAR *csName, const CHAR *clName,
                                  _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onRenameCS( const CHAR *oldCSName, const CHAR *newCSName,
                                _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 onRenameCL( const CHAR *csName, const CHAR *oldCLName,
                                const CHAR *newCLName, _pmdEDUCB *cb,
                                SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 done( DMS_EXTOPR_TYPE type, _pmdEDUCB *cb,
                          SDB_DPSCB *dpscb = NULL ) ;

      virtual INT32 abortOperation( DMS_EXTOPR_TYPE type, _pmdEDUCB *cb ) ;

   private:
      BOOLEAN _hasExtName( const ixmIndexCB &indexCB ) ;
      INT32 _extendIndexDef( const CHAR *csName, const CHAR *clName,
                             ixmIndexCB &indexCB ) ;

      // For compatibility with version 3.0. The external name rule is
      // different. This is used during upgrading from version 3.0, to append
      // 'ExtDataName' to indexCB.
      void _getExtDataNameV1( const CHAR *csName, const CHAR *clName,
                              const CHAR *idxName, string &extName ) ;
   private:
      rtnExtDataProcessorMgr  *_edpMgr ;
      rtnExtContextMgr        _contextMgr ;
   } ;
   typedef _rtnExtDataHandler rtnExtDataHandler ;

   rtnExtDataHandler* rtnGetExtDataHandler() ;
}

#endif /* RTN_EXTDATAHANDLER_HPP__ */

