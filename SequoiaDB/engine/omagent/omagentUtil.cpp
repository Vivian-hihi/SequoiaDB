#include "omagentUtil.hpp"


namespace CLSMGR
{
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

   INT32 omagentInitCommand ( _omagentCommand *pCommand )
   {

   }

   INT32 omagentRunCommand ( _omagentCommand *pCommand )
   {

   }

   INT32 omagentReleaseCommand ( _omagentCommand **ppCommand )
   {

   }



}
