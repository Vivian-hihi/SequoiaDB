/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = dmsExtDataHandler.hpp

   Descriptive Name = External data process handler for dms.

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
#ifndef DMS_EXTDATAHANDLER_HPP__
#define DMS_EXTDATAHANDLER_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "dpsLogWrapper.hpp"
#include "monDMS.hpp"
#include "../bson/oid.h"

namespace engine
{
   enum _dmsExtOprType
   {
      DMS_EXT_INVALID = 0,
      DMS_EXT_INSERT = 1,
      DMS_EXT_DELETE,
      DMS_EXT_UPDATE,
      DMS_EXT_TRUNCATE
   } ;

   class _IDmsExtDataHandler
   {
      public:
         _IDmsExtDataHandler() {}
         virtual ~_IDmsExtDataHandler() {}

      public:
         virtual INT32 onDropCS( const monCSSimple &csInfo,
                                 _pmdEDUCB *cb ) = 0 ;
         virtual INT32 onCreateTextIdx( const CHAR *clFullName,
                                        const CHAR *idxName,
                                        INT64 bufferSize,
                                        _pmdEDUCB* cb,
                                        SDB_DPSCB *dpsCB = NULL ) = 0 ;
         virtual INT32 onDropTextIdx( const CHAR *clFullName,
                                      const CHAR *idxName,
                                      _pmdEDUCB *cb,
                                      SDB_DPSCB *dpscb = NULL ) = 0 ;
         virtual INT32 onInsert( const CHAR *clFullName, const CHAR *idxName,
                                 BSONObj &object, bson::OID &oid,
                                 INT32 flags, _pmdEDUCB* cb,
                                 SDB_DPSCB *dpscb = NULL ) = 0 ;
         virtual INT32 onDelete( const CHAR *clFullName, const CHAR *idxName,
                                 bson::OID &oid, _pmdEDUCB* cb,
                                 SDB_DPSCB *dpscb = NULL ) = 0 ;
         virtual INT32 onUpdate( const CHAR *clFullName, const CHAR *idxName,
                                 BSONObj &object, bson::OID &oid, INT32 flags,
                                 _pmdEDUCB* cb, SDB_DPSCB *dpscb = NULL ) = 0 ;
         virtual INT32 onTruncate( const CHAR *clFullName, const CHAR *idxName,
                                   _pmdEDUCB* cb,
                                   SDB_DPSCB *dpscb = NULL ) = 0 ;
   } ;
   typedef _IDmsExtDataHandler IDmsExtDataHandler ;
}

#endif /* DMS_EXTDATAHANDLER_HPP__ */

