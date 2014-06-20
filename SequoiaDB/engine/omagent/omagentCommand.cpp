#include "omagentCommand.cpp"


using namespace bson ;

namespace CLSMGR
{
   // _omagentCommand
   _omagentCommand::_omagentCommand ()
   {
   }

   _omagentCommand::~_omagentCommand ()
   {
   }

   // _omagentCmdAssit
   _omagentCmdAssit::_omagentCmdAssit ( OA_NEW_FUNC pFunc )
   {
      if ( pFunc )
      {
         _omagentCommand *pCommand = (*pFunc)() ;
         if ( pCommand )
         {
            getOACmdBuilder()->_register ( pCommand->name(), pFunc ) ;
            SDB_OSS_DEL pCommand ;
            pCommand = NULL ;
         }
      }
   }

   _omagentCmdAssit::~_omagentCmdAssit ()
   {
   }

   // _omagentCmdBuilder
   _omagentCmdBuilder::_omagentCmdBuilder ()
   {
   }

   _omagentCmdBuilder::~omagentCmdBuilder ()
   {
      // TODO: do i need to release memory in map ?
   }

   _omagentCommand* _omagentCmdBuilder::create ( const CHAR *command )
   {
      OA_NEW_FUNC pFunc = _find ( command ) ;
      if ( pFunc )
      {
         return (*pFunc)() ;
      }
      return NULL ;
   }

   void _omagentCmdBuilder::release ( const CHAR *pCommand )
   {
      if ( pCommand )
      {
         SDB_OSS_DEL pCommand ;
      }
   }

   INT32 _omagentCmdBuilder::_register ( const CHAR *name, OA_NEW_FUNC pFunc )
   {
      INT32 rc = SDB_OK ;
/*
      if (_cmdMap.cout( name ) > 0)
      {
         ossPrintf ( "Failed to register omagent command %s, \
                      already exist"OSS_NEWLINE, name ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      // register
      _cmdMap[name] = pFunc ;
*/
      std::pair<MAP_OACMD_IT, BOOLEAN> ret ;
      ret = _cmdMap.insert( std::pair<const CHAR*, OA_NEW_FUNC>(name, pFunc) ) ;
      if ( FALSE == ret.second )
      {
         ossPrintf ( "Failed to register omagent command %s, \
                      already exist"OSS_NEWLINE, name ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto error ;
   }

   OA_NEW_FUNC _omagentCmdBuilder::_find ( const CHAR *name )
   {
      if ( name )
      {
         MAP_OACMD_IT it ;
         it = _cmdMap.find( name ) ;
         if ( it == _cmdMap.end() )
            return NULL ;
         else
            it->second ;
      }
      return NULL ;
   }

   // command list:
   IMPLEMENT_OACMD_AUTO_REGISTER( _omagentAddHost )


   // _omagentAddHost
   _omagentAddHost::_omagentAddHost()
   {
      ossPrintf ( "In add host constructor."OSS_NEWLINE ) ;
   }

   _omagentAddHost::~_omagentAddHost()
   {
      ossPrintf ( "In add host destructor."OSS_NEWLINE ) ;
   }

   _omagentAddHost::init ()
   {
      ossPrintf ( "In add host init."OSS_NEWLINE ) ;
   }

   _omagentAddHost::doit ()
   {
      ossPrintf ( "In add host doit."OSS_NEWLINE ) ;
   }


}
