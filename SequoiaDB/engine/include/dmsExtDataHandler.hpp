#ifndef DMS_EXTDATAHANDLER_HPP__
#define DMS_EXTDATAHANDLER_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "dpsLogWrapper.hpp"
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
         virtual INT32 onCreate( const CHAR *clFullName, const CHAR *idxName,
                                 pmdEDUCB* cb, SDB_DPSCB *dpsCB = NULL ) = 0 ;
         virtual INT32 onDrop( const CHAR *clFullName, const CHAR *idxName,
                               _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) = 0 ;
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

