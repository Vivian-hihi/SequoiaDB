
#include "core.hpp"
#include "rtn.hpp"
#include "pd.hpp"


namespace engine
{

   INT32 rtnMsg ( MsgOpMsg *pMsg )
   {
      INT32 rc = SDB_OK ;

      if ( pMsg->header.messageLength < sizeof( MsgHeader ) )
      {
         PD_LOG( PDERROR, "Recieve invalid msg[length: %d]",
                 pMsg->header.messageLength ) ;
         rc = SDB_INVALIDARG ;
      }
      else
      {
         CHAR *message = &pMsg->msg[0] ;
         message[ pMsg->header.messageLength - sizeof(MsgHeader) - 1 ] = 0 ;
         PD_LOG ( _curPDLevel, "%s", message ) ;
      }
      return rc ;
   }

}

