#ifndef SPTCOMMON_HPP__
#define SPTCOMMON_HPP__

#include "core.h"
#include "jsapi.h"

#define CMD_HELP           "help"
#define CMD_QUIT           "quit"
#define CMD_QUIT1          "quit;"
#define CMD_CLEAR          "clear"
#define CMD_CLEAR1         "clear;"
#define CMD_CLEARHISTORY   "history-c"
#define CMD_CLEARHISTORY1  "history-c;"

namespace engine
{
   /*
      Global function define
   */
   const CHAR *sdbGetErrMsg() ;
   void  sdbSetErrMsg( const CHAR *err ) ;
   BOOLEAN sdbIsErrMsgEmpty() ;

   INT32 sdbGetErrno() ;
   void  sdbSetErrno( INT32 errNum ) ;

   // clear msg and errno
   void  sdbClearErrorInfo() ;

   BOOLEAN  sdbNeedPrintError() ;
   void     sdbSetPrintError( BOOLEAN print ) ;

   void     sdbSetReadData( BOOLEAN hasRead ) ;
   BOOLEAN  sdbHasReadData() ;

   void     sdbSetNeedClearErrorInfo( BOOLEAN need ) ;
   BOOLEAN  sdbIsNeedClearErrorInfo() ;

   void     sdbReportError( JSContext *cx, const char *msg,
                            JSErrorReport *report ) ;

}

#endif //SPTCOMMON_HPP__

