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
         virtual INT32 onCreate( const CHAR *clFullName, const CHAR *idxName,
                                 pmdEDUCB* cb, SDB_DPSCB *dpsCB = NULL ) ;
         virtual INT32 onDrop( const CHAR *clFullName, const CHAR *idxName,
                               _pmdEDUCB *cb, SDB_DPSCB *dpscb = NULL ) ;
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
      private:
         INT32 _addOprRecord( const CHAR *name,
                              _dmsExtOprType oprType,
                              pmdEDUCB *cb,
                              const bson::OID *dataOID,
                              const BSONObj *dataObj,
                              SDB_DPSCB *dpsCB = NULL ) ;
   } ;
   typedef _rtnExtDataHandler rtnExtDataHandler ;

   rtnExtDataHandler* getRtnExtDataHandler() ;
}

#endif /* RTN_EXTDATAHANDLER_HPP__ */

