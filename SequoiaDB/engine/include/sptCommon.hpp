#ifndef SPTCOMMON_HPP__
#define SPTCOMMON_HPP__

#include "core.h"

#define CMD_HELP           "help"
#define CMD_QUIT           "quit"
#define CMD_QUIT1          "quit;"
#define CMD_CLEAR          "clear"
#define CMD_CLEAR1         "clear;"
#define CMD_CLEARHISTORY   "history-c"
#define CMD_CLEARHISTORY1  "history-c;"


INT32 setProgramName( const CHAR *name ) ;
const CHAR* getProgramName() ;
INT32 getProgramPath( CHAR *pOutputPath ) ;
//INT32 getProgramPath( const CHAR *pInputPath, const CHAR *pOutputPath ) ;

#endif //SPTCOMMON_HPP__
