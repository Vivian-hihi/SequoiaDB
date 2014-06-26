#ifndef OMAGENT_COMMAND_HPP_
#define OMAGENT_COMMAND_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "sptApi.hpp"
#include <map>

namespace CLSMGR
{
   #define DECLARE_OACMD_AUTO_REGISTER()                       \
      public:                                                  \
         static _omagentCommand *newThis () ;                  \

   #define IMPLEMENT_OACMD_AUTO_REGISTER(theClass)             \
      _omagentCommand* theClass::newThis ()                    \
      {                                                        \
         return SDB_OSS_NEW theClass() ;                       \
      }                                                        \
      _omagentCmdAssit theClass##Assit ( theClass::newThis ) ; \

   // _omagentCommand
   class _omagentCommand : public SDBObject
   {
      public:
         _omagentCommand () ;
         virtual ~_omagentCommand () ;

      public:
         virtual const CHAR * name () = 0 ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) = 0 ;
         virtual INT32 doit ( omagentObjBuff &objBuff ) = 0 ;
      protected:
         static _sptScope *scope ;
   };

   _sptScope* _omagentCommand::scope = NULL ;

   typedef _omagentCommand* (*OA_NEW_FUNC) () ;

   // _omagentCmdAssit
   class _omagentCmdAssit : public SDBObject
   {
      public:
         _omagentCmdAssit ( OA_NEW_FUNC ) ;
         ~_rtnCmdAssit () ;
   };

   typedef std::map<const CHAR*, OA_NEW_FUNC> MAP_OACMD ;
   typedef std::map<const CHAR*, OA_NEW_FUNC>::iterator MAP_OACMD_IT ;

   // _omagentCmdBuilder
   class _omagentCmdBuiler : public SDBObject
   {
      friend class _omagentCmdAssit ;

      public:
         _omagentCmdBuilder () ;
         ~_omagentCmdBuilder () ;

      public:
         _omagentCommand *create ( const CHAR *command ) ;

         void release ( _rtnCommand *pCommand ) ;

         INT32 _register ( const CHAR *name, OA_NEW_FUNC pFunc ) ;

         OA_NEW_FUNC _find ( const CHAR * name ) ;

      private:
         MAP_OACMD _cmdMap ;
   };

   _omagentCmdBuilder * getOACmdBuilder () ;

   // spiderMonkey engine
   _sptScope* getScope()
   {

   }

   // _omagentAddHost
   class _omagentAddHost : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER()

      public:
         _omagentAddHost () ;
         ~_omagentAddHost () ;

         virtual const CHAR * name () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( omagentObjBuff &objBuff ) ;
   };
}


#endif
