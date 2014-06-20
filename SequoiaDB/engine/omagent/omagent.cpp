#include "core.hpp"
#include "ossUtil.hpp"
#include "msgMessage.hpp"
#include "omagent.hpp"

namespace CLSMGR
{

/*
   omagent control func
*/
   BOOLEAN omagentIsCommand ( const CHAR *name ) ;

   BOOLEAN omagentParseCommand ( const CHAR *name,
                                 _omagentCommand **ppCommand ) ;

   INT32 omagentInitCommand ( _omagentCommand *pCommand ) ;

   INT32 omagentRunCommand ( _omagentCommand *pCommand ) ;

   INT32 omagentReleaseCommand ( _omagentCommand **ppCommand ) ;

/*
   CM and OM entry pointers
*/
   INT32 processOmgentRequest( CHAR *pReceiveBuffer,
                               SINT32 packetSize,
                               MsgOpReply &replyHeader,
                               BSONObj **ppErrorObj )
   {
      INT32 rc = SDB_OK ;

      INT32 flags = 0 ;
      SINT64 numToSkip = -1 ;
      SINT64 numToReturn = -1 ;
      CHAR *pCollectionName = NULL ;
      CHAR *pQuery = NULL ;
      CHAR *pFieldSelector = NULL ;
      CHAR *pOrderByBuffer = NULL ;
      CHAR *pHintBuffer = NULL ;

      SDB_ASSERT ( pReceiveBuffer, "PReceiveBuffer is NULL" ) ;
      ossPrintf ( "omsvc request received" ) ;
      // extract command
      rc = msgExtractQuery ( pReceiveBuffer, &flags, &pCollectionName,
                             &numToSkip, &numToReturn, &pQuery,
                             &pFieldSelector, &pOrderByBuffer,
                             &pHintBuffer ) ;
      if ( rc )
      {
         ossPrintf ( "Failed to read omsvc packet" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // 
   done:
      return rc ;
   error:
      goto done ;
   }


}
