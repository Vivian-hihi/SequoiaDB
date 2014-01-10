/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnCommand.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          13/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_COMMAND_HPP_
#define RTN_COMMAND_HPP_

#include "core.hpp"
#include "oss.hpp"
#include <string>
#include <vector>
#include "../bson/bson.h"
#include "msgDef.h"
#include "dms.hpp"
#include "msg.hpp"
#include "migLoad.hpp"

using namespace bson ;

namespace engine
{

#define NAME_BACKUP_OFFLINE                  CMD_NAME_BACKUP_OFFLINE
#define NAME_CREATE_COLLECTION               CMD_NAME_CREATE_COLLECTION
#define NAME_CREATE_COLLECTIONSPACE          CMD_NAME_CREATE_COLLECTIONSPACE
#define NAME_CREATE_INDEX                    CMD_NAME_CREATE_INDEX
#define NAME_CANCEL_TASK                     CMD_NAME_CANCEL_TASK
#define NAME_DROP_COLLECTION                 CMD_NAME_DROP_COLLECTION
#define NAME_DROP_COLLECTIONSPACE            CMD_NAME_DROP_COLLECTIONSPACE
#define NAME_DROP_INDEX                      CMD_NAME_DROP_INDEX
#define NAME_GET_COUNT                       CMD_NAME_GET_COUNT
#define NAME_GET_INDEXES                     CMD_NAME_GET_INDEXES
#define NAME_GET_DATABLOCKS                  CMD_NAME_GET_DATABLOCKS
#define NAME_GET_QUERYMETA                   CMD_NAME_GET_QUERYMETA
#define NAME_LIST_COLLECTIONS                CMD_NAME_LIST_COLLECTIONS
#define NAME_LIST_COLLECTIONSPACES           CMD_NAME_LIST_COLLECTIONSPACES
#define NAME_LIST_CONTEXTS                   CMD_NAME_LIST_CONTEXTS
#define NAME_LIST_CONTEXTS_CURRENT           CMD_NAME_LIST_CONTEXTS_CURRENT
#define NAME_LIST_SESSIONS                   CMD_NAME_LIST_SESSIONS
#define NAME_LIST_SESSIONS_CURRENT           CMD_NAME_LIST_SESSIONS_CURRENT
#define NAME_LIST_STORAGEUNITS               CMD_NAME_LIST_STORAGEUNITS
#define NAME_LIST_BACKUPS                    CMD_NAME_LIST_BACKUP
#define NAME_LIST_TASKS                      CMD_NAME_LIST_TASK
#define NAME_RENAME_COLLECTION               CMD_NAME_RENAME_COLLECTION
#define NAME_REORG_OFFLINE                   CMD_NAME_REORG_OFFLINE
#define NAME_REORG_ONLINE                    CMD_NAME_REORG_ONLINE
#define NAME_REORG_RECOVER                   CMD_NAME_REORG_RECOVER
#define NAME_SHUTDOWN                        CMD_NAME_SHUTDOWN
#define NAME_SNAPSHOT_CONTEXTS               CMD_NAME_SNAPSHOT_CONTEXTS
#define NAME_SNAPSHOT_CONTEXTS_CURRENT       CMD_NAME_SNAPSHOT_CONTEXTS_CURRENT
#define NAME_SNAPSHOT_DATABASE               CMD_NAME_SNAPSHOT_DATABASE
#define NAME_SNAPSHOT_RESET                  CMD_NAME_SNAPSHOT_RESET
#define NAME_SNAPSHOT_SESSIONS               CMD_NAME_SNAPSHOT_SESSIONS
#define NAME_SNAPSHOT_SESSIONS_CURRENT       CMD_NAME_SNAPSHOT_SESSIONS_CURRENT
#define NAME_SNAPSHOT_SYSTEM                 CMD_NAME_SNAPSHOT_SYSTEM
#define NAME_SNAPSHOT_COLLECTIONS            CMD_NAME_SNAPSHOT_COLLECTIONS
#define NAME_SNAPSHOT_COLLECTIONSPACES       CMD_NAME_SNAPSHOT_COLLECTIONSPACES
#define NAME_TEST_COLLECTION                 CMD_NAME_TEST_COLLECTION
#define NAME_TEST_COLLECTIONSPACE            CMD_NAME_TEST_COLLECTIONSPACE
#define NAME_SET_PDLEVEL                     CMD_NAME_SET_PDLEVEL
#define NAME_SPLIT                           CMD_NAME_SPLIT
#define NAME_TRACE_START                     CMD_NAME_TRACE_START
#define NAME_TRACE_STOP                      CMD_NAME_TRACE_STOP
#define NAME_TRACE_STATUS                    CMD_NAME_TRACE_STATUS
#define NAME_EXPORT_CONFIGURATION            CMD_NAME_EXPORT_CONFIG
#define NAME_REMOVE_BACKUP                   CMD_NAME_REMOVE_BACKUP

// the commands that does not supported by data nodes or standalone mode
#define NAME_CREATE_GROUP                    CMD_NAME_CREATE_GROUP
#define NAME_REMOVE_GROUP                    CMD_NAME_REMOVE_GROUP
#define NAME_CREATE_NODE                     CMD_NAME_CREATE_NODE
#define NAME_REMOVE_NODE                     CMD_NAME_REMOVE_NODE
#define NAME_UPDATE_NODE                     CMD_NAME_UPDATE_NODE
#define NAME_ACTIVE_GROUP                    CMD_NAME_ACTIVE_GROUP
#define NAME_START_NODE                      CMD_NAME_STARTUP_NODE
#define NAME_SHUTDOWN_NODE                   CMD_NAME_SHUTDOWN_NODE
#define NAME_SHUTDOWN_GROUP                  CMD_NAME_SHUTDOWN_GROUP
#define NAME_GET_CONFIG                      CMD_NAME_GET_CONFIG
#define NAME_LIST_DOMAINS                    CMD_NAME_LIST_DOMAINS
#define NAME_LIST_GROUPS                     CMD_NAME_LIST_GROUPS
#define NAME_ALTER_COLLECTION                CMD_NAME_ALTER_COLLECTION
#define NAME_CREATE_CATAGROUP                CMD_NAME_CREATE_CATA_GROUP
#define NAME_CREATE_DOMAIN                   CMD_NAME_CREATE_DOMAIN
#define NAME_REMOVE_DOMAIN                   CMD_NAME_REMOVE_DOMAIN
#define NAME_ADD_DOMAIN_GROUP                CMD_NAME_ADD_DOMAIN_GROUP
#define NAME_REMOVE_DOMAIN_GROUP             CMD_NAME_REMOVE_DOMAIN_GROUP
#define NAME_SNAPSHOT_CATA                   CMD_NAME_SNAPSHOT_CATA
#define NAME_LINK_COLLECTION                 CMD_NAME_LINK_CL
#define NAME_UNLINK_COLLECTION               CMD_NAME_UNLINK_CL
#define NAME_WAITTASK                        CMD_NAME_WAITTASK

#if defined (_DEBUG)
// the commands for debug
#define NAME_DEBUG_QUERY                     CMD_NAME_DEBUG_QUERY
#endif
#define NAME_JSON_LOAD                       CMD_NAME_JSON_LOAD

   enum RTN_COMMAND_TYPE
   {
      CMD_BACKUP_OFFLINE                     = 1,

      CMD_CREATE_COLLECTION                  = 10,
      CMD_CREATE_COLLECTIONSPACE             = 11,
      CMD_CREATE_INDEX                       = 12,
      CMD_ALTER_COLLECTION                   = 13,

      CMD_DROP_COLLECTION                    = 20,
      CMD_DROP_COLLECTIONSPACE               = 21,
      CMD_DROP_INDEX                         = 22,

      CMD_GET_COUNT                          = 30,
      CMD_GET_INDEXES                        = 31,
      CMD_GET_DATABLOCKS                     = 32,
      CMD_GET_QUERYMETA                      = 33,

      CMD_LIST_COLLECTIONS                   = 40,
      CMD_LIST_COLLECTIONSPACES              = 41,
      CMD_LIST_CONTEXTS                      = 42,
      CMD_LIST_CONTEXTS_CURRENT              = 43,
      CMD_LIST_SESSIONS                      = 44,
      CMD_LIST_SESSIONS_CURRENT              = 45,
      CMD_LIST_STORAGEUNITS                  = 46,
      CMD_LIST_GROUPS                        = 47,
      CMD_LIST_DOMAINS                       = 48,
      CMD_LIST_BACKUPS                       = 49,
      CMD_LIST_TASKS                         = 50,

      CMD_RENAME_COLLECTION                  = 60,

      CMD_REORG_OFFLINE                      = 70,
      CMD_REORG_ONLINE                       = 71,
      CMD_REORG_RECOVER                      = 72,

      CMD_SHUTDOWN                           = 80,

      CMD_SNAPSHOT_CONTEXTS                  = 90,
      CMD_SNAPSHOT_CONTEXTS_CURRENT          = 91,
      CMD_SNAPSHOT_DATABASE                  = 92,
      CMD_SNAPSHOT_RESET                     = 93,
      CMD_SNAPSHOT_SESSIONS                  = 94,
      CMD_SNAPSHOT_SESSIONS_CURRENT          = 95,
      CMD_SNAPSHOT_SYSTEM                    = 96,
      CMD_SNAPSHOT_COLLECTIONS               = 97,
      CMD_SNAPSHOT_COLLECTIONSPACES          = 98,
      CMD_SNAPSHOT_CATA                      = 99,

      CMD_TEST_COLLECTION                    = 110,
      CMD_TEST_COLLECTIONSPACE               = 111,

      CMD_SET_PDLEVEL                        = 120,

      CMD_CREATE_GROUP                       = 130,
      CMD_CREATE_NODE                        = 131,
      CMD_UPDATE_NODE                        = 132,
      CMD_ACTIVE_GROUP                       = 133,
      CMD_GET_CONFIG                         = 134,
      CMD_SPLIT                              = 135,
      CMD_REMOVE_GROUP                       = 136,
      CMD_REMOVE_NODE                        = 137,
      CMD_START_NODE                         = 138,
      CMD_SHUTDOWN_NODE                      = 139,
      CMD_SHUTDOWN_GROUP                     = 140,
      CMD_CREATE_CATAGROUP                   = 141,
      CMD_CREATE_DOMAIN                      = 142,
      CMD_REMOVE_DOMAIN                      = 143,
      CMD_ADD_DOMAIN_GROUP                   = 144,
      CMD_REMOVE_DOMAIN_GROUP                = 145,
      CMD_WAITTASK                           = 146,
      CMD_CANCEL_TASK                        = 147,

      CMD_LINK_COLLECTION                    = 150,
      CMD_UNLINK_COLLECTION                  = 151,

      // trace commands
      CMD_TRACE_START                        = 160,
      CMD_TRACE_RESUME                       = 161,
      CMD_TRACE_STOP                         = 162,
      CMD_TRACE_STATUS                       = 163,

      CMD_JSON_LOAD                          = 180,

      CMD_EXPORT_CONFIG                      = 200,

      CMD_REMOVE_BACKUP                      = 210,

#if defined (_DEBUG)
      // all debug commands goes into here
      CMD_DEBUG_QUERY                        = 65534,
#endif
      CMD_UNKNOW                             = 65535
   };

#define CMD_SPACE_NODE_NULL               0x0000
#define CMD_SPACE_NODE_STANDALONE         0x0001
#define CMD_SPACE_NODE_COORD              0x0002
#define CMD_SPACE_NODE_DATA               0x0004
#define CMD_SPACE_NODE_CATA               0x0008
#define CMD_SPACE_NODE_ALL \
   (CMD_SPACE_NODE_STANDALONE|CMD_SPACE_NODE_COORD|CMD_SPACE_NODE_DATA|CMD_SPACE_NODE_CATA)

#define CMD_SPACE_SERVICE_LOCAL           0x0010
#define CMD_SPACE_SERVICE_SHARD           0x0020
#define CMD_SPACE_SERVICE_CATA            0x0040
#define CMD_SPACE_SERVICE_ALL \
   (CMD_SPACE_SERVICE_LOCAL|CMD_SPACE_SERVICE_SHARD|CMD_SPACE_SERVICE_CATA)


#define DECLARE_CMD_AUTO_REGISTER() \
   public: \
      static _rtnCommand *newThis () ; \

#define IMPLEMENT_CMD_AUTO_REGISTER(theClass) \
   _rtnCommand *theClass::newThis () \
   { \
      return SDB_OSS_NEW theClass() ;\
   } \
   _rtnCmdAssit theClass##Assit ( theClass::newThis ) ; \

   class _pmdEDUCB ;
   class _SDB_DMSCB ;
   class _SDB_RTNCB ;
   class _dpsLogWrapper ;
   class _rtnCommand : public SDBObject
   {
      public:
         _rtnCommand () ;
         virtual ~_rtnCommand () ;

         void  setFromService( INT32 fromService ) ;
         INT32 getFromService() const { return _fromService ; }

         virtual INT32 spaceNode () ;
         virtual INT32 spaceService () ;

      public:
         virtual const CHAR * name () = 0 ;
         virtual RTN_COMMAND_TYPE type () = 0 ;
         virtual BOOLEAN      writable () ;
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) = 0 ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) = 0 ;

      protected:
         INT32             _fromService ;

   };

   typedef _rtnCommand* (*CDM_NEW_FUNC)() ;

   class _rtnCmdAssit : public SDBObject
   {
      public:
         _rtnCmdAssit ( CDM_NEW_FUNC pFunc ) ;
         ~_rtnCmdAssit () ;
   };

   struct _cmdBuilderInfo : public SDBObject
   {
   public :
      std::string    cmdName ;
      UINT32         nameSize ;
      CDM_NEW_FUNC   createFunc ;

      _cmdBuilderInfo *sub ;
      _cmdBuilderInfo *next ;
   } ;

   class _rtnCmdBuilder : public SDBObject
   {
      friend class _rtnCmdAssit ;

      public:
         _rtnCmdBuilder () ;
         ~_rtnCmdBuilder () ;
      public:
         _rtnCommand *create ( const CHAR *command ) ;
         void         release ( _rtnCommand *pCommand ) ;

      //protected:
         INT32 _register ( const CHAR * name, CDM_NEW_FUNC pFunc ) ;

         INT32        _insert ( _cmdBuilderInfo * pCmdInfo,
                                const CHAR * name, CDM_NEW_FUNC pFunc ) ;
         CDM_NEW_FUNC _find ( const CHAR * name ) ;

         void _releaseCmdInfo ( _cmdBuilderInfo *pCmdInfo ) ;

         UINT32 _near ( const CHAR *str1, const CHAR *str2 ) ;

      private:
         _cmdBuilderInfo                     *_pCmdInfoRoot ;

   };

   _rtnCmdBuilder * getRtnCmdBuilder () ;


   //Command list
   class _rtnCoordOnly : public _rtnCommand
   {
      protected:
         _rtnCoordOnly () {}
      public:
         ~_rtnCoordOnly () {}
         virtual INT32 spaceNode () { return CMD_SPACE_NODE_COORD ; }
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff )
         { return SDB_RTN_COORD_ONLY ; }
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL )
         { return SDB_RTN_COORD_ONLY ; }
   };

   class _rtnCreateGroup : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnCreateGroup () {}
         ~_rtnCreateGroup () {}
         virtual const CHAR * name () { return NAME_CREATE_GROUP ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_CREATE_GROUP ; }
   } ;

   class _rtnRemoveGroup : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnRemoveGroup () {}
         ~_rtnRemoveGroup () {}
         virtual const CHAR * name () { return NAME_REMOVE_GROUP ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_REMOVE_GROUP ; }
   };

   class _rtnCreateNode : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnCreateNode () {}
         ~_rtnCreateNode () {}
         virtual const CHAR * name () { return NAME_CREATE_NODE ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_CREATE_NODE ; }
   } ;

   class _rtnRemoveNode : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnRemoveNode () {}
         ~_rtnRemoveNode () {}
         virtual const CHAR * name () { return NAME_REMOVE_NODE ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_REMOVE_NODE ; }
   };

   class _rtnUpdateNode : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnUpdateNode () {}
         ~_rtnUpdateNode () {}
         virtual const CHAR * name () { return NAME_UPDATE_NODE ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_UPDATE_NODE ; }
   } ;

   class _rtnActiveGroup : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnActiveGroup () {}
         ~_rtnActiveGroup () {}
         virtual const CHAR * name () { return NAME_ACTIVE_GROUP ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_ACTIVE_GROUP ; }
   } ;

   class _rtnStartNode : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnStartNode () {}
         ~_rtnStartNode () {}
         virtual const CHAR * name () { return NAME_START_NODE ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_START_NODE ; }
   };

   class _rtnShutdownNode : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnShutdownNode () {}
         ~_rtnShutdownNode () {}
         virtual const CHAR * name () { return NAME_SHUTDOWN_NODE ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_SHUTDOWN_NODE ; }
   };

   class _rtnShutdownGroup : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnShutdownGroup () {}
         ~_rtnShutdownGroup () {}
         virtual const CHAR * name () { return NAME_SHUTDOWN_GROUP ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_SHUTDOWN_GROUP ; }
   };

   class _rtnGetConfig : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnGetConfig () {}
         ~_rtnGetConfig () {}
         virtual const CHAR * name () { return NAME_GET_CONFIG ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_GET_CONFIG ; }
   } ;

   class _rtnAlterCollection : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnAlterCollection () {}
         ~_rtnAlterCollection () {}
         virtual const CHAR * name () { return NAME_ALTER_COLLECTION ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_ALTER_COLLECTION ; }
   };

   class _rtnListGroups : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnListGroups () {}
         ~_rtnListGroups () {}
         virtual const CHAR * name () { return NAME_LIST_GROUPS ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_GROUPS ; }
   };

   class _rtnCreateCataGroup : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnCreateCataGroup () {}
         ~_rtnCreateCataGroup () {}
         virtual const CHAR * name () { return NAME_CREATE_CATAGROUP ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_CREATE_CATAGROUP ; }
   };

   class _rtnCreateDomain : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnCreateDomain () {}
         ~_rtnCreateDomain () {}
         virtual const CHAR * name () { return NAME_CREATE_DOMAIN ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_CREATE_DOMAIN ; }
   };

   class _rtnRemoveDomain : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnRemoveDomain () {}
         ~_rtnRemoveDomain () {}
         virtual const CHAR * name () { return NAME_REMOVE_DOMAIN ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_REMOVE_DOMAIN ; }
   };

   class _rtnAddDomainGroup : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnAddDomainGroup () {}
         ~_rtnAddDomainGroup () {}
         virtual const CHAR * name () { return NAME_ADD_DOMAIN_GROUP ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_ADD_DOMAIN_GROUP ; }
   };

   class _rtnRemoveDomainGroup : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnRemoveDomainGroup () {}
         ~_rtnRemoveDomainGroup () {}
         virtual const CHAR * name () { return NAME_REMOVE_DOMAIN_GROUP ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_REMOVE_DOMAIN_GROUP ; }
   };

   class _rtnListDomains : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnListDomains () {}
         ~_rtnListDomains () {}
         virtual const CHAR * name () { return NAME_LIST_DOMAINS ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_DOMAINS ; }
   };

   class _rtnSnapshotCata : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnSnapshotCata () {}
         ~_rtnSnapshotCata () {}
         virtual const CHAR * name () { return NAME_SNAPSHOT_CATA ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_SNAPSHOT_CATA ; }
   };

   class _rtnWaitTask : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnWaitTask () {}
         ~_rtnWaitTask () {}
         virtual const CHAR * name () { return NAME_WAITTASK ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_WAITTASK ; }
   } ;

   class _rtnListTask : public _rtnCoordOnly
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnListTask () {}
         ~_rtnListTask () {}
         virtual const CHAR * name () { return NAME_LIST_TASKS ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_TASKS ; }
   } ;

   class _rtnBackup : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnBackup () ;
         ~_rtnBackup () ;

         virtual BOOLEAN      writable () { return TRUE ; }
         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;
      protected:
         const CHAR        *_matherBuff ;

         const CHAR        *_backupName ;
         const CHAR        *_path ;
         const CHAR        *_desp ;
         BOOLEAN           _ensureInc ;
         BOOLEAN           _rewrite ;

   };

   class _rtnCreateCollection : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnCreateCollection () ;
         ~_rtnCreateCollection () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual BOOLEAN      writable () ;
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;

      protected:
         const CHAR                 *_collectionName ;
         BSONObj                    _shardingKey ;
         UINT32                     _attributes ;

   };

   class _rtnCreateCollectionspace : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

     public:
         _rtnCreateCollectionspace () ;
         ~_rtnCreateCollectionspace () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual BOOLEAN      writable () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;

     protected:
         const CHAR                 *_spaceName ;
         INT32                      _pageSize ;

   };

   class _rtnCreateIndex : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnCreateIndex () ;
         ~_rtnCreateIndex () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual BOOLEAN      writable () ;
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;

      protected:
         const CHAR              *_collectionName ;
         BSONObj                 _index ;

   };

   class _rtnDropCollection : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnDropCollection () ;
         ~_rtnDropCollection () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual BOOLEAN      writable () ;
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR           *_collectionName ;

   };

   class _rtnDropCollectionspace : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnDropCollectionspace () ;
         ~_rtnDropCollectionspace () ;

         const CHAR *spaceName () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual BOOLEAN      writable () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR           *_spaceName ;
   };

   class _rtnDropIndex : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnDropIndex () ;
         ~_rtnDropIndex () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual BOOLEAN      writable () ;
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR           *_collectionName ;
         BSONObj              _index ;
   };

   class _rtnGet : public _rtnCommand
   {
      protected:
         _rtnGet () ;
         ~_rtnGet () ;
      public:
         virtual const CHAR * collectionFullName () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR           *_collectionName ;
         INT64                _numToReturn ;
         INT64                _numToSkip ;
         const CHAR           *_matcherBuff ;
         const CHAR           *_selectBuff ;
         const CHAR           *_orderByBuff ;
         BSONObj              _hintObj ;
         INT32                _flags ;

   } ;

   class _rtnGetCount : public _rtnGet
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnGetCount () ;
         ~_rtnGetCount () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnGetIndexes : public _rtnGet
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnGetIndexes () ;
         ~_rtnGetIndexes () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnGetDatablocks : public _rtnGet
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnGetDatablocks () ;
         ~_rtnGetDatablocks () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnGetQueryMeta : public _rtnGet
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnGetQueryMeta () ;
         ~_rtnGetQueryMeta () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;

         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;

   } ;

   class _rtnList : public _rtnCommand
   {
      protected:
         _rtnList () ;
         ~_rtnList () ;
      protected:
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         INT64                _numToReturn ;
         INT64                _numToSkip ;
         const CHAR           *_matcherBuff ;
         const CHAR           *_selectBuff ;
         const CHAR           *_orderByBuff ;
         const CHAR           *_hintBuff ;
         INT32                _flags ;
   };

   class _rtnListCollections : public _rtnList
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnListCollections () {}
         ~_rtnListCollections () {}

         virtual const CHAR * name () { return NAME_LIST_COLLECTIONS ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_COLLECTIONS ; }
   };

   class _rtnListCollectionspaces : public _rtnList
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnListCollectionspaces () {}
         ~_rtnListCollectionspaces () {}

         virtual const CHAR * name () { return NAME_LIST_COLLECTIONSPACES ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_COLLECTIONSPACES ; }
   };

   class _rtnListContexts : public _rtnList
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnListContexts () {}
         ~_rtnListContexts () {}

         virtual const CHAR * name () { return NAME_LIST_CONTEXTS ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_CONTEXTS ; }
   };

   class _rtnListContextsCurrent : public _rtnList
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnListContextsCurrent () {}
         ~_rtnListContextsCurrent () {}

         virtual const CHAR * name () { return NAME_LIST_CONTEXTS_CURRENT ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_CONTEXTS_CURRENT ; }
   };

   class _rtnListSessions : public _rtnList
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnListSessions () {}
         ~_rtnListSessions () {}

         virtual const CHAR * name () { return NAME_LIST_SESSIONS ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_SESSIONS ; }
   };

   class _rtnListSessionsCurrent : public _rtnList
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnListSessionsCurrent () {}
         ~_rtnListSessionsCurrent () {}

         virtual const CHAR * name () { return NAME_LIST_SESSIONS_CURRENT ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_SESSIONS_CURRENT ; }
   };

   class _rtnListStorageUnits : public _rtnList
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnListStorageUnits () {}
         ~_rtnListStorageUnits () {}

         virtual const CHAR * name () { return NAME_LIST_STORAGEUNITS ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_STORAGEUNITS ; }
   } ;

   class _rtnListBackups : public _rtnList
   {
      DECLARE_CMD_AUTO_REGISTER () ;

      public:
         _rtnListBackups () {} ;
         ~_rtnListBackups () {} ;

         virtual const CHAR * name () { return NAME_LIST_BACKUPS ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_LIST_BACKUPS ; }
   } ;

   class _rtnRenameCollection : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnRenameCollection () ;
         ~_rtnRenameCollection () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual BOOLEAN      writable () ;
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR           *_oldCollectionName ;
         const CHAR           *_newCollectionName ;
         const CHAR           *_csName ;
         std::string          fullCollectionName ;
   };

   class _rtnReorg : public _rtnCommand
   {
      protected:
         _rtnReorg () ;
         ~_rtnReorg () ;
         virtual INT32 spaceNode () ;
      public:
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR           *_collectionName ;
         const CHAR           *_hintBuffer ;

   };

   class _rtnReorgOffline : public _rtnReorg
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnReorgOffline () ;
         ~_rtnReorgOffline () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnReorgOnline : public _rtnReorg
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnReorgOnline () ;
         ~_rtnReorgOnline () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnReorgRecover : public _rtnReorg
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnReorgRecover () ;
         ~_rtnReorgRecover () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnShutdown : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnShutdown () ;
         ~_rtnShutdown () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
   };

   class _rtnSnapshot : public _rtnCommand
   {
      protected:
         _rtnSnapshot () ;
         ~_rtnSnapshot () ;
      public:
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         INT64                _numToReturn ;
         INT64                _numToSkip ;
         const CHAR           *_matcherBuff ;
         const CHAR           *_selectBuff ;
         const CHAR           *_orderByBuff ;
         INT32                _flags ;
   };

   class _rtnSnapshotSystem : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public :
         _rtnSnapshotSystem () ;
         ~_rtnSnapshotSystem () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSnapshotContexts : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSnapshotContexts () ;
         ~_rtnSnapshotContexts () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSnapshotContextsCurrent : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSnapshotContextsCurrent () ;
         ~_rtnSnapshotContextsCurrent () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSnapshotDatabase : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSnapshotDatabase () ;
         ~_rtnSnapshotDatabase () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSnapshotCollections : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSnapshotCollections () ;
         ~_rtnSnapshotCollections () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSnapshotCollectionSpaces : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSnapshotCollectionSpaces () ;
         ~_rtnSnapshotCollectionSpaces () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSnapshotReset : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSnapshotReset () ;
         ~_rtnSnapshotReset () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSnapshotSessions : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSnapshotSessions () ;
         ~_rtnSnapshotSessions () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSnapshotSessionsCurrent : public _rtnSnapshot
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSnapshotSessionsCurrent () ;
         ~_rtnSnapshotSessionsCurrent () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnTest : public _rtnCommand
   {
      protected:
         _rtnTest () ;
         ~_rtnTest () ;
      public:
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR        *_collectionName ;
   };

   class _rtnTestCollection : public _rtnTest
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnTestCollection () ;
         ~_rtnTestCollection () ;

         virtual const CHAR * collectionFullName () ;
         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnTestCollectionspace : public _rtnTest
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnTestCollectionspace () ;
         ~_rtnTestCollectionspace () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
   };

   class _rtnSetPDLevel : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSetPDLevel () ;
         ~_rtnSetPDLevel () ;

      public:
         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;
      protected:
         INT32             _pdLevel ;
   } ;

   class _rtnSplit : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnSplit () ;
         ~_rtnSplit () ;

         virtual INT32 spaceService () ;

      public :
         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual BOOLEAN      writable () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;
      protected:
         CHAR _szCollection [ DMS_COLLECTION_SPACE_NAME_SZ +
                              DMS_COLLECTION_NAME_SZ + 2 ] ;
         CHAR _szTargetName [ OP_MAXNAMELENGTH + 1 ] ;
         CHAR _szSourceName [ OP_MAXNAMELENGTH + 1 ] ;
         FLOAT64 _percent  ;

         bson::BSONObj _splitKey ;
   } ;

   class _rtnCancelTask : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()
      public:
         _rtnCancelTask () { _taskID = 0 ; }
         ~_rtnCancelTask () {}
         virtual INT32 spaceNode () ;
         virtual BOOLEAN      writable () { return TRUE ; }
         virtual const CHAR * name () { return NAME_CANCEL_TASK ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_CANCEL_TASK ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;

      protected:
         UINT64            _taskID ;

   } ;

   class _rtnTraceStart : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnTraceStart () ;
         ~_rtnTraceStart () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;
      protected :
         UINT32 _mask ;
         std::vector<UINT32> _tid ;
         std::vector<UINT64> _funcCode ;
         UINT32 _size ;
   };

   class _rtnTraceResume : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnTraceResume () ;
         ~_rtnTraceResume () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;
   };

   class _rtnTraceStop : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnTraceStop () ;
         ~_rtnTraceStop () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;
      protected :
         const CHAR *_pDumpFileName ;
   };

   class _rtnTraceStatus : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnTraceStatus () ;
         ~_rtnTraceStatus () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;
      protected:
         INT64                _numToReturn ;
         INT64                _numToSkip ;
         const CHAR           *_matcherBuff ;
         const CHAR           *_selectBuff ;
         const CHAR           *_orderByBuff ;
         INT32                _flags ;
   };

   class _rtnLoad : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnLoad () ;
         ~_rtnLoad () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;
      protected :
         setParameters _parameters ;
         CHAR     _fileName[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR     _csName[ DMS_COLLECTION_SPACE_NAME_SZ + 1 ]   ;
         CHAR     _clName[ DMS_COLLECTION_NAME_SZ + 1 ]   ;
   };

   class _rtnExportConf : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public :
         _rtnExportConf(){}
         virtual ~_rtnExportConf(){}

         virtual const CHAR * name () { return NAME_EXPORT_CONFIGURATION ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_EXPORT_CONFIG ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn,
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff )
         {
            return SDB_OK ;
         }

         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;

   } ;

   class _rtnRemoveBackup : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER ()

      public:
         _rtnRemoveBackup () ;
         ~_rtnRemoveBackup () {}

      public:
         virtual const CHAR * name () { return NAME_REMOVE_BACKUP ; }
         virtual RTN_COMMAND_TYPE type () { return CMD_REMOVE_BACKUP ; }

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL ) ;

      protected:
         const CHAR              *_path ;
         const CHAR              *_backupName ;
         const CHAR              *_matcherBuff ;

   } ;

   class _rtnLinkCollection : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnLinkCollection () ;
         ~_rtnLinkCollection () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR           *_collectionName ;
   };

   class _rtnUnlinkCollection : public _rtnCommand
   {
      DECLARE_CMD_AUTO_REGISTER()

      public:
         _rtnUnlinkCollection () ;
         ~_rtnUnlinkCollection () ;

         virtual const CHAR * name () ;
         virtual RTN_COMMAND_TYPE type () ;
         virtual const CHAR * collectionFullName () ;

         virtual INT32 init ( INT32 flags, INT64 numToSkip, INT64 numToReturn, 
                              const CHAR *pMatcherBuff,
                              const CHAR *pSelectBuff,
                              const CHAR *pOrderByBuff,
                              const CHAR *pHintBuff ) ;
         virtual INT32 doit ( _pmdEDUCB *cb, _SDB_DMSCB *dmsCB,
                              _SDB_RTNCB *rtnCB, _dpsLogWrapper *dpsCB,
                              INT16 w = 1, INT64 *pContextID = NULL  ) ;
      protected:
         const CHAR           *_collectionName ;
   };

}

const UINT32 pdGetTraceFunctionListNum();

#endif //RTN_COMMAND_HPP_

