#ifndef OMAGENT_COMMAND_HPP_
#define OMAGENT_COMMAND_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossTypes.h"
#include "ossUtil.h"
#include "../bson/bson.h"
#include "sptApi.hpp"
#include "ossMem.h"
#include "omagent.hpp"
#include "omagentMsgDef.hpp"
#include <map>
#include <string>

using namespace engine ;
using namespace bson ;

namespace CLSMGR
{
//   class omagentObjBuff ;

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
         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) = 0 ;

     protected:
         _sptScope *_scope ;
         const CHAR *_jsFileName ;
         CHAR *_fileBuff ;
         UINT32 _buffSize ;
         UINT32 _readSize ;
         std::vector<BSONObj> _hosts ;
         std::string _content ;

   };

   typedef _omagentCommand* (*OA_NEW_FUNC) () ;

   // _omagentCmdAssit
   class _omagentCmdAssit : public SDBObject
   {
      public:
         _omagentCmdAssit ( OA_NEW_FUNC ) ;
         virtual ~_omagentCmdAssit () ;
   };

/*
   bool _fncomp ( const CHAR* &lhs, const CHAR* rhs ) const
   {
      INT32 ret = ossStrcmp( lhs, rhs ) ;
      if ( 0 <= ret )
         return true ;
      else
         return false ;
   }
*/
   struct _classComp
   {
      bool operator()( const CHAR *lhs, const CHAR *rhs ) const
      {
         return ossStrcmp( lhs, rhs ) < 0 ;
      }
   } ;

   typedef std::map<const CHAR*, OA_NEW_FUNC, _classComp> MAP_OACMD ;
   typedef std::map<const CHAR*, OA_NEW_FUNC, _classComp>::iterator MAP_OACMD_IT ;

   // _omagentCmdBuilder
   class _omagentCmdBuilder : public SDBObject
   {
      friend class _omagentCmdAssit ;

      public:
         _omagentCmdBuilder () ;
         ~_omagentCmdBuilder () ;

      public:
         _omagentCommand *create ( const CHAR *command ) ;

         void release ( const _omagentCommand *pCommand ) ;

         INT32 _register ( const CHAR *name, OA_NEW_FUNC pFunc ) ;

         OA_NEW_FUNC _find ( const CHAR * name ) ;

      private:

         MAP_OACMD _cmdMap ;
   };

   // get omagent command builder
   _omagentCmdBuilder* getOmagentCmdBuilder() ;

   // _omagentAddHost
   class _omagentAddHost : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER()

      public:
         _omagentAddHost () ;
         ~_omagentAddHost () ;

         virtual const CHAR * name () { return "add host" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

   };


   // _omagentScanHost
   class _omagentScanHost : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER()

      public:
         _omagentScanHost () ;
         ~_omagentScanHost () ;

         virtual const CHAR* name () { return "scan host" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;

//         virtual INT32 doit ( omagentObjBuff &objBuff ) ;
         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;
      private:
         _sptScope *_scope ;
         const CHAR *_jsFileName ;
         CHAR *_fileBuff ;
         UINT32 _buffSize ;
         UINT32 _readSize ;
         std::vector<BSONObj> _hosts ;
         std::string _content ;

   };

   // _omagentInstallRemoteAgent
   class _omagentInstallRemoteAgent : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()

      public:
         _omagentInstallRemoteAgent () ;
         ~_omagentInstallRemoteAgent () ;

         virtual const CHAR* name () { return "install remote agent" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                             const CHAR *pMatcherBuff,
                             const CHAR *pSelectBuff,
                             const CHAR *pOrderByBuff,
                             const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

         INT32 getRemoteAgentStatus ( const CHAR *pIp, const CHAR *pUsername,
                                      const CHAR *pPasswork, BSONObj &result ) ;

         CHAR* getVersion() { return "1.0" ; }

   } ;

   // _omagentCheckRemoteAgentProcess
   class _omagentCheckRemoteAgentProcess : public _omagentCommand
   {

      public:
         _omagentCheckRemoteAgentProcess () ;
         ~_omagentCheckRemoteAgentProcess () ;

         virtual const CHAR* name () { return "install remote agent" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                             const CHAR *pMatcherBuff,
                             const CHAR *pSelectBuff,
                             const CHAR *pOrderByBuff,
                             const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

         INT32 check ( const CHAR *pIp, const CHAR *pUserName,
                       const CHAR *pPassword, BSONObj &result ) ;

   } ;


   // _omagentInstallAgentProcess
   class _omagentInstallAgentProcess : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omagentInstallAgentProcess () ;
         ~_omagentInstallAgentProcess () ;

         virtual const CHAR* name () { return "install agent process" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                             const CHAR *pMatcherBuff,
                             const CHAR *pSelectBuff,
                             const CHAR *pOrderByBuff,
                             const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

   } ;

   // _omagentRemoveAgentProcess
   class _omagentRemoveAgentProcess : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omagentRemoveAgentProcess () ;
         ~_omagentRemoveAgentProcess () ;

         virtual const CHAR* name () { return "remove agent process" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                             const CHAR *pMatcherBuff,
                             const CHAR *pSelectBuff,
                             const CHAR *pOrderByBuff,
                             const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

   } ;

   // _omagentStopAgentProcess
   class _omagentStopAgentProcess : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omagentStopAgentProcess () ;
         ~_omagentStopAgentProcess () ;

         virtual const CHAR* name () { return "stop agent process" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                             const CHAR *pMatcherBuff,
                             const CHAR *pSelectBuff,
                             const CHAR *pOrderByBuff,
                             const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

   } ;


   // _omagentGetHostInfo
   class _omagentGetHostInfo : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omagentGetHostInfo () ;
         ~_omagentGetHostInfo () ;

         virtual const CHAR* name () { return "get host info" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                             const CHAR *pMatcherBuff,
                             const CHAR *pSelectBuff,
                             const CHAR *pOrderByBuff,
                             const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

   } ;

   // _omagentRegHosts
   class _omagentRegHosts : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omagentRegHosts () ;
         ~_omagentRegHosts () ;

         virtual const CHAR* name () { return "reg hosts info" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                             const CHAR *pMatcherBuff,
                             const CHAR *pSelectBuff,
                             const CHAR *pOrderByBuff,
                             const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

      private:
         INT32 _getHostsTableInfo () ;

         INT32 _getContentForJS ( const CHAR *pIp, std::vector<string> &hostsInfo ) ;

         std::map<string, string> _hostsTableInfo ;

   } ;


   // _omagentGetHostNames
   class _omagentGetHostNames : public _omagentCommand
   {
      DECLARE_OACMD_AUTO_REGISTER ()
      public:
         _omagentGetHostNames () ;
         ~_omagentGetHostNames () ;

         virtual const CHAR* name () { return "get host name" ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                             const CHAR *pMatcherBuff,
                             const CHAR *pSelectBuff,
                             const CHAR *pOrderByBuff,
                             const CHAR *pHintBuff ) ;

         virtual INT32 doit ( CHAR **ppBody, INT32 &bodyLen, INT32 &returnNum ) ;

         INT32 getHostName( const CHAR *pIp, const CHAR *pUserName,
                            const CHAR *pPassword, BSONObj &result ) ;

   } ;


}


#endif
