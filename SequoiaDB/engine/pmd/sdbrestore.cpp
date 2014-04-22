/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sdbrestore.cpp

   Descriptive Name = Process MoDel Main

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for SequoiaDB,
   and all other process-initialization code.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/07/2013  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmd.hpp"
#include "pmdSignalHandler.hpp"
#include "msgMessage.hpp"
#include "ossStackDump.hpp"
#include "ossEDU.hpp"
#include "pmdCommon.hpp"
#include "rtn.hpp"
#include "pmdCB.hpp"
#include "barRestoreJob.hpp"
#include "ossVer.h"
#include "pmdStartup.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;
using namespace bson;
namespace po = boost::program_options ;
namespace fs = boost::filesystem ;

namespace engine
{

   extern boost::thread_specific_ptr<oss_edu_data> _ossEduData ;

   /*
      restore logger define
   */
   barRSOfflineLogger   g_restoreLogger ;

   /*
      configure define
   */
   #define RS_BK_PATH            "bkpath"
   #define RS_BK_NAME            "bkname"
   #define RS_INC_ID             "increaseid"
   #define RS_BK_ACTION          "action"
   #define RS_BK_RESTORE         "restore"
   #define RS_BK_LIST            "list"
   #define RS_BK_IS_SELF         "isSelf"

   #define PMD_RS_OPTIONS  \
      ( PMD_COMMANDS_STRING (PMD_OPTION_HELP, ",h"), "help" ) \
      ( PMD_OPTION_VERSION, "show version" ) \
      ( PMD_COMMANDS_STRING (RS_BK_PATH, ",p"), boost::program_options::value<string>(), "backup path" ) \
      ( PMD_COMMANDS_STRING (RS_INC_ID, ",i"), boost::program_options::value<int>(), "increase id, default is -1" ) \
      ( PMD_COMMANDS_STRING (RS_BK_NAME, ",n"), boost::program_options::value<string>(), "backup name" ) \
      ( PMD_COMMANDS_STRING (RS_BK_ACTION, ",a"), boost::program_options::value<string>(), "action(restore/list), defalut is restore" ) \
      ( RS_BK_IS_SELF, boost::program_options::value<string>(), "wether restore self node(true/false),default is true" ) \
      ( PMD_OPTION_DBPATH, boost::program_options::value<string>(), "database path" ) \
      ( PMD_OPTION_IDXPATH, boost::program_options::value<string>(), "index path" ) \
      ( PMD_OPTION_LOGPATH, boost::program_options::value<string>(), "log file path" ) \
      ( PMD_OPTION_CONFPATH, boost::program_options::value<string>(), "configure file path" ) \
      ( PMD_OPTION_DIAGLOGPATH, boost::program_options::value<string>(), "diagnostic log file path" ) \
      ( PMD_OPTION_BKUPPATH, boost::program_options::value<string>(), "backup path" ) \
      ( PMD_OPTION_SVCNAME, boost::program_options::value<string>(), "local service name or port" ) \
      ( PMD_OPTION_REPLNAME, boost::program_options::value<string>(), "replication service name or port" ) \
      ( PMD_OPTION_SHARDNAME, boost::program_options::value<string>(), "sharding service name or port" ) \
      ( PMD_OPTION_CATANAME, boost::program_options::value<string>(), "catalog service name or port" ) \
      ( PMD_OPTION_RESTNAME, boost::program_options::value<string>(), "REST service name or port" ) \

   #define RS_BK_ACTION_NAME_LEN          (20)


   BSONObj rsMakeNoneSelfCfg( const BSONObj &obj )
   {
      BSONObjBuilder builder ;
      BSONObjIterator it( obj ) ;
      while ( it.more() )
      {
         BSONElement ele = it.next () ;

         if ( 0 == ossStrcmp( ele.fieldName(), PMD_OPTION_DBPATH ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_IDXPATH ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_LOGPATH ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_CONFPATH ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_DIAGLOGPATH ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_BKUPPATH ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_SVCNAME ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_REPLNAME ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_SHARDNAME ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_CATANAME ) ||
              0 == ossStrcmp( ele.fieldName(), PMD_OPTION_RESTNAME ) )
         {
            continue ;
         }
         builder.append( ele ) ;
      }
      return builder.obj() ;
   }

   /*
      Tool functions :
   */
   INT32 sdbCleanDirFiles( const CHAR *pPath )
   {
      INT32 rc = SDB_OK ;

      fs::path dbDir ( pPath ) ;
      fs::directory_iterator end_iter ;

      if ( fs::exists ( dbDir ) && fs::is_directory ( dbDir ) )
      {
         for ( fs::directory_iterator dir_iter ( dbDir );
               dir_iter != end_iter; ++dir_iter )
         {
            if ( fs::is_regular_file ( dir_iter->status() ) )
            {
               const std::string fileName = dir_iter->path().string() ;
               rc = ossDelete( fileName.c_str() ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to remove %s, rc: %d",
                            fileName.c_str(), rc ) ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 sdbCleanDirSUFiles( const CHAR *pPath )
   {
      INT32 rc = SDB_OK ;
      CHAR csName [ DMS_COLLECTION_SPACE_NAME_SZ + 1 ] = {0} ;
      UINT32 sequence = 0 ;

      fs::path dbDir ( pPath ) ;
      fs::directory_iterator end_iter ;

      if ( fs::exists ( dbDir ) && fs::is_directory ( dbDir ) )
      {
         for ( fs::directory_iterator dir_iter ( dbDir );
               dir_iter != end_iter; ++dir_iter )
         {
            if ( fs::is_regular_file ( dir_iter->status() ) )
            {
               const std::string fileName =
                  dir_iter->path().filename().string() ;
               if ( rtnVerifyCollectionSpaceFileName( fileName.c_str(), csName,
                    DMS_COLLECTION_SPACE_NAME_SZ, sequence,
                    DMS_DATA_SU_EXT_NAME ) ||
                    rtnVerifyCollectionSpaceFileName( fileName.c_str(), csName,
                    DMS_COLLECTION_SPACE_NAME_SZ, sequence,
                    DMS_INDEX_SU_EXT_NAME ) )
               {
                  const std::string pathName = dir_iter->path().string() ;
                  rc = ossDelete( pathName.c_str() ) ;
                  PD_RC_CHECK( rc, PDERROR, "Failed to remove %s, rc: %d",
                               pathName.c_str(), rc ) ;
               }
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /*
      _rsOptionMgr define and implement
   */
   class _rsOptionMgr : public _pmdCfgRecord
   {
      public:
         _rsOptionMgr ()
         {
            ossMemset( _bkPath, 0, sizeof( _bkPath ) ) ;
            ossMemset( _bkName, 0, sizeof( _bkName ) ) ;
            ossMemset( _action, 0, sizeof( _action ) ) ;
            ossMemset( _dialogPath, 0, sizeof( _dialogPath ) ) ;
            ossMemset( _dbPath, 0, sizeof( _dbPath ) ) ;
            ossMemset( _cfgPath, 0, sizeof( _cfgPath ) ) ;
            ossMemset( _svcName, 0, sizeof( _svcName ) ) ;
            _incID = -1 ;
            _isSelf = TRUE ;

            ossStrcpy( _dialogPath, "dialog" ) ;
         }

      protected:
         virtual INT32 doDataExchange( pmdCfgExchange *pEX )
         {
            resetResult() ;

            rdxString( pEX, RS_BK_PATH, _bkPath, sizeof( _bkPath ), FALSE,
                       FALSE, PMD_CURRENT_PATH ) ;
            rdxString( pEX, RS_BK_NAME, _bkName, sizeof( _bkName ), FALSE,
                       FALSE, "" ) ;
            rdxString( pEX, RS_BK_ACTION, _action, sizeof( _action ), FALSE,
                       FALSE, RS_BK_RESTORE ) ;
            rdxString( pEX, PMD_OPTION_DBPATH, _dbPath, sizeof( _dbPath ),
                       FALSE, FALSE, "" ) ;
            rdxString( pEX, PMD_OPTION_CONFPATH, _cfgPath, sizeof( _cfgPath ),
                       FALSE, FALSE, "" ) ;
            rdxString( pEX, PMD_OPTION_SVCNAME, _svcName, sizeof( _svcName ),
                       FALSE, FALSE, "" ) ;
            rdxBooleanS( pEX, RS_BK_IS_SELF, _isSelf, FALSE, FALSE, TRUE ) ;
            rdxInt( pEX, RS_INC_ID, _incID, FALSE, FALSE, -1 ) ;

            return getResult() ;
         }
         virtual INT32 postLoaded()
         {
            if ( 0 != ossStrcmp( _action, RS_BK_RESTORE ) &&
                 0 != ossStrcmp( _action, RS_BK_LIST ) )
            {
               std::cerr << "action[ " << _action << " ] not invalid"
                         << std::endl ;
               return SDB_INVALIDARG ;
            }
            if ( 0 == ossStrcmp( _action, RS_BK_RESTORE ) &&
                 0 == ossStrlen( _bkName ) )
            {
               std::cerr << "In restore action, bkname can't be empty"
                         << std::endl ;
               return SDB_INVALIDARG ;
            }

            if ( !_isSelf && ( 0 == _dbPath[0] || 0 == _cfgPath[0] ||
                 0 == _svcName[0] ) )
            {
               std::cerr << "Restore not self node, must config "
                         << PMD_OPTION_DBPATH << ", " << PMD_OPTION_CONFPATH
                         << ", " << PMD_OPTION_SVCNAME << std::endl ;
               return SDB_INVALIDARG ;
            }

            // make dir
            ossMkdir( _dialogPath, OSS_CREATE|OSS_READWRITE ) ;
            // cur dialog
            ossMemset ( _pdDiagLogPath, 0, sizeof(_pdDiagLogPath) ) ;
            pmdBuildFullPath( _dialogPath, PMD_DFT_DIAGLOG,
                              OSS_MAX_PATHSIZE, _pdDiagLogPath ) ;

            return SDB_OK ;
         }

      public:
         CHAR              _bkPath[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR              _bkName[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR              _action[ RS_BK_ACTION_NAME_LEN + 1 ] ;
         CHAR              _dialogPath[ OSS_MAX_PATHSIZE + 1 ] ;
         INT32             _incID ;

         BOOLEAN           _isSelf ;
         CHAR              _dbPath[ OSS_MAX_PATHSIZE + 1 ] ;
         CHAR              _svcName[ OSS_MAX_SERVICENAME + 1 ] ;
         CHAR              _cfgPath[ OSS_MAX_PATHSIZE + 1 ] ;

         po::variables_map _vm ;
   } ;
   typedef _rsOptionMgr rsOptionMgr ;

   INT32 resolveArguments( INT32 argc, CHAR** argv, rsOptionMgr &rsOptMgr )
   {
      INT32 rc = SDB_OK ;

      po::variables_map vm ;
      po::options_description desc( "Command options" ) ;

      PMD_ADD_PARAM_OPTIONS_BEGIN( desc )
         PMD_RS_OPTIONS
      PMD_ADD_PARAM_OPTIONS_END

      rc = pmdGetKRCB()->getOptionCB()->readCmd( argc, argv, desc, vm ) ;
      if ( rc )
      {
         std::cerr << "read command line failed: " << rc << std::endl ;
         goto error ;
      }

      rsOptMgr._vm = vm ;
      /// read cmd first
      if ( vm.count( PMD_OPTION_HELP ) )
      {
         std::cout << desc << std::endl ;
         rc = SDB_PMD_HELP_ONLY ;
         goto done ;
      }
      if ( vm.count( PMD_OPTION_VERSION ) )
      {
         INT32 version, subVersion, release ;
         const CHAR *pBuild = NULL ;
         ossGetVersion ( &version, &subVersion, &release, &pBuild ) ;
         std::cout << "version: " << version << "."
         << subVersion << std::endl ;
         std::cout << "Release: " << release << std::endl ;
         std::cout << "Build: " << pBuild <<std::endl ;
         rc = SDB_PMD_VERSION_ONLY ;
         goto done ;
      }

      rc = rsOptMgr.init( NULL, &vm ) ;
      if ( rc )
      {
         std::cerr << "Init restore optionMgr failed: " << rc << std::endl ;
         goto error ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

   INT32 restoreSysInit ()
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      //SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
      SDB_DPSCB *dpsCB = krcb->getDPSCB() ;

      if ( _ossEduData.get()==0 )
      {
         _ossEduData.reset(SDB_OSS_NEW oss_edu_data());
      }

      std::cout << "Begin to clean dps logs..." << std::endl ;
      // clean dps logs
      rc = sdbCleanDirFiles( krcb->getLogPath() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to clean dps logs[%s], rc: %d",
                   krcb->getLogPath(), rc ) ;
      std::cout << "Begin to clean dms storages..." << std::endl ;
      // clean dms storages
      rc = sdbCleanDirSUFiles( krcb->getDBPath() ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to clean data[%s] su, rc: %d",
                   krcb->getDBPath(), rc ) ;
      if ( 0 != ossStrcmp( krcb->getDBPath(), krcb->getIndexPath() ) )
      {
         rc = sdbCleanDirSUFiles( krcb->getIndexPath() ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to clean index[%s], rc: %d",
                      krcb->getIndexPath(), rc ) ;
      }
      // remove start file
      {
         CHAR startFile[ OSS_MAX_PATHSIZE + 1 ] = {0} ;
         pmdBuildFullPath( krcb->getDBPath(), PMD_STARTUP_FILE_NAME,
                           OSS_MAX_PATHSIZE, startFile ) ;
         if ( SDB_OK == ossAccess( startFile ) )
         {
            rc = ossDelete( startFile ) ;
            PD_RC_CHECK( rc, PDERROR, "Failed to remove start file[%s], rc: %d",
                         startFile, rc ) ;
         }
      }

      // load all collectionspaces
      /*rc = rtnLoadCollectionSpaces ( krcb->getDBPath(),
                                     krcb->getIndexPath(), dmsCB ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to load collection spaces" ) ;
         goto error ;
      }

      // initialize temp space
      rc = dmsCB->getTempCB()->init () ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to initialize temp cb, rc:%d", rc ) ;
         goto error ;
      }*/
      std::cout << "Begin to init dps logs..." << std::endl ;
      // init dps
      rc = dpsCB->init( krcb->getLogPath(), krcb->getLogBufSize() ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to initialize dps cb, rc = %d", rc ) ;
         goto error ;
      }

   done :
      return rc ;
   error :
      goto done ;
   }

#if defined (_LINUX)

   // this structure should not be inherited from SDBObject because it will be
   // assigned by array
   struct _signalInfo
   {
      const CHAR *name ;
      INT32       handle ;
   } ;

   static _signalInfo signalHandleMap [] = {
      { "Unknow", 0 },
      { "SIGHUP", 1 },     //1
      { "SIGINT", 1 },     //2
      { "SIGQUIT", 1 },    //3
      { "SIGILL", 1 },     //4
      { "SIGTRAP", 1 },    //5
      { "SIGABRT", 1 },    //6
      { "SIGBUS", 1 },     //7
      { "SIGFPE", 1 },     //8
      { "SIGKILL", 1 },    //9
      { "SIGUSR1", 0 },    //10
      { "SIGSEGV", 1 },    //11
      { "SIGUSR2", 0 },    //12
      { "SIGPIPE", 0 },    //13
      { "SIGALRM", 0 },    //14
      { "SIGTERM", 1 },    //15
      { "SIGSTKFLT", 0 },  //16
      { "SIGCHLD", 0 },    //17
      { "SIGCONT", 0 },    //18
      { "SIGSTOP", 1 },    //19
      { "SIGTSTP", 0 },    //20
      { "SIGTTIN", 0 },    //21
      { "SIGTTOU", 0 },    //22
      { "SIGURG", 0 },     //23
      { "SIGXCPU", 0 },    //24
      { "SIGXFSZ", 0 },    //25
      { "SIGVTALRM", 0 },  //26
      { "SIGPROF", 0 },    //27
      { "SIGWINCH", 0 },   //28
      { "SIGIO", 0 },      //29
      { "SIGPWR", 1 },     //30
      { "SIGSYS", 1 },     //31
      { "UNKNOW", 0 },     //32
      { "UNKNOW", 0 },     //33
      { "SIGRTMIN", 0 },   //34
      { "SIGRTMIN+1", 0 }, //35
      { "SIGRTMIN+2", 0 }, //36
      { "SIGRTMIN+3", 0 }, //37
      { "SIGRTMIN+4", 0 }, //38
      { "SIGTTMIN+5", 0 }, //39
      { "SIGRTMIN+6", 0 }, //40
      { "SIGRTMIN+7", 0 }, //41
      { "SIGTTMIN+8", 0 }, //42
      { "SIGRTMIN+9", 0 }, //43
      { "SIGRTMIN+10", 0 },//44
      { "SIGRTMIN+11", 0 },//45
      { "SIGRTMIN+12", 0 },//46
      { "SIGRTMIN+13", 0 },//47
      { "SIGRTMIN+14", 0 },//48
      { "SIGRTMIN+15", 0 },//49
      { "SIGRTMAX-14", 0 },//50
      { "SIGRTMAX-13", 0 },//51
      { "SIGRTMAX-12", 0 },//52
      { "SIGRTMAX-11", 0 },//53
      { "SIGRTMAX-10", 0 },//54
      { "SIGRTMAX-9", 0 }, //55
      { "SIGRTMAX-8", 0 }, //56
      { "SIGRTMAX-7", 0 }, //57
      { "SIGRTMAX-6", 0 }, //58
      { "SIGRTMAX-5", 0 }, //59
      { "SIGRTMAX-4", 0 }, //60
      { "SIGRTMAX-3", 0 }, //61
      { "SIGRTMAX-2", 0 }, //62
      { "SIGRTMAX-1", 0 }, //63
      { "SIGRTMAX", 0 },   //64
   };

   void pmdSignalHandler ( INT32 sigNum )
   {
      if ( sigNum > 0 && sigNum <= OSS_MAX_SIGAL )
      {
         if ( signalHandleMap[sigNum].handle ) // quit
         {
            PMD_SHUTDOWN_DB( SDB_INTERRUPT ) ;
         }
      }
   }
#endif

   INT32 pmdSetupSignalHandler ()
   {
      INT32 rc = SDB_OK ;
#if defined (_LINUX)
      ossSigSet sigSet ;
      struct sigaction newact ;
      ossMemset ( &newact, 0, sizeof(newact)) ;
      sigemptyset ( &newact.sa_mask ) ;
#endif
      pmdKRCB *krcb = pmdGetKRCB () ;
      ossSetInEngine () ;
      ossSetTrapExceptionPath ( krcb->getDiagLogPath() ) ;
#if defined (_LINUX)
      // Sigsegv
      // newact.sa_sigaction = ( OSS_SIGFUNCPTR ) ossSignalSigsegv ;
      newact.sa_sigaction = ( OSS_SIGFUNCPTR ) ossEDUCodeTrapHandler ;
      newact.sa_flags |= SA_SIGINFO ;
      newact.sa_flags |= SA_ONSTACK ;
      if ( sigaction ( SIGSEGV, &newact, NULL ) )
      {
         std::cerr << "Failed to setup signal handler for SigSegV"
                   << std::endl ;
         rc = SDB_SYS ;
         goto error ;
      }
      if ( sigaction ( SIGBUS, &newact, NULL ) )
      {
         std::cerr << "Failed to setup signal handler for SigBus"
                   << std::endl ;
         rc = SDB_SYS ;
         goto error ;
      }
      // stack dump signals
      newact.sa_sigaction = ( OSS_SIGFUNCPTR ) pmdEDUCodeTrapHandler ;
      newact.sa_flags |= SA_SIGINFO ;
      newact.sa_flags |= SA_ONSTACK ;
      // capture the user stack dump signal
      // 23 for linux
      if ( sigaction ( OSS_STACK_DUMP_SIGNAL, &newact, NULL ) )
      {
         std::cerr << "Failed to setup signal handler for dump signal"
                   << std::endl ;
         rc = SDB_SYS ;
         goto error ;
      }
      // capture the internal user stack dump signal
      if ( sigaction ( OSS_STACK_DUMP_SIGNAL_INTERNAL, &newact, NULL ) )
      {
         std::cerr << "Failed to setup signal handler for dump signal"
                   << std::endl ;
         rc = SDB_SYS ;
         goto error ;
      }

      sigSet.fillSet () ;
      sigSet.sigDel ( SIGSEGV ) ;
      sigSet.sigDel ( SIGBUS ) ;
      sigSet.sigDel ( SIGALRM ) ;
      sigSet.sigDel ( SIGPROF ) ;
      sigSet.sigDel ( OSS_STACK_DUMP_SIGNAL ) ;
      sigSet.sigDel ( OSS_STACK_DUMP_SIGNAL_INTERNAL ) ;
      rc = ossRegisterSignalHandle( sigSet, (SIG_HANDLE)pmdSignalHandler ) ;
      if ( SDB_OK != rc )
      {
         std::cerr << "Failed to register signals, rc: " << rc << std::endl ;
         // we do not abort startup process if any signal handler can't be
         // installed
         rc = SDB_OK ;
      }
   done :
#endif
      return rc ;
#if defined (_LINUX)
   error :
      goto done ;
#endif
   }

   INT32 listBackups ( rsOptionMgr &optMgr )
   {
      barBackupMgr bkMgr ;
      INT32 rc = bkMgr.init( optMgr._bkPath, optMgr._bkName, NULL ) ;
      if ( rc )
      {
         std::cerr << "Init backup manager failed: " << rc << std::endl ;
         return rc ;
      }
      vector < BSONObj > backups ;
      rc = bkMgr.list( backups, TRUE ) ;
      if ( rc )
      {
         std::cerr << "List backups failed: " << rc << std::endl ;
         return rc ;
      }
      std::cout << "backup list: " << std::endl ;
      // list all backups
      vector < BSONObj >::iterator it = backups.begin() ;
      while ( it != backups.end() )
      {
         std::cout << "    " << (*it).toString().c_str() << std::endl ;
         ++it ;
      }
      std::cout << "total: " << backups.size() << std::endl ;

      return SDB_OK ;
   }

   INT32 pmdRestoreThreadMain ( INT32 argc, CHAR** argv )
   {
      INT32      rc       = SDB_OK ;
      pmdKRCB   *krcb     = pmdGetKRCB () ;
      pmdEDUMgr *eduMgr   = krcb->getEDUMgr () ;
      EDUID      agentEDU = PMD_INVALID_EDUID ;
      rsOptionMgr optMgr ;

      rc = krcb->init() ;
      if ( rc )
      {
         std::cerr << "init krcb failed, " << rc << std::endl ;
         return rc ;
      }

      rc = resolveArguments ( argc, argv, optMgr ) ;
      if ( SDB_PMD_HELP_ONLY == rc || SDB_PMD_VERSION_ONLY == rc )
      {
         PMD_SHUTDOWN_DB( SDB_OK ) ;
         rc = SDB_OK ;
         return rc ;
      }
      else if ( rc )
      {
         return rc ;
      }

      // handlers and init global mem
      rc = pmdSetupSignalHandler () ;
      if ( rc )
      {
         std::cerr << "Failed to setup signal handler, rc: " << rc << std::endl ;
         return rc ;
      }

      // only for list
      if ( 0 == ossStrcmp( optMgr._action, RS_BK_LIST ) )
      {
         rc = listBackups( optMgr ) ;
         return rc ;
      }

      PD_LOG ( PDEVENT, "Master thread starts..." ) ;

      rc = g_restoreLogger.init( optMgr._bkPath, optMgr._bkName, NULL,
                                 optMgr._incID ) ;
      if ( rc )
      {
         std::cerr << "Init restore failed: " << rc << std::endl ;
         goto error ;
      }

      if ( optMgr._isSelf )
      {
         // restore configs
         rc = krcb->getOptionCB()->restore ( g_restoreLogger.getConf(),
                                             &(optMgr._vm) ) ;
      }
      else
      {
         BSONObj newCfgObj = rsMakeNoneSelfCfg( g_restoreLogger.getConf() ) ;
         rc = krcb->getOptionCB()->restore ( newCfgObj, &(optMgr._vm) ) ;
      }
      if ( rc )
      {
         std::cerr << "Init option cb failed: " << rc << std::endl ;
         goto error ;
      }

      // initialize variables
      rc = restoreSysInit () ;
      PD_RC_CHECK ( rc, PDERROR, "Failed to initialize, rc: %d", rc ) ;

      // Then start log global writer thread
      eduMgr->startEDU ( EDU_TYPE_LOGGW, NULL, &agentEDU ) ;
      eduMgr->regSystemEDU ( EDU_TYPE_LOGGW, agentEDU ) ;
      // dps rollback
      eduMgr->startEDU ( EDU_TYPE_DPSROLLBACK_TASK, NULL, &agentEDU );
      eduMgr->regSystemEDU ( EDU_TYPE_DPSROLLBACK_TASK, agentEDU );

      std::cout << "Begin to restore... " << std::endl ;
      // start restore task
      rc = startRestoreJob( &agentEDU, &g_restoreLogger ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to start restore task, rc: %d", rc ) ;
         std::cerr << "Start restore task failed: " << rc << std::endl ;
         goto error ;
      }

      // Now master thread get into big loop and check shutdown flag
      while ( PMD_IS_DB_UP )
      {
         ossSleepsecs ( 1 ) ;
      }
      rc = krcb->getExitCode() ;

   done :
      PMD_SHUTDOWN_DB( rc ) ;
      krcb->destroy () ;
      PD_LOG ( PDEVENT, "Master thread exits, exist code: %d",
               krcb->getExitCode() ) ;

      std::cerr << "*****************************************************"
                << std::endl ;
      if ( SDB_OK != krcb->getExitCode() )
      {
         std::cerr << "Restore failed: " << krcb->getExitCode() << std::endl ;
      }
      else
      {
         std::cout << "Restore succeed!" << std::endl ;
      }
      std::cerr << "*****************************************************"
                << std::endl ;
      return rc ;
   error :
      goto done ;
   }

#if defined (_WINDOWS)
   BOOL pmdCtrlHandler( DWORD fdwCtrlType )
   {
      BOOLEAN ret = FALSE ;
      switch( fdwCtrlType )
      {
      // Handle the CTRL-C signal.
      case CTRL_C_EVENT:
         PMD_SHUTDOWN_DB( SDB_INTERRUPT ) ;
         printf( "Ctrl-C event\n\n" );
         Beep( 750, 300 );
         ret = TRUE ;
         goto done ;

      // CTRL-CLOSE: confirm that the user wants to exit.
      case CTRL_CLOSE_EVENT:
         Beep( 600, 200 );
         printf( "Ctrl-Close event\n\n" );
         ret = TRUE ;
         goto done ;

      // Pass other signals to the next handler.
      case CTRL_BREAK_EVENT:
         Beep( 900, 200 );
         printf( "Ctrl-Break event\n\n" );
         ret = FALSE ;
         goto done ;

      case CTRL_LOGOFF_EVENT:
         Beep( 1000, 200 );
         printf( "Ctrl-Logoff event\n\n" );
         ret = FALSE ;
         goto done ;

      case CTRL_SHUTDOWN_EVENT:
         Beep( 750, 500 );
         printf( "Ctrl-Shutdown event\n\n" );
         ret = FALSE ;
         goto done ;

      default:
         ret = FALSE ;
         goto done ;
      }
   done :
      return ret ;
   }
#endif

   void pmdOnetimeInit()
   {
   #if defined (_WINDOWS)
      SetConsoleCtrlHandler( (PHANDLER_ROUTINE) pmdCtrlHandler, TRUE ) ;
   #endif
   }
}

/**************************************/
/*   SDB RESTORE MAIN FUNCTION        */
/**************************************/
INT32 main ( INT32 argc, CHAR** argv )
{
   INT32 rc = SDB_OK ;
   engine::pmdOnetimeInit() ;
   rc = engine::pmdRestoreThreadMain ( argc, argv ) ;
   return rc ;
}

