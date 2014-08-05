#ifndef OMAGENT_HELPER_HPP_
#define OMAGENT_HELPER_HPP_

#include "core.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "ossTypes.hpp"
#include "omagentCommand.hpp"


namespace engine
{
   BOOLEAN omaIsCommand ( const CHAR *name ) ;

   INT32 omaParseCommand ( const CHAR *name,
                               _omaCommand **ppCommand ) ;

   INT32 omaInitCommand ( _omaCommand *pCommand ,INT32 flags,
                              INT64 numToSkip,
                              INT64 numToReturn, const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff, const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

//   INT32 omagentRunCommand ( _omaCommand *pCommand, omagentObjBuff &objBuff ) ;
   INT32 omaRunCommand ( _omaCommand *pCommand, CHAR **ppBody,
                             INT32 &bodyLen, INT32 &returnNum ) ;

   INT32 omaReleaseCommand ( _omaCommand **ppCommand ) ;

   // build reply buffer
   INT32 omaBuildReplyMsgBody ( CHAR **ppBuffer, INT32 *bufferSize,
                                    SINT32 numReturned,
                                    vector<BSONObj> *objList ) ;
   INT32 omaBuildReplyMsgBody ( CHAR **ppBuffer, INT32 *bufferSize,
                                    SINT32 numReturned,
                                    const BSONObj *bsonobj ) ;

}

#endif // OMAGENT_HELPER_HPP_
