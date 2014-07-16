#ifndef OMAGENT_HELPER_HPP_
#define OMAGENT_HELPER_HPP_

#include "core.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "ossTypes.hpp"
#include "omagentCommand.hpp"


namespace CLSMGR
{
   BOOLEAN omagentIsCommand ( const CHAR *name ) ;

   INT32 omagentParseCommand ( const CHAR *name,
                               _omagentCommand **ppCommand ) ;

   INT32 omagentInitCommand ( _omagentCommand *pCommand ,INT32 flags,
                              INT64 numToSkip,
                              INT64 numToReturn, const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff, const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

//   INT32 omagentRunCommand ( _omagentCommand *pCommand, omagentObjBuff &objBuff ) ;
   INT32 omagentRunCommand ( _omagentCommand *pCommand, CHAR **ppBody,
                             INT32 &bodyLen, INT32 &returnNum ) ;

   INT32 omagentReleaseCommand ( _omagentCommand **ppCommand ) ;

   // build reply buffer
   INT32 omagentBuildReplyMsgBody ( CHAR **ppBuffer, INT32 *bufferSize,
                                    SINT32 numReturned,
                                    vector<BSONObj> *objList ) ;
   INT32 omagentBuildReplyMsgBody ( CHAR **ppBuffer, INT32 *bufferSize,
                                    SINT32 numReturned,
                                    const BSONObj *bsonobj ) ;

}

#endif // OMAGENT_HELPER_HPP_
