/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdEDU.cpp

   Descriptive Name = Process MoDel Agent Engine Dispatchable Unit

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for EDU processing.
   EDU thread is a wrapper of all threads. It will call each entry function
   depends on the EDU type.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include <stdio.h>
#include <boost/thread/thread.hpp>
#include "pd.hpp"
#include "ossEDU.hpp"
#include "ossMem.hpp"
#include "pmdEDU.hpp"
#include "pmdEDUMgr.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include <map>
#include <string>
#include <../snappy/snappy.h>
#include <dmsRecord.hpp>

namespace engine
{
   extern boost::thread_specific_ptr<oss_edu_data> _ossEduData ;

   static std::map<EDU_TYPES, std::string> mapEDUName ;
   static std::map<EDU_TYPES,EDU_TYPES>    mapEDUTypeSys ;

   // PD_TRACE_DECLARE_FUNCTION ( SDB_REGEDUNAME, "registerEDUName" )
   INT32 registerEDUName ( EDU_TYPES type, const CHAR * name, BOOLEAN system )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_REGEDUNAME );
      std::map<EDU_TYPES, std::string>::iterator it =
         mapEDUName.find ( type ) ;
      if ( it != mapEDUName.end() )
      {
         PD_LOG ( PDERROR, "EDU type confict[type:%d, %s<->%s]", (INT32)type,
            it->second.c_str(), name ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      mapEDUName[type] = std::string( name ) ;

      if ( system )
      {
         mapEDUTypeSys[type] = type ;
      }
   done :
      PD_TRACE_EXIT ( SDB_REGEDUNAME );
      return rc ;
   error :
      goto done ;
   }

   const CHAR * getEDUStatusDesp ( EDU_STATUS status )
   {
      const CHAR *desp = "Unknown" ;

      switch ( status )
      {
         case PMD_EDU_CREATING :
            desp = "Creating" ;
            break ;
         case PMD_EDU_RUNNING :
            desp = "Running" ;
            break ;
         case PMD_EDU_WAITING :
            desp = "Waiting" ;
            break ;
         case PMD_EDU_IDLE :
            desp = "Idle" ;
            break ;
         case PMD_EDU_DESTROY :
            desp = "Destroying" ;
            break ;
         default :
            break ;
      }

      return desp ;
   }

   const CHAR * getEDUName( EDU_TYPES type )
   {
      std::map<EDU_TYPES, std::string>::iterator it =
         mapEDUName.find ( type ) ;
      if ( it != mapEDUName.end() )
      {
         return it->second.c_str() ;
      }

      return "Unknow" ;
   }

   BOOLEAN isSystemEDU ( EDU_TYPES type )
   {
      std::map<EDU_TYPES,EDU_TYPES>::iterator it = mapEDUTypeSys.find( type ) ;
      return it == mapEDUTypeSys.end() ? FALSE : TRUE ;
   }

   // array assignment later, can't inherit from SDBObject
   struct _eduEntryInfo
   {
      EDU_TYPES         type ;
      INT32             regResult ;
      pmdEntryPoint     entryFunc ;
   };

#define ON_EDUTYPE_TO_ENTRY1(type, system, entry, desp) \
   { type, registerEDUName(type, desp, system), entry }

#define ON_EDUTYPE_TO_ENTRY2(type, system, entry) \
   ON_EDUTYPE_TO_ENTRY1(type, system, entry, #type)

   // PD_TRACE_DECLARE_FUNCTION ( SDB_GETENTFUNCBYTYPE, "getEntryFuncByType" )
   pmdEntryPoint getEntryFuncByType ( EDU_TYPES type )
   {
      pmdEntryPoint rt = NULL ;
      PD_TRACE_ENTRY ( SDB_GETENTFUNCBYTYPE );
      static const _eduEntryInfo entry[] = {
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_SHARDAGENT, FALSE,
                                pmdShardAgentEntryPoint,
                                "ShardAgent" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_COORDAGENT, FALSE,
                                pmdAgentEntryPoint,
                                "CoordAgent" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_AGENT, FALSE,
                                pmdAgentEntryPoint,
                                "Agent" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_REPLAGENT, FALSE,
                                pmdReplAgentEntryPoint,
                                "ReplAgent" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_HTTPAGENT, FALSE,
                                pmdHTTPAgentEntryPoint,
                                "HTTPAgent" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_TCPLISTENER, TRUE,
                                pmdTcpListenerEntryPoint,
                                "TCPListener" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_HTTPLISTENER, TRUE,
                                pmdHTTPListenerEntryPoint,
                                "HTTPListener" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_CLUSTER, TRUE,
                                pmdClusterEntryPoint,
                                "Cluster" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_CLUSTERSHARD, TRUE,
                                pmdClusterShardEntryPoint,
                                "ClusterShard" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_REPR, TRUE,
                                pmdRepREntryPoint,
                                "ReplReader" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_LOGGW, TRUE,
                                pmdLoggWEntryPoint,
                                "LogWriter" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_LOGGNTY, TRUE,
                                pmdLoggNtyEntryPoint,
                                "LogNotify" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_SHARDR, TRUE,
                                pmdShardREntryPoint,
                                "ShardReader" ),
#if defined (_WINDOWS)
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_WINDOWSLISTENER, TRUE,
                                pmdWindowsListenerEntryPoint,
                                "WindowsListener" ),
#endif
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_BACKGROUND_JOB, FALSE,
                                pmdBackgroundJobEntryPoint,
                                "Task" ),

         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_CATMAINCONTROLLER, TRUE,
                                pmdCatMainControllerEntryPoint,
                                "CatalogMC" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_CATNODEMANAGER, TRUE,
                                pmdCatNodeManagerEntryPoint,
                                "CatalogNM" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_CATCATALOGUEMANAGER, TRUE,
                                pmdCatCatalogManagerEntryPoint,
                                "CatalogManager" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_CATNETWORK, TRUE,
                                pmdCatNetWorkEntryPoint,
                                "CatalogNetwork" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_COORDNETWORK, TRUE,
                                pmdCoordNetWorkEntryPoint,
                                "CoordNetwork" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_DPSROLLBACK_TASK, TRUE,
                                pmdDpsTransRollbackTaskEntryPoint,
                                "DpsRollbackTask"),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_LOADWORKER, FALSE,
                                pmdLoadWorkerEntryPoint,
                                "MigLoadWork" ),
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_PREFETCHER, FALSE,
                                pmdPreLoaderEntryPoint,
                                "PreLoader" ),

         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_SYNCCLOCK, TRUE,
                                pmdSyncClock,
                                "SyncClockWorker" ),
         //TODO:

         // For the end
         ON_EDUTYPE_TO_ENTRY1 ( EDU_TYPE_MAXIMUM, FALSE,
                                NULL,
                                "Unknow" )
      };

      static const UINT32 number = sizeof ( entry ) / sizeof ( _eduEntryInfo ) ;

      UINT32 index = 0 ;
      for ( ; index < number ; index ++ )
      {
         if ( entry[index].type == type )
         {
            rt = entry[index].entryFunc ;
            goto done ;
         }
      }

   done :
      PD_TRACE_EXIT ( SDB_GETENTFUNCBYTYPE );
      return rt ;
   }

   _pmdEDUCB::_pmdEDUCB( _pmdEDUMgr *mgr, EDU_TYPES type ) :
   _tid(0),
   _processEventCount(0),
   _eduMgr(mgr),
   _eduType(type),
   _ctrlFlag(0),
   _writingDB(FALSE),
   _threadHdl(0),
#if defined (_LINUX)
   _threadID(0),
#endif
   _pClientSock(NULL),
   _pCoordSession(NULL),
   _beginLsn(0),
   _endLsn(0),
   _lsnNumber(0),
   _curTransLSN(DPS_INVALID_LSN_OFFSET),
   _curTransID(DPS_INVALID_TRANS_ID),
   _pTransNodeMap(NULL),
   _isDoRollback(FALSE),
   _transRC(SDB_OK),
   _pCompressionBuffer(NULL),
   _compressionBufferSize(0),
   _pUncompressionBuffer(NULL),
   _uncompressionBufferSize(0),
   _pTempCompBuffer(NULL),
   _tempCompBufferSize(0)
   {
      _Name[0] = 0 ;
      _monCfgCB = *( (monConfigCB*)(pmdGetKRCB()->getMonCB()) );
      _pErrorBuff = (CHAR *)SDB_OSS_MALLOC( EDU_ERROR_BUFF_SIZE + 1 );
   }

   _pmdEDUCB::~_pmdEDUCB ()
   {
      // wait for destory
      {
         ossScopedRWLock assist ( &_callInMutex, EXCLUSIVE ) ;
      }

      if ( _pErrorBuff )
      {
         SDB_OSS_FREE ( _pErrorBuff ) ;
         _pErrorBuff = NULL ;
      }
      if ( _pCompressionBuffer )
      {
         SDB_OSS_FREE ( _pCompressionBuffer ) ;
         _pCompressionBuffer = NULL ;
      }
      if ( _pUncompressionBuffer )
      {
         SDB_OSS_FREE ( _pUncompressionBuffer ) ;
         _pUncompressionBuffer = NULL ;
      }
      if ( _pTempCompBuffer )
      {
         SDB_OSS_FREE ( _pTempCompBuffer ) ;
         _pTempCompBuffer = NULL ;
      }
      DpsTransCBLockList::iterator iterLst = _transLockLst.begin();
      while( iterLst != _transLockLst.end() )
      {
         if ( iterLst->second )
         {
            SDB_OSS_DEL iterLst->second ;
         }
         _transLockLst.erase( iterLst++ );
      }
      if ( _pTransNodeMap )
      {
         delete _pTransNodeMap;
         _pTransNodeMap = NULL;
      }
   }

   void _pmdEDUCB::interrupt ()
   {
      _ctrlFlag |= EDU_CTRL_INTERRUPTED ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_DISCONNECT, "_pmdEDUCB::disconnect" )
   void _pmdEDUCB::disconnect ()
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_DISCONNECT );
      ossScopedRWLock assist ( &_callInMutex, SHARED ) ;
      interrupt () ;
      _ctrlFlag |= EDU_CTRL_DISCONNECTED ;
      postEvent ( pmdEDUEvent ( PMD_EDU_EVENT_TERM ) ) ;
      PD_TRACE_EXIT ( SDB__PMDEDUCB_DISCONNECT );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_FORCE, "_pmdEDUCB::force" )
   void _pmdEDUCB::force ()
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_FORCE );
      ossScopedRWLock assist ( &_callInMutex, SHARED ) ;
      disconnect () ;
      _ctrlFlag |= EDU_CTRL_FORCED ;
      PD_TRACE_EXIT ( SDB__PMDEDUCB_FORCE );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_ISINT, "_pmdEDUCB::isInterrupted" )
   BOOLEAN _pmdEDUCB::isInterrupted ()
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_ISINT );
      BOOLEAN ret = FALSE ;

      // mask interrupt while doing rollback
      if ( _isDoRollback )
      {
         goto done;
      }
      if ( _ctrlFlag & EDU_CTRL_INTERRUPTED )
      {
         ret = TRUE ;
         goto done ;
      }
      else if ( _pClientSock )
      {
         INT32 receivedLen ;
         MsgHeader header ;
         INT32 rc = _pClientSock->recv( (CHAR*)&header , sizeof(header),
                                        receivedLen, 0, MSG_PEEK ) ;
         if ( ( rc >= (INT32)sizeof(header)
                && MSG_BS_DISCONNECT == header.opCode )
              || SDB_NETWORK_CLOSE == rc || SDB_NETWORK == rc )
         {
            _ctrlFlag |= ( EDU_CTRL_INTERRUPTED | EDU_CTRL_DISCONNECTED ) ;
            ret = TRUE ;
         }
         else if ( rc >= (INT32)sizeof(header)
                   && MSG_BS_INTERRUPTE == header.opCode )
         {
            _ctrlFlag |= EDU_CTRL_INTERRUPTED ;
            ret = TRUE ;
         }
      }
   done :
      PD_TRACE1 ( SDB__PMDEDUCB_ISINT, PD_PACK_INT(ret) );
      PD_TRACE_EXIT ( SDB__PMDEDUCB_ISINT );
      return ret ;
   }

   BOOLEAN _pmdEDUCB::isDisconnected ()
   {
      return ( _ctrlFlag & EDU_CTRL_DISCONNECTED ) ? TRUE : FALSE ;
   }

   BOOLEAN _pmdEDUCB::isForced ()
   {
      return ( _ctrlFlag & EDU_CTRL_FORCED ) ? TRUE : FALSE ;
   }

   void _pmdEDUCB::resetInterrupt ()
   {
      _ctrlFlag &= ~EDU_CTRL_INTERRUPTED ;
   }

   void _pmdEDUCB::resetDisconnect ()
   {
      resetInterrupt () ;
      _ctrlFlag &= ~EDU_CTRL_DISCONNECTED ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_CONTXTPEEK, "_pmdEDUCB::contextPeek" )
   SINT64 _pmdEDUCB::contextPeek ()
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_CONTXTPEEK );
      EDUCB_XLOCK
      SINT64 contextID = -1 ;
      std::set<SINT64>::const_iterator it ;
      if ( _contextList.empty() )
         goto done ;
      it = _contextList.begin() ;
      contextID = (*it) ;
      _contextList.erase(it) ;
   done :
      PD_TRACE1 ( SDB__PMDEDUCB_CONTXTPEEK, PD_PACK_LONG(contextID) );
      PD_TRACE_EXIT ( SDB__PMDEDUCB_CONTXTPEEK );
      return contextID ;
   }

   void _pmdEDUCB::setClientInfo ( const CHAR *clientName, UINT16 clientPort )
   {
      EDUCB_XLOCK
      ossSnprintf( _Name, PMD_EDU_NAME_LENGTH, "%s:%u",
                   clientName, clientPort ) ;
      _Name[PMD_EDU_NAME_LENGTH] = 0 ;
   }

   void _pmdEDUCB::setName ( const CHAR * name )
   {
      EDUCB_XLOCK
      ossStrncpy ( _Name, name, PMD_EDU_NAME_LENGTH ) ;
      _Name[PMD_EDU_NAME_LENGTH] = 0 ;
   }

   void _pmdEDUCB::clearTransInfo()
   {
      _curTransID = DPS_INVALID_TRANS_ID ;
      _curTransLSN = DPS_INVALID_LSN_OFFSET ;
      dpsTransCB *pTransCB = pmdGetKRCB()->getTransCB();
      if ( pTransCB )
      {
         pTransCB->transLockReleaseAll( this );
      }
   }

   void _pmdEDUCB::clear()
   {
      pmdEDUEvent data;
      while ( _queue.try_pop( data ) )
      {
         if ( data._release && data._Data )
         {
            SDB_OSS_FREE ( data._Data ) ;
         }
      }
      _processEventCount = 0 ;
      _Name[0] = 0 ;
      clearTransInfo();
   }

   void _pmdEDUCB::setClientSock ( ossSocket *pSock )
   {
      _pClientSock = pSock ;
   }

   ossSocket *_pmdEDUCB::getClientSock ( )
   {
      return _pClientSock ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB___PMDEDUCB_DUMPINFO, "_pmdEDUCB::dumpInfo" )
   void _pmdEDUCB::dumpInfo ( monEDUSimple &simple )
   {
      PD_TRACE_ENTRY ( SDB___PMDEDUCB_DUMPINFO );
      EDUCB_SLOCK
      ossMemset ( &simple._eduStatus, 0, MON_EDU_STATUS_SZ ) ;
      ossMemset ( &simple._eduType, 0, MON_EDU_TYPE_SZ ) ;
      ossMemset ( &simple._eduName, 0, MON_EDU_NAME_SZ ) ;
      simple._eduID = _eduID ;
      simple._tid = _tid ;
      ossStrncpy ( simple._eduStatus, getEDUStatusDesp(_status),
                   MON_EDU_STATUS_SZ ) ;
      ossStrncpy ( simple._eduType, getEDUName (_eduType), MON_EDU_TYPE_SZ ) ;
      ossStrncpy ( simple._eduName, _Name, MON_EDU_NAME_SZ ) ;
      PD_TRACE_EXIT ( SDB___PMDEDUCB_DUMPINFO );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB___PMDEDUCB_DUMPINFO2, "_pmdEDUCB::dumpInfo" )
   void _pmdEDUCB::dumpInfo ( monEDUFull &full )
   {
      PD_TRACE_ENTRY ( SDB___PMDEDUCB_DUMPINFO2 );
      EDUCB_SLOCK
      ossMemset ( &full._eduStatus, 0, MON_EDU_STATUS_SZ ) ;
      ossMemset ( &full._eduType, 0, MON_EDU_TYPE_SZ ) ;
      ossMemset ( &full._eduName, 0, MON_EDU_NAME_SZ ) ;
      full._eduID = _eduID ;
      full._tid = _tid ;
      full._processEventCount = _processEventCount ;
      full._queueSize = _queue.size() ;
      ossStrncpy ( full._eduStatus, getEDUStatusDesp(_status),
                   MON_EDU_STATUS_SZ ) ;
      ossStrncpy ( full._eduType, getEDUName (_eduType), MON_EDU_TYPE_SZ ) ;
      ossStrncpy ( full._eduName, _Name, MON_EDU_NAME_SZ ) ;

      full._monApplCB = _monApplCB ;
      full._threadHdl = _threadHdl ;

      full._eduContextList = _contextList ;
      PD_TRACE_EXIT ( SDB___PMDEDUCB_DUMPINFO2 );
   }

   CHAR *_pmdEDUCB::_getBuffInfo ( EDU_INFO_TYPE type, UINT32 & size )
   {
      CHAR *buff = NULL ;
      switch ( type )
      {
         case EDU_INFO_ERROR :
            buff = _pErrorBuff ;
            size = EDU_ERROR_BUFF_SIZE ;
            break ;
         default :
            break ;
      }

      return buff ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_PRINTINFO, "_pmdEDUCB::printInfo" )
   INT32 _pmdEDUCB::printInfo ( EDU_INFO_TYPE type, const CHAR * format, ... )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_PRINTINFO );
      //already exist, return ok
      if ( getInfo ( type ) )
      {
         goto done ;
      }

      {
      UINT32 buffSize = 0 ;
      CHAR *buff = _getBuffInfo ( type, buffSize ) ;

      if ( NULL == buff || buffSize == 0 )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      va_list ap ;
      va_start ( ap, format ) ;
      vsnprintf ( buff, buffSize, format, ap ) ;
      va_end ( ap ) ;

      buff[ buffSize ] = 0 ;
      }

   done :
      PD_TRACE_EXITRC ( SDB__PMDEDUCB_PRINTINFO, rc );
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_GETINFO, "_pmdEDUCB::getInfo" )
   const CHAR *_pmdEDUCB::getInfo ( EDU_INFO_TYPE type )
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_GETINFO );
      UINT32 buffSize = 0 ;
      CHAR *buff = _getBuffInfo ( type, buffSize ) ;
      if ( buff && buff[0] != 0 )
      {
         PD_TRACE_EXIT ( SDB__PMDEDUCB_GETINFO );
         return buff ;
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_GETINFO );
      return NULL ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_RESETINFO, "_pmdEDUCB::resetInfo" )
   void _pmdEDUCB::resetInfo ( EDU_INFO_TYPE type )
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_RESETINFO );
      UINT32 buffSize = 0 ;
      CHAR *buff = _getBuffInfo ( type, buffSize ) ;
      if ( buff )
      {
         buff[0] = 0 ;
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_RESETINFO );
   }
   void  _pmdEDUCB::addBpEvents(pmdEDUEvent event)
   {
      _bpEventQueue.push(event);
   }

   void  _pmdEDUCB::clearBpEvents()
   {
      while( !_bpEventQueue.empty() )
      {
         pmdEDUEvent event;
         _bpEventQueue.try_pop(event);
         postEvent(event);
      }
   }


   void _pmdEDUCB::setTransID( DPS_TRANS_ID transID )
   {
      _curTransID = transID;
   }

   DPS_TRANS_ID _pmdEDUCB::getTransID()
   {
      return _curTransID;
   }

   void _pmdEDUCB::setCurTransLsn( DPS_LSN_OFFSET curLsn )
   {
      _curTransLSN = curLsn;
   }

   DPS_LSN_OFFSET _pmdEDUCB::getCurTransLsn()
   {
      return _curTransLSN;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_GETTRANSLOCK, "_pmdEDUCB::getTransLock" )
   dpsTransCBLockInfo *_pmdEDUCB::getTransLock( const dpsTransLockId &lockId )
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_GETTRANSLOCK );
      dpsTransCBLockInfo *pLockInfo = NULL;
      DpsTransCBLockList::iterator iterLst = _transLockLst.find( lockId );
      if ( iterLst != _transLockLst.end() )
      {
         pLockInfo = iterLst->second ;
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_GETTRANSLOCK );
      return pLockInfo;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_ADDLOCKINFO, "_pmdEDUCB::addLockInfo" )
   void _pmdEDUCB::addLockInfo( const dpsTransLockId &lockId, DPS_TRANSLOCK_TYPE lockType )
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_ADDLOCKINFO );
      dpsTransCBLockInfo *pLockInfo = NULL ;
      pLockInfo = SDB_OSS_NEW dpsTransCBLockInfo( lockType );
      if ( pLockInfo )
      {
         _transLockLst[ lockId ] = pLockInfo ;
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_ADDLOCKINFO );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_DELLOCKINFO, "_pmdEDUCB::delLockInfo" )
   void _pmdEDUCB::delLockInfo( const dpsTransLockId &lockId )
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_DELLOCKINFO );
      DpsTransCBLockList::iterator iter
                        = _transLockLst.find( lockId );
      if ( iter != _transLockLst.end() )
      {
         SDB_OSS_DEL iter->second;
         _transLockLst.erase( iter );
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_DELLOCKINFO );
   }

   DpsTransCBLockList *_pmdEDUCB::getLockList()
   {
      return &_transLockLst;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_CREATETRANSACTION, "_pmdEDUCB::createTransaction" )
   INT32 _pmdEDUCB::createTransaction()
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_CREATETRANSACTION );
      INT32 rc = SDB_OK;
      if ( NULL == _pTransNodeMap )
      {
         _pTransNodeMap = new DpsTransNodeMap;
         setTransRC(SDB_OK);
      }
      if ( NULL == _pTransNodeMap )
      {
         rc = SDB_OOM;
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_CREATETRANSACTION );
      return rc;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_DELTRANSACTION, "_pmdEDUCB::delTransaction" )
   void _pmdEDUCB::delTransaction()
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_DELTRANSACTION );
      if ( _pTransNodeMap )
      {
         delete _pTransNodeMap;
         _pTransNodeMap = NULL;
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_DELTRANSACTION );
   }

   DpsTransNodeMap *_pmdEDUCB::getTransNodeLst()
   {
      return _pTransNodeMap;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_ADDTRANSNODE, "_pmdEDUCB::addTransNode" )
   void _pmdEDUCB::addTransNode( MsgRouteID &routeID )
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_ADDTRANSNODE );
      if ( _pTransNodeMap )
      {
         (*_pTransNodeMap)[routeID.columns.groupID] = routeID;
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_ADDTRANSNODE );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_GETTRANSNODEROUTEID, "_pmdEDUCB::getTransNodeRouteID" )
   void _pmdEDUCB::getTransNodeRouteID( UINT32 groupID,
                                       MsgRouteID &routeID )
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_GETTRANSNODEROUTEID );
      DpsTransNodeMap::iterator iterMap;
      routeID.value = 0;
      if ( _pTransNodeMap )
      {
         iterMap = _pTransNodeMap->find( groupID );
         if ( iterMap != _pTransNodeMap->end() )
         {
            routeID = iterMap->second;
         }
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_GETTRANSNODEROUTEID );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_ISTRANSNODE, "_pmdEDUCB::isTransNode" )
   BOOLEAN _pmdEDUCB::isTransNode( MsgRouteID &routeID )
   {
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_ISTRANSNODE );
      BOOLEAN isTransNode = FALSE;
      DpsTransNodeMap::iterator iterMap;
      if ( _pTransNodeMap )
      {
         iterMap = _pTransNodeMap->find( routeID.columns.groupID );
         if (  iterMap != _pTransNodeMap->end() )
         {
            isTransNode = TRUE;
         }
      }
      PD_TRACE_EXIT ( SDB__PMDEDUCB_ISTRANSNODE );
      return isTransNode;
   }

   BOOLEAN _pmdEDUCB::isTransaction()
   {
      if ( _pTransNodeMap )
      {
         return TRUE;
      }
      return FALSE;
   }

   void _pmdEDUCB::startRollback()
   {
      _isDoRollback = TRUE;
   }

   void _pmdEDUCB::stopRollback()
   {
      _isDoRollback = FALSE;
   }

   void _pmdEDUCB::setTransRC( INT32 rc )
   {
      _transRC = rc ;
   }

   INT32 _pmdEDUCB::getTransRC()
   {
      return _transRC ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_REALLOCCOMPBUF, "_pmdEDUCB::reallocCompressionBuffer" )
   INT32 _pmdEDUCB::reallocCompressionBuffer ( INT32 requestedSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_REALLOCCOMPBUF );
      PD_TRACE2 ( SDB__PMDEDUCB_REALLOCCOMPBUF,
                  PD_PACK_INT(requestedSize),
                  PD_PACK_INT(_compressionBufferSize) ) ;
      if ( requestedSize > _compressionBufferSize )
      {
         CHAR *pOrigMem = _pCompressionBuffer ;
         INT32 newSize = ossRoundUpToMultipleX ( requestedSize,
                                                 SDB_PAGE_SIZE ) ;
         PD_CHECK ( newSize >= 0, SDB_INVALIDARG, error, PDERROR,
                    "realloc compression buffer overflow" ) ;
         _pCompressionBuffer = (CHAR*)SDB_OSS_REALLOC ( _pCompressionBuffer,
                                                        newSize ) ;
         if ( !_pCompressionBuffer )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for "
                     "compression buffer, size = %d", newSize ) ;
            rc = SDB_OOM ;
            // realloc does NOT free original memory if it fails, so we have to
            // assign pointer to original
            _pCompressionBuffer = pOrigMem ;
            goto error ;
         }
         _compressionBufferSize = newSize ;
      }
   done :
      PD_TRACE_EXITRC ( SDB__PMDEDUCB_REALLOCCOMPBUF, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_REALLOCUNCOMPBUF, "_pmdEDUCB::reallocUncompressionBuffer" )
   INT32 _pmdEDUCB::reallocUncompressionBuffer ( INT32 requestedSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_REALLOCUNCOMPBUF );
      PD_TRACE2 ( SDB__PMDEDUCB_REALLOCUNCOMPBUF,
                  PD_PACK_INT(requestedSize),
                  PD_PACK_INT(_uncompressionBufferSize) ) ;
      if ( requestedSize > _uncompressionBufferSize )
      {
         CHAR *pOrigMem = _pUncompressionBuffer ;
         INT32 newSize = ossRoundUpToMultipleX ( requestedSize,
                                                 SDB_PAGE_SIZE ) ;
         PD_CHECK ( newSize >= 0, SDB_INVALIDARG, error, PDERROR,
                    "realloc uncompression buffer overflow" ) ;
         _pUncompressionBuffer = (CHAR*)SDB_OSS_REALLOC ( _pUncompressionBuffer,
                                                          newSize ) ;
         if ( !_pUncompressionBuffer )
         {
            PD_LOG ( PDERROR, "Failed to allocate memory for "
                     "uncompression buffer, size = %d", newSize ) ;
            rc = SDB_OOM ;
            // realloc does NOT free original memory if it fails, so we have to
            // assign pointer to original
            _pUncompressionBuffer = pOrigMem ;
            goto error ;
         }
         _uncompressionBufferSize = newSize ;
      }
   done :
      PD_TRACE_EXITRC ( SDB__PMDEDUCB_REALLOCUNCOMPBUF, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_COMPRESS, "_pmdEDUCB::compress" )
   INT32 _pmdEDUCB::compress ( const CHAR *pInputData, INT32 inputSize,
                               CHAR **ppData, INT32 *pDataSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_COMPRESS );
      SDB_ASSERT ( pInputData && ppData && pDataSize,
                   "Data pointer and size pointer can't be NULL" )

      // estimate the max possible size for compressed data
      size_t maxCompressedLen = snappy::MaxCompressedLength ( inputSize ) ;
      // make sure we have enough memory for it
      rc = reallocCompressionBuffer ( (INT32)maxCompressedLen ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Unable to allocate %d bytes compression buffer, rc = %d",
                    (INT32)maxCompressedLen, rc ) ;
      // let's rock :)
      snappy::RawCompress ( pInputData, (size_t)inputSize,
                            _pCompressionBuffer, &maxCompressedLen ) ;
      // assign the output buffer pointer
      *ppData = _pCompressionBuffer ;
      *pDataSize = (INT32)maxCompressedLen ;
   done :
      PD_TRACE_EXITRC ( SDB__PMDEDUCB_COMPRESS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_COMPRESS1, "_pmdEDUCB::compress" )
   INT32 _pmdEDUCB::compress ( const BSONObj &obj, const CHAR* pOIDPtr,
                               INT32 oidLen, CHAR **ppData, INT32 *pDataSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_COMPRESS1 ) ;
      // if we want to append OID, then
      if ( oidLen && pOIDPtr )
      {
         // get the requested size by adding object size and oid size
         INT32 requestedSize = obj.objsize() + oidLen ;
         // check if we need to allocate new buffer
         if ( obj.objsize() + oidLen > _tempCompBufferSize )
         {
            CHAR *pOrigMem = _pTempCompBuffer ;
            INT32 newSize = ossRoundUpToMultipleX ( requestedSize,
                                                    SDB_PAGE_SIZE ) ;
            PD_CHECK ( newSize >= 0, SDB_INVALIDARG, error, PDERROR,
                       "realloc temp compression buffer overflow" ) ;
            // reallocate memory
            _pTempCompBuffer = (CHAR*)SDB_OSS_REALLOC(_pTempCompBuffer,
                                                      newSize ) ;
            // make sure memory is available
            if ( !_pTempCompBuffer )
            {
               PD_LOG ( PDERROR, "Failed to allocate memory for "
                        "temp compression buffer, size = %d", newSize ) ;
               rc = SDB_OOM ;
               // if we failed to realloc, we have to restore the original
               // pointer
               _pTempCompBuffer = pOrigMem ;
               goto error ;
            }
            // change the memory size
            _tempCompBufferSize = newSize ;
         }
         // actually append, note we substract DMS_RECORD_METADATA_SZ from
         // _pTempCompBuffer because DMS_RECORD_SETDATA_OID will add
         // DMS_RECORD_METADATA_SZ for all offsets
         DMS_RECORD_SETDATA_OID (
               _pTempCompBuffer - DMS_RECORD_METADATA_SZ,
               obj.objdata(), obj.objsize(), BSONElement(pOIDPtr) ) ;
         rc = compress ( _pTempCompBuffer, BSONObj(_pTempCompBuffer).objsize(),
                         ppData, pDataSize ) ;
      }
      else
         rc = compress ( obj.objdata(), obj.objsize(), ppData, pDataSize ) ;
   done :
      PD_TRACE_EXITRC ( SDB__PMDEDUCB_COMPRESS1, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__PMDEDUCB_UNCOMPRESS, "_pmdEDUCB::uncompress" )
   INT32 _pmdEDUCB::uncompress ( const CHAR *pInputData, INT32 inputSize,
                                 CHAR **ppData, INT32 *pDataSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDEDUCB_UNCOMPRESS );
      SDB_ASSERT ( pInputData && ppData && pDataSize,
                   "Data pointer and size pointer can't be NULL" )

      size_t maxUncompressedLen = 0 ;
      // estimate the max possible size for uncompressed data + sanity check
      rc = snappy::GetUncompressedLength ( pInputData, (size_t)inputSize,
                                           &maxUncompressedLen ) ;
      PD_CHECK ( rc, SDB_CORRUPTED_RECORD, error, PDERROR,
                 "Failed to get uncompressed length" ) ;
      // make sure we have enough memory for it
      rc = reallocUncompressionBuffer ( (INT32)maxUncompressedLen ) ;
      PD_RC_CHECK ( rc, PDERROR,
                    "Unable to allocate %d bytes uncompression buffer, rc = %d",
                    (INT32)maxUncompressedLen, rc ) ;
      // let's rock :)
      rc = snappy::RawUncompress ( pInputData, (size_t)inputSize,
                                   _pUncompressionBuffer ) ;
      PD_CHECK ( rc, SDB_CORRUPTED_RECORD, error, PDERROR,
                 "Failed to uncompress record" ) ;
      rc = SDB_OK ;
      // assign return value
      *ppData = _pUncompressionBuffer ;
      *pDataSize = (INT32)maxUncompressedLen ;
   done :
      PD_TRACE_EXITRC ( SDB__PMDEDUCB_UNCOMPRESS, rc ) ;
      return rc ;
   error :
      goto done ;
   }

   INT32 pmdEDUEntryPointWrapper ( EDU_TYPES type, pmdEDUCB *cb, void *arg )
   {
#if defined (_WINDOWS)
      __try
      {
#endif
         return pmdEDUEntryPoint ( type, cb, arg ) ;
#if defined (_WINDOWS)
      }
      __except ( engine::ossEDUExceptionFilter ( GetExceptionInformation() ) )
      {}
#endif
      return SDB_SYS ;
   }

   // main entry point for all EDUs
   // it will call individual main function for each EDU type
   // entry points are defined in getEntryFuncByType
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDEDUENTPNT, "pmdEDUEntryPoint" )
   INT32 pmdEDUEntryPoint ( EDU_TYPES type, pmdEDUCB *cb, void *arg )
   {
      INT32       rc           = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDEDUENTPNT );
      pmdKRCB     *krcb        = pmdGetKRCB () ;
      EDUID       myEDUID      = cb->getID () ; // edu id for myself
      pmdEDUMgr  *eduMgr       = cb->getEDUMgr() ; // the manager class
      pmdEDUEvent event ;
      BOOLEAN     eduDestroyed = FALSE ;
      BOOLEAN     isForced     = FALSE ;

      // save kernel thread id ( Linux ), or thread handle ( windows )
   #if defined (_WINDOWS)
      HANDLE      tHdl = NULL ;
      BOOLEAN     isHdlCreated = false ;

      if ( DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
                            GetCurrentProcess(), &tHdl, 0, false, 
                            DUPLICATE_SAME_ACCESS ) )
      {
         isHdlCreated = true ;
      }
   #elif defined (_LINUX )
      OSSTID tHdl = ossGetCurrentThreadID() ;
      cb->setThreadID ( ossPThreadSelf() ) ;
   #endif
      cb->setThreadHdl( tHdl ) ;
      cb->setTID ( ossGetCurrentThreadID() ) ;
      eduMgr->setEDU ( ossGetCurrentThreadID(), myEDUID ) ;

      PD_LOG ( PDDEBUG, "Start EDU %lld for thread id %d", myEDUID, 
         ossGetCurrentThreadID() ) ;

      if ( _ossEduData.get() == 0 )
      {
         _ossEduData.reset( SDB_OSS_NEW oss_edu_data() );
      }

      while ( !eduDestroyed )
      {
         type = cb->getType () ;
         // currently the thread status should be either WAITING or CREATING
         // usually we don't expect agent sitting in creating for long time
         // the thread spawning the agent supposed to post event immediately
         // after the thread is created
         if ( !cb->waitEvent ( event, 1000 ) )
         {
            // if don't receive anything in 1000 milliseconds,
            // we should check "killed" mark for this session
            // if we continue to run, then run
            if ( cb->isForced () )
            {
               // we break the main loop, then we go ahead to call
               // destroyEDU to destroy the memory
               // this should be safe because in this code path
               // the status of edu cannot be RUNNING
               // then in either CREATING/IDLE/WAITING status
               // it should be safe for us to destroy it
               PD_LOG ( PDEVENT, "EDU %lld is forced", myEDUID ) ;
               isForced = TRUE ;
            }
            else
               continue ;
         }

         if ( !isForced && PMD_EDU_EVENT_RESUME == event._eventType )
         {
            // set EDU status to wait
            eduMgr->waitEDU ( cb->getID () ) ;
            // find their main function entry by type
            pmdEntryPoint entryFunc = getEntryFuncByType ( cb->getType() ) ;
            if ( NULL == entryFunc )
            {
               PD_LOG ( PDERROR , "EDU[type=%d] entry point func is NULL",
                        cb->getType() ) ;
               PMD_SHUTDOWN_DB( SDB_SYS ) ;
               rc = SDB_SYS ;
            }
            else
            {
               // initial monCfgCB
               *(cb->getMonConfigCB() ) = *( (monConfigCB*)(krcb->getMonCB()) );
               // initial monAppCB
               cb->initMonAppCB() ;

               PD_LOG ( PDDEBUG, "Start edu[EDUID:%lld, TID:%d]", myEDUID,
                        cb->getTID() ) ;
               rc = entryFunc ( cb, event._Data ) ;
            }

            if ( PMD_IS_DB_UP )
            {
               if ( isSystemEDU( cb->getType() ) )
               {
                  PD_LOG ( PDSEVERE, "System EDU[ID:%lld, type:%s] exits with %d",
                           cb->getID(), getEDUName(cb->getType()), rc ) ;
                  PMD_SHUTDOWN_DB( rc ) ;
               }
               else if ( SDB_OK != rc )
               {
                  PD_LOG ( PDWARNING, "EDU[ID:%lld, type:%s, Name:%s] exits with %d",
                           cb->getID(), getEDUName(cb->getType()),
                           cb->getName(), rc ) ;
               }
            }

            // set EDU status to wait
            eduMgr->waitEDU ( cb->getID () ) ;
         }
         else if ( !isForced && PMD_EDU_EVENT_TERM != event._eventType )
         {
            //the event is error
            PD_LOG ( PDERROR, "Recieve the error event[type=%d] in EDU[ID:%lld, type:%s]", 
                     event._eventType, myEDUID, getEDUName(cb->getType()) ) ;
            rc = SDB_SYS ;
         }
         else if ( !isForced && PMD_EDU_EVENT_TERM == event._eventType
            && cb->isForced () )
         {
            PD_LOG ( PDEVENT, "EDU[ID:%lld, type:%s] is forced", myEDUID,
                     getEDUName(cb->getType()) ) ;
            isForced = TRUE ;
         }

         // release the event data
         if ( !isForced && event._Data && event._release )
         {
            SDB_OSS_FREE ( event._Data ) ;
            event.reset () ;
         }

         // call return EDU to return the EDU to pool. pool will decide whether
         // to destroy it or continue let it run
         // eduDestroyed argument will be assigned when pool want to destroy
         // the thread

         //reset and clear
         cb->resetMonAppCB() ;
         cb->clear() ;

         //delete all leak context
         {
            SDB_RTNCB *rtnCB = pmdGetKRCB()->getRTNCB() ;
            SINT64 contextID = -1 ;
            while ( -1 != (contextID = cb->contextPeek() ))
            {
               rtnCB->contextDelete( contextID, NULL ) ;
               PD_LOG ( PDWARNING, "EDU[%lld,%s] context[%d] leaked",
                        myEDUID, getEDUName(type), contextID ) ;
            }
         }

         rc = eduMgr->returnEDU ( cb->getID (), isForced, &eduDestroyed ) ;

         // otherwise let's check rc and report error if it's not OK
         if ( SDB_OK != rc )
         {
            PD_LOG ( PDERROR, "Invalid EDU Status for EDU[ID:%lld, type:%s]",
                     myEDUID, getEDUName( type ) ) ;
         }
      }
      PD_LOG ( PDDEBUG, "Terminating thread for EDU[ID:%lld, type:%s]",
               myEDUID, getEDUName( type ) ) ;

   #if defined (_WINDOWS)
      // close handle
      if ( isHdlCreated )
      {
         CloseHandle( tHdl ) ;
      }
   #endif
      PD_TRACE_EXITRC ( SDB_PMDEDUENTPNT, rc );
      return rc ;

   }

   // for pmdRecv, we wait indefinitely until the agent is forced, because
   // client may not send us anything due to idle of user activities
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDRECV, "pmdRecv" )
   INT32 pmdRecv ( CHAR *pBuffer, INT32 recvSize,
                   ossSocket *sock, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( sock, "Socket is NULL" ) ;
      SDB_ASSERT ( cb, "cb is NULL" ) ;
      PD_TRACE_ENTRY ( SDB_PMDRECV );
      INT32 receivedSize = 0 ;
      INT32 totalReceivedSize = 0 ;
      while ( TRUE )
      {
         if ( cb->isForced () )
         {
            rc = SDB_APP_FORCED ;
            goto done ;
         }
         rc = sock->recv ( &pBuffer[totalReceivedSize],
                           recvSize-totalReceivedSize,
                           receivedSize ) ;
         totalReceivedSize += receivedSize ;
         if ( SDB_TIMEOUT == rc )
         {
            continue ;
         }
         goto done ;
      }
   done :
      if ( totalReceivedSize > 0 )
      {
         pmdGetKRCB()->getMonDBCB()->svcNetInAdd( totalReceivedSize ) ;
      }
      PD_TRACE_EXITRC ( SDB_PMDRECV, rc );
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDSEND, "pmdSend" )
   INT32 pmdSend ( const CHAR *pBuffer, INT32 sendSize,
                   ossSocket *sock, pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT ( sock, "Socket is NULL" ) ;
      SDB_ASSERT ( cb, "cb is NULL" ) ;
      PD_TRACE_ENTRY ( SDB_PMDSEND );
      INT32 sentSize = 0 ;
      INT32 totalSentSize = 0 ;
      while ( true )
      {
         if ( cb->isForced () )
         {
            rc = SDB_APP_FORCED ;
            goto done ;
         }
         rc = sock->send ( &pBuffer[totalSentSize],
                           sendSize-totalSentSize,
                           sentSize ) ;
         totalSentSize += sentSize ;
         if ( SDB_TIMEOUT == rc )
            continue ;
         goto done ;
      }
   done :
      if ( totalSentSize > 0 )
      {
         pmdGetKRCB()->getMonDBCB()->svcNetOutAdd( totalSentSize ) ;
      }
      PD_TRACE_EXITRC ( SDB_PMDSEND, rc );
      return rc ;
   }

#if defined (_LINUX)
   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDEDUCODETRAPHNDL, "pmdEDUCodeTrapHandler" )
   void pmdEDUCodeTrapHandler( OSS_HANDPARMS )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDEDUCODETRAPHNDL );
      oss_edu_data * pEduData = NULL ;
      CHAR *dumpPath = NULL ;
      ossGetTrapExceptionPath ( &dumpPath ) ;
      if ( !dumpPath )
         goto done ;

      pEduData = _ossEduData.get() ;

      if ( NULL == pEduData )
      {
         goto done ;
      }

      if ( OSS_AM_I_INSIDE_SIGNAL_HANDLER( pEduData ) )
      {
         goto done ;
      }
      OSS_ENTER_SIGNAL_HANDLER( pEduData ) ;

      if ( signum == OSS_STACK_DUMP_SIGNAL )
      {
         PD_LOG ( PDEVENT,
                  "Signal %d is received, "
                  "prepare to dump stack for all threads", signum ) ;
         std::set<pthread_t>::iterator it ;
         std::set<pthread_t> tidList ;
         pmdGetKRCB()->getEDUMgr()->getEDUThreadID ( tidList ) ;
         for ( it = tidList.begin(); it != tidList.end(); ++it )
         {
            // threadID was initialized to 0 in constructor, and set to real
            // thread id in pmdEDUEntryPoint
            if ( 0 == (*it) )
               continue ;
            rc = ossPThreadKill ( (*it), OSS_STACK_DUMP_SIGNAL_INTERNAL ) ;
            if ( rc )
            {
               PD_LOG ( PDWARNING,
                        "Failed to send signal %d to thread %llu, errno = %d",
                        OSS_STACK_DUMP_SIGNAL_INTERNAL,
                        (*it), ossGetLastError() ) ;
            }
         }
         ossMemTrace ( dumpPath ) ;
      }
      else if ( signum == OSS_STACK_DUMP_SIGNAL_INTERNAL )
      {
         PD_LOG ( PDEVENT,
                  "Signal %d is received, "
                  "prepare to dump stack for %u:%u", signum,
                  ossGetCurrentProcessID(),
                  ossGetCurrentThreadID() ) ;
         ossStackTrace( OSS_HANDARGS, dumpPath ) ;
      }
      else
      {
         PD_LOG ( PDWARNING, "Unexpected signal is received: %d",
                  signum ) ;
      }
      OSS_LEAVE_SIGNAL_HANDLER( pEduData ) ;
   done :
      PD_TRACE1 ( SDB_PMDEDUCODETRAPHNDL, PD_PACK_INT(rc) );
      PD_TRACE_EXIT ( SDB_PMDEDUCODETRAPHNDL );
      return ;
   }
#endif

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDSYNCCLOCK, "pmdSyncClock" )
   INT32 pmdSyncClock( pmdEDUCB * cb, void * arg )
   {
#define SYNCCLOCK_INTERVAL    10    //10ms
      ossTick tmp ;
      // PD_TRACE_ENTRY ( SDB_PMDSYNCCLOCK );
      pmdKRCB *pKrcb = pmdGetKRCB() ;
      while ( !cb->isDisconnected() )
      {
         pKrcb->syncCurTime() ;
         ossSleep( SYNCCLOCK_INTERVAL ) ;
      }

      // PD_TRACE_EXITRC ( SDB_PMDSYNCCLOCK, rc );
      return SDB_OK ;
   }
}
