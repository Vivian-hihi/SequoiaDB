#include "omagentHelper.hpp"
#include "omagentUtil.hpp"
namespace CLSMGR
{

/*
   omagent control func
*/
   BOOLEAN omagentIsCommand ( const CHAR *name )
   {
      if ( name && '$' == name[0] )
      {
         return TRUE ;
      }
      return FALSE ;
   }

   INT32 omagentParseCommand ( const CHAR *name, _omagentCommand **ppCommand )
   {
      INT32 rc = SDB_INVALIDARG ;
      if ( ppCommand && omagentIsCommand ( name ) )
      {
         *ppCommand = getOmagentCmdBuilder()->create ( &name[1] ) ;
         if ( *ppCommand )
         {
            rc = SDB_OK ;
         }
      }
      return rc ;
   }


   INT32 omagentInitCommand ( _omagentCommand *pCommand ,INT32 flags,
                              INT64 numToSkip,
                              INT64 numToReturn, const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff, const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff )
   {
      INT32 rc = SDB_OK ;
      if ( !pCommand )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      try
      {
         pCommand->init ( flags, numToSkip, numToReturn, pMatcherBuff,
                          pSelectBuff, pOrderByBuff, pHintBuff ) ;
      }
      catch ( std::exception &e )
      {
            PD_LOG ( PDERROR, "omagent init command[%s] exception[%s]",
                     pCommand->name(), e.what() ) ;
            rc = SDB_INVALIDARG ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

//   INT32 omagentRunCommand ( _omagentCommand *pCommand, omagentObjBuff &objBuff )
   INT32 omagentRunCommand ( _omagentCommand *pCommand, CHAR **ppBody,
                             INT32 &bodyLen, INT32 &returnNum )
   {
      INT32 rc = SDB_OK ;
      if ( !pCommand )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      try
      {
//         rc = pCommand->doit ( objBuff ) ;
         rc = pCommand->doit ( ppBody, bodyLen, returnNum ) ;
      }
      catch ( std::exception &e )
      {
            PD_LOG ( PDERROR, "omagent run command[%s] exception[%s]",
                     pCommand->name(), e.what() ) ;
            rc = SDB_INVALIDARG ;
      }

      if ( SDB_OK != rc  )
      {
         PD_LOG( PDERROR, "omagent run command[%s] failed[rc=%d]",
                 pCommand->name(), rc ) ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omagentReleaseCommand ( _omagentCommand **ppCommand )
   {
     INT32 rc = SDB_OK ;
      if ( ppCommand && *ppCommand )
      {
         getOmagentCmdBuilder()->release( *ppCommand ) ;
         *ppCommand = NULL ;
      }
      return rc ;
   }

   INT32 omagentBuildReplyMsgBody ( CHAR **ppBuffer, INT32 *bufferSize,
                                    SINT32 numReturned,
                                    vector<BSONObj> *objList )
   {
      SDB_ASSERT ( ppBuffer && bufferSize && objList, "Invalid input" ) ;
      INT32 rc             = SDB_OK ;
      INT32 packetLength = ossRoundUpToMultipleX ( 0, 4 ) ;
      if ( packetLength < 0 )
      {
         PD_LOG ( PDERROR, "Packet size overflow" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      for ( UINT32 i = 0; i < objList->size(); i ++ )
      {
         rc = checkBuffer ( ppBuffer, bufferSize, packetLength ) ;
         if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to check buffer" ) ;
            goto error ;
         }
         ossMemcpy ( &((*ppBuffer)[packetLength]), (*objList)[i].objdata(),
                                                   (*objList)[i].objsize() ) ;
         packetLength += ossRoundUpToMultipleX ( (*objList)[i].objsize(), 4 ) ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 omagentBuildReplyMsgBody ( CHAR **ppBuffer, INT32 *bufferSize,
                                    SINT32 numReturned, const BSONObj *bsonobj )
   {
      SDB_ASSERT ( ppBuffer && bufferSize, "Invalid input" ) ;
      INT32 rc           = SDB_OK ;
      INT32 offset       = 0 ;
      INT32 packetLength = ossRoundUpToMultipleX ( bsonobj->objsize(), 4 ) ;
      if ( packetLength < 0 )
      {
         PD_LOG ( PDERROR, "Packet size overflow" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      rc = checkBuffer ( ppBuffer, bufferSize, packetLength ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to check buffer, rc: %d", rc ) ;
         goto error ;
      }
      if ( numReturned != 0 )
      {
         ossMemcpy ( *ppBuffer, bsonobj->objdata(), bsonobj->objsize() ) ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }

}
