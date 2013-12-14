
#include "rtn.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"

using namespace bson;

namespace engine
{
   INT32 rtnAggregate( const CHAR *pCollectionName, BSONObj &objs,
                     INT32 objNum, SINT32 flags, pmdEDUCB *cb,
                     SDB_DMSCB *dmsCB, SINT64 &contextID )
   {
      INT32 rc = SDB_OK;
      rc = pmdGetKRCB()->getAggrCB()->build( objs, objNum, pCollectionName,
                                             cb, contextID );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to execute aggregation operation(rc=%d)",
                  rc );

   done:
      return rc;
   error:
      goto done;
   }
}
