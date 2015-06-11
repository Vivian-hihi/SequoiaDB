#include "baseCommand.hpp"

baseCommand::baseCommand( const CHAR *name, const CHAR *secondName )
{
   commandMgr *mgr = commandMgr::instance() ;
   if ( NULL != mgr )
   {
      mgr->addCommand( name, this ) ;
      _name = name ;
      if ( NULL != secondName )
      {
         mgr->addCommand( secondName, this ) ;
      }
   }
}

commandMgr* commandMgr::instance()
{
   static commandMgr mgr ;
   return &mgr ;
}
