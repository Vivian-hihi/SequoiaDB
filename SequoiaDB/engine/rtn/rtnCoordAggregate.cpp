
#include "rtnCoordAggregate.hpp"
#include "msgMessage.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"

using namespace bson;
namespace engine
{
   INT32 rtnCoordAggregate::execute( CHAR *pReceiveBuffer, SINT32 packSize,
                                    CHAR **ppResultBuffer, pmdEDUCB *cb,
                                    MsgOpReply &replyHeader,
                                    BSONObj **ppErrorObj )
   {
      INT32 rc = SDB_OK;
      MsgHeader *pHeader = (MsgHeader *)pReceiveBuffer;
      CHAR *pCollectionName = NULL;
      CHAR *pObjs = NULL;
      INT32 count = 0;
      BSONObj objs;
      SINT64 contextID = -1;

      replyHeader.contextID = -1;
      replyHeader.flags = SDB_OK;
      replyHeader.numReturned = 0;
      replyHeader.startFrom = 0;
      replyHeader.header.messageLength = sizeof( MsgOpReply );
      replyHeader.header.opCode = MSG_BS_QUERY_RES;
      replyHeader.header.requestID = pHeader->requestID;
      replyHeader.header.routeID.value = 0;
      replyHeader.header.TID = pHeader->TID;

      rc = msgExtractAggrRequest( pReceiveBuffer, &pCollectionName, &pObjs, count );
      PD_RC_CHECK( rc, PDERROR, "failed to parse aggregate request(rc=%d)", rc );

      try
      {
         objs = BSONObj( pObjs );
      }
      catch ( std::exception &e )
      {
         PD_RC_CHECK( rc, PDERROR,
                     "failed to execute aggregate, received unexpecte error:%s",
                     e.what() );
      }
      rc = pmdGetKRCB()->getAggrCB()->build( objs, count, pCollectionName,
                                             cb, contextID );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to execute aggregation operation(rc=%d)",
                  rc );
      replyHeader.contextID = contextID;
   done:
      return rc;
   error:
      replyHeader.flags = rc;
      goto done;
   }
}
