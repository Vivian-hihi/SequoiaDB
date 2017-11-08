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

namespace engine
{
   class _rtnExtDataHandler : public _IDmsExtDataHandler
   {
      public:
         _rtnExtDataHandler() ;
         virtual ~_rtnExtDataHandler() ;

      public:
         virtual INT32 onDropCS( const monCSSimple &csInfo,
                                 _pmdEDUCB *cb ) ;
         virtual INT32 onUnloadCS( const monCSSimple &csInfo,
                                   _pmdEDUCB *cb ) ;
         virtual INT32 onCreateTextIdx( const CHAR *clFullName,
                                        const CHAR *idxName,
                                        INT64 bufferSize,
                                        pmdEDUCB* cb,
                                        SDB_DPSCB *dpsCB = NULL ) ;

         virtual INT32 onDropTextIdx( const CHAR *clFullName,
                                      const CHAR *idxName,
                                      _pmdEDUCB *cb,
                                      SDB_DPSCB *dpscb = NULL ) ;

         virtual INT32 onInsert( const CHAR *clFullName, const CHAR *idxName,
                                 BSONObj &object, bson::OID &oid,
                                 INT32 flags, _pmdEDUCB* cb,
                                 SDB_DPSCB *dpscb = NULL ) ;
         virtual INT32 onDelete( const CHAR *clFullName, const CHAR *idxName,
                                 bson::OID &oid, _pmdEDUCB* cb,
                                 SDB_DPSCB *dpscb = NULL ) ;
         virtual INT32 onUpdate( const CHAR *clFullName, const CHAR *idxName,
                                 BSONObj &object, bson::OID &oid, INT32 flags,
                                 _pmdEDUCB* cb, SDB_DPSCB *dpscb = NULL ) ;
         virtual INT32 onTruncate( const CHAR *clFullName, const CHAR *idxName,
                                   _pmdEDUCB* cb,
                                   SDB_DPSCB *dpscb = NULL ) ;

         static INT32 buildNames( const CHAR *origCLFullName,
                                  const CHAR *idxName,
                                  string &cappedCSName,
                                  string &cappedCLName ) ;
      private:
         INT32 _addOprRecord( const CHAR *name,
                              _dmsExtOprType oprType,
                              pmdEDUCB *cb,
                              const bson::OID *dataOID,
                              const BSONObj *dataObj,
                              SDB_DPSCB *dpsCB = NULL ) ;
         void _getTextIdxCSList( const monCSSimple &csInfo,
                                 vector<string> &csNameVec ) ;
   } ;
   typedef _rtnExtDataHandler rtnExtDataHandler ;

   rtnExtDataHandler* getRtnExtDataHandler() ;
}

#endif /* RTN_EXTDATAHANDLER_HPP__ */

