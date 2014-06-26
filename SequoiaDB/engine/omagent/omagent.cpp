#include "core.hpp"
#include "ossUtil.hpp"
#include "msgMessage.hpp"
#include "omagent.hpp"

namespace CLSMGR
{
   // _omagentObjBuff
/*
   _omagentObjBuff::_omagentObjBuf ( _omagentObjBuff &right )
   {

   }

   _omagentObjBuff& _omagentObjBuff::operator= ( _omagentObjBuff &right )
   {

   }
*/
   _omagentObjBuff::~_omagentObjBuff ()
   {
      if ( _pBuff )
      {
         SDB_OSS_DEL[] _pBuff ;
         _pBuff = NULL ;
      }
      _buffLen = 0 ;
      _recordNum = 0 ;
   }

   INT32 _omagentObjBuff::setObj( const CHAR *pBuff,
                                  INT32 buffLen, INT32 recordNum )
   {
      INT32 rc = SDB_OK ;
      if ( NULL == pBuff || 0 > buffLen || 0 > recordNum )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( NULL != _pBuff )
         SDB_OSS_DEL[] _pBuff ;
      _pBuff = SDB_OSS_NEW CHAR[buffLen] ;
      ossMemcpy ( _pBuff, pBuff, buffLen ) ;
      _buffLen = buffLen ;
      _recordNum = recordNum ;
   done:
     return rc ;
   error:
     goto done ;
   }

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
            ossPrintf ( "omagent init command[%s] exception[%s]"OSS_NEWLINE,
                        pCommand->name(), e.what() ) ;
            rc = SDB_INVALIDARG ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omagentRunCommand ( _omagentCommand *pCommand, omagentObjBuff &objBuff )
   {
      INT32 rc = SDB_OK ;
      if ( !pCommand )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      try
      {
         rc = pCommand->doit ( objBuff ) ;
      }
      catch ( std::exception &e )
      {
            ossPrintf ( "omagent run command[%s] exception[%s]"OSS_NEWLINE,
                        pCommand->name(), e.what() ) ;
            rc = SDB_INVALIDARG ;
      }

      if ( SDB_OK != rc  )
      {
         ossPrintf ( "omagent run command[%s] failed[rc=%d]"OSS_NEWLINE,
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
         getRtnCmdBuilder()->release( *ppCommand ) ;
         *ppCommand = NULL ;
      }
      return rc ;
   }

}
