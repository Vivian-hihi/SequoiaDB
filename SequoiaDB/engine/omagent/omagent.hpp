#ifndef OMAGENT_HPP_
#define OMAGENT_HPP_

#include "core.hpp"

namespace CLSMGR
{

/*
   omagent control func
*/

   BOOLEAN omagentIsCommand ( const CHAR *name ) ;

   BOOLEAN omagentParseCommand ( const CHAR *name,
                                 _omagentCommand **ppCommand ) ;

   INT32 omagentInitCommand ( _omagentCommand *pCommand ) ;
/*
   INT32 omagentInitCommand ( _omagentCommand *pCommand ,INT32 flags, INT64
numToSkip,
                              INT64 numToReturn, const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff, const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
*/
   INT32 omagentRunCommand ( _omagentCommand *pCommand ) ;

   INT32 omagentReleaseCommand ( _omagentCommand **ppCommand ) ;

/*
   CM and OM entry pointers
*/
   INT32 omagentEntryPoint () ;
   INT32 cmEntryPoint () ;


}





#endif // OMAGENT_HPP_
