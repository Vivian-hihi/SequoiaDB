#ifndef _SDB_FAP_MONGO_COMMANDS_HPP_
#define _SDB_FAP_MONGO_COMMANDS_HPP_

#include "baseCommand.hpp"

#define __DECLARE_COMMAND( cmd, secondName, clsName )                \
class clsName : public baseCommand                                   \
{                                                                    \
public:                                                              \
   clsName() : baseCommand( cmd, secondName )                        \
   {}                                                                \
                                                                     \
   virtual INT32 convert( msgParser &parser ) ;                      \
                                                                     \
   virtual INT32 buildMsg( msgParser &parser, msgBuffer &sdbMsg ) ;  \
                                                                     \
   virtual INT32 doCommand( void *pData = NULL ) ;                   \
} ;

#define __DECLARE_COMMAND_VAR( commandClass, var )                \
      commandClass var ;

#define DECLARE_COMMAND( command )                                \
      __DECLARE_COMMAND( #command, NULL, command##Command )

#define DECLARE_COMMAND_ALAIS( command, secondName )              \
      __DECLARE_COMMAND( #command, secondName, command##Command)

#define DECLARE_COMMAND_VAR( command )                            \
      __DECLARE_COMMAND_VAR( command##Command, command##Cmd )

///////////////////////////////////////////////////////////////////
// declare all command supported

DECLARE_COMMAND( insert )
DECLARE_COMMAND( delete )
DECLARE_COMMAND( update )
DECLARE_COMMAND( query )
DECLARE_COMMAND( getMore )
DECLARE_COMMAND( killCursors )

// other command
DECLARE_COMMAND( getnonce )
DECLARE_COMMAND( authenticate )
DECLARE_COMMAND( createUser )
DECLARE_COMMAND( removeUser )
DECLARE_COMMAND( listUsers )
DECLARE_COMMAND( create )
DECLARE_COMMAND( createCS )
//DECLARE_COMMAND( createCollection )
DECLARE_COMMAND( listCollection )
DECLARE_COMMAND( drop )
DECLARE_COMMAND( count )
DECLARE_COMMAND( aggregate )
DECLARE_COMMAND( dropDatabase )
DECLARE_COMMAND( createIndexes )
DECLARE_COMMAND_ALAIS( deleteIndexes, "dropIndexes")
DECLARE_COMMAND( listIndexes )
DECLARE_COMMAND( getlasterror )
DECLARE_COMMAND_ALAIS( ismaster, "isMaster" )
DECLARE_COMMAND( ping )

#endif
