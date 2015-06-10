#ifndef _SDB_FAP_MONGO_BASE_COMMAND_HPP_
#define _SDB_FAP_MONGO_BASE_COMMAND_HPP_

#include <map>
#include "util.hpp"
#include "mongodef.hpp"
#include "parser.hpp"

class baseCommand : public SDBObject
{
public:
   baseCommand( const CHAR *name, const CHAR *secondName = NULL ) ;

   virtual ~baseCommand() {} ;

   const CHAR *name() const
   {
      return _name ;
   }

   virtual INT32 convert( msgParser &parser )
   {
      return SDB_OK ;
   }

   virtual INT32 buildMsg( msgParser &parser, msgBuffer &sdbMsg )
   {
      return SDB_OK ;
   }

   virtual INT32 doCommand( void *pData = NULL )
   {
      return SDB_OK ;
   }

private:
   const CHAR *_name ;
} ;


class commandMgr : public SDBObject
{
public:
   static commandMgr *instance() ;

   void addCommand( const std::string &name, baseCommand *cmd )
   {
      baseCommand *tmp = _cmdMap[name] ;
      if ( NULL != tmp )
      {
         // should never hit here!
      }
      _cmdMap[name] = cmd ;
   }

   baseCommand *findCommand( const std::string &name )
   {
      baseCommand *cmd = NULL ;
      std::map< std::string, baseCommand* >::iterator it = _cmdMap.find( name ) ;
      if ( _cmdMap.end() != it )
      {
         cmd = it->second ;
      }
      return cmd ;
   }

private:

   commandMgr()
   {
      _cmdMap.clear() ;
   }

   ~commandMgr()
   {
      _cmdMap.clear() ;
   }

private:
   std::map< std::string, baseCommand* > _cmdMap ;
} ;


#endif
