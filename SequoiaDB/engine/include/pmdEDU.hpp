/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdEDU.hpp

   Descriptive Name = Process MoDel Engine Dispatchable Unit Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains structure for EDU Control
   Block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDEDU_HPP__
#define PMDEDU_HPP__

#include "ossLatch.hpp"
#include "ossQueue.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "ossSocket.hpp"
#include "oss.hpp"
#include "pmdDef.hpp"
#include "monEDU.hpp"
#include "monCB.hpp"
#include "msg.h"
#include "ossRWMutex.hpp"
#include "ossSignal.hpp"
#include "dpsLogDef.hpp"
#include "dpsTransCB.hpp"
#include "dpsTransLockDef.hpp"
#include "../bson/bson.h"
#include "../bson/bsonobj.h"
#include "pdTrace.hpp"
#include <boost/thread.hpp>
#include <set>
using namespace bson ;

namespace engine
{
   #define PMD_EDU_NAME_LENGTH       (512)

   enum EDU_TYPES
   {
      //System EDU Type
      EDU_TYPE_TCPLISTENER = 0,
      EDU_TYPE_HTTPLISTENER,
      EDU_TYPE_REPR,
      EDU_TYPE_LOGGW,
      EDU_TYPE_LOGGNTY,
      EDU_TYPE_SHARDR,
      EDU_TYPE_CLUSTER,
      EDU_TYPE_CLUSTERSHARD,
      EDU_TYPE_CATMAINCONTROLLER,
      EDU_TYPE_CATNODEMANAGER,
      EDU_TYPE_CATCATALOGUEMANAGER,
      EDU_TYPE_CATNETWORK,
      EDU_TYPE_COORDNETWORK,
      EDU_TYPE_SYNCCLOCK,
#if defined (_WINDOWS)
      EDU_TYPE_WINDOWSLISTENER,
#endif
      //Agent EDU Type
      EDU_TYPE_AGENT,
      EDU_TYPE_COORDAGENT,
      EDU_TYPE_SHARDAGENT,
      EDU_TYPE_REPLAGENT,
      EDU_TYPE_HTTPAGENT,
      EDU_TYPE_RESTAGENT,

      //background job EDU Type
      EDU_TYPE_BACKGROUND_JOB,

      //rollback task EDU Type
      EDU_TYPE_DPSROLLBACK_TASK,

      EDU_TYPE_LOADWORKER,
      EDU_TYPE_PREFETCHER,
      EDU_TYPE_UNKNOWN,
      EDU_TYPE_MAXIMUM = EDU_TYPE_UNKNOWN
   } ;

   #define PMD_INVALID_EDUID               0

   // EDU status
   enum EDU_STATUS
   {
      // EDU Manager initialize status to this
      PMD_EDU_CREATING = 0,
      // EDU should change status to running when serve a request
      PMD_EDU_RUNNING,
      // EDU should change to wait after request result send back
      PMD_EDU_WAITING,
      // EDU should change status to idle when get into pool
      PMD_EDU_IDLE,
      // Before terminating, EDU should set to destroy
      PMD_EDU_DESTROY,
      PMD_EDU_UNKNOW,
      PMD_EDU_STATUS_MAXIMUM = PMD_EDU_UNKNOW
   } ;
   const CHAR *getEDUStatusDesp ( EDU_STATUS status ) ;

   #define PMD_IS_EDU_CREATING(x)      ( PMD_EDU_CREATING == x )
   #define PMD_IS_EDU_RUNNING(x)       ( PMD_EDU_RUNNING  == x )
   #define PMD_IS_EDU_WAITING(x)       ( PMD_EDU_WAITING  == x )
   #define PMD_IS_EDU_IDLE(x)          ( PMD_EDU_IDLE     == x )
   #define PMD_IS_EDU_DESTROY(x)       ( PMD_EDU_DESTROY  == x )

   class _pmdEDUMgr ;
   const CHAR *getEDUName ( EDU_TYPES type ) ;
   BOOLEAN     isSystemEDU ( EDU_TYPES type ) ;

   #define EDU_CTRL_INTERRUPTED        0x01
   #define EDU_CTRL_DISCONNECTED       0x02
   #define EDU_CTRL_FORCED             0x04

   #define EDU_ERROR_BUFF_SIZE         1024

   enum EDU_INFO_TYPE
   {
      EDU_INFO_ERROR                   = 1   //Error
   } ;

   class CoordSession;
   /* control block for Engine Dispatchable Unit */
   class _pmdEDUCB : public SDBObject
   {
   public:
      _pmdEDUCB( _pmdEDUMgr *mgr, EDU_TYPES type ) ;
      ~_pmdEDUCB () ;

   #ifdef EDUCB_XLOCK
   #undef EDUCB_XLOCK
   #endif
   #define EDUCB_XLOCK ossScopedLock _lock ( &_mutex, EXCLUSIVE ) ;
   #ifdef EDUCB_SLOCK
   #undef EDUCB_SLOCK
   #endif
   #define EDUCB_SLOCK ossScopedLock _lock ( &_mutex, SHARED ) ;
   private :
      ossEvent       _event ;   // for cls replSet notify
      ossRWMutex     _callInMutex ;
      EDUID          _eduID ;
      UINT32         _tid ;
      UINT64         _processEventCount ;
      _pmdEDUMgr     *_eduMgr ;
      ossSpinSLatch  _mutex ;
      EDU_STATUS     _status ;
      ossQueue<pmdEDUEvent> _queue ;
      EDU_TYPES      _eduType ;

      INT32          _ctrlFlag ;
      BOOLEAN        _writingDB ;

      // thread specific error message buffer, aka SQLCA
      CHAR           *_pErrorBuff ;
      ossQueue<pmdEDUEvent> _bpEventQueue;
   #if defined ( _WINDOWS )
      HANDLE _threadHdl ;
   #elif defined ( _LINUX )
      OSSTID _threadHdl ;
      pthread_t       _threadID ;
   #endif
      monAppCB          _monApplCB ;
      monConfigCB       _monCfgCB ;

      CHAR              _Name [ PMD_EDU_NAME_LENGTH + 1 ] ;
      ossSocket        *_pClientSock ;

      std::set<SINT64>  _contextList ;

      // coord related variables
      CoordSession      *_pCoordSession;

      // transaction related variables
      UINT64               _beginLsn ;
      UINT64               _endLsn ;
      UINT32               _lsnNumber ;
      DPS_LSN_OFFSET       _curTransLSN;
      DPS_TRANS_ID         _curTransID;
      DpsTransCBLockList   _transLockLst;
      DpsTransNodeMap      *_pTransNodeMap;
      BOOLEAN              _isDoRollback;
      INT32                _transRC;

      // compression related variables
      CHAR                 *_pCompressionBuffer ;
      INT32                _compressionBufferSize ;
      CHAR                 *_pUncompressionBuffer ;
      INT32                _uncompressionBufferSize ;
      CHAR                 *_pTempCompBuffer ;
      INT32                _tempCompBufferSize ;

      void setStatus ( EDU_STATUS status )
      {
         _status = status ;
      }
      void setID ( EDUID id )
      {
         _eduID = id ;
      }
      CHAR *_getBuffInfo ( EDU_INFO_TYPE type, UINT32 &size ) ;

      // only allow pmdEDUMgr calling those private functions
      friend class _pmdEDUMgr ;
   public :
      void incEventCount ()
      {
         ++_processEventCount ;
      }
      void initMonAppCB()
      {
         _monApplCB.reset() ;
         if ( _monCfgCB.timestampON )
         {
            _monApplCB.recordConnectTimestamp() ;
         }
      }

      void setCoordSession( CoordSession *pSession )
      {
         _pCoordSession = pSession;
      }

      CoordSession *getCoordSession()
      {
         return _pCoordSession;
      }

      void resetMonAppCB()
      {
         _monApplCB.reset() ;
      }
      monConfigCB * getMonConfigCB()
      {
         return & _monCfgCB ;
      }
      ossEvent & getEvent ()
      {
         return _event ;
      }
      monAppCB * getMonAppCB()
      {
         return & _monApplCB ;
      }

      void interrupt () ;
      void disconnect () ;
      void force () ;
      BOOLEAN isInterrupted () ;
      BOOLEAN isDisconnected () ;
      BOOLEAN isForced () ;
      void resetInterrupt () ;
      void resetDisconnect () ;

      void writingDB( BOOLEAN writing ) { _writingDB = writing ; }
      BOOLEAN isWritingDB() const { return _writingDB ; }

      INT32 printInfo ( EDU_INFO_TYPE type, const CHAR *format, ... ) ;
      const CHAR *getInfo ( EDU_INFO_TYPE type ) ;
      void  resetInfo ( EDU_INFO_TYPE type ) ;

      void contextInsert ( SINT64 contextID )
      {
         EDUCB_XLOCK
         _contextList.insert ( contextID ) ;
      }
      void contextDelete ( SINT64 contextID )
      {
         EDUCB_XLOCK
         _contextList.erase ( contextID ) ;
      }
      SINT64 contextPeek () ;
      BOOLEAN contextFind ( SINT64 contextID )
      {
         EDUCB_SLOCK
         return _contextList.end() != _contextList.find(contextID) ;
      }
      UINT32 contextNum ()
      {
         EDUCB_SLOCK
         return _contextList.size() ;
      }
      void contextCopy ( std::set<SINT64> &contextList )
      {
         EDUCB_SLOCK
         contextList = _contextList ;
      }
      void setClientInfo ( const CHAR *clientName, UINT16 clientPort ) ;
      void setName ( const CHAR *name ) ;
      void setClientSock ( ossSocket *pSock ) ;
      ossSocket * getClientSock () ;

      ossSocket *getSocket()
      {
         return _pClientSock ;
      }

      EDUID getID ()
      {
         return _eduID ;
      }
      EDU_STATUS getStatus ()
      {
         return _status ;
      }
      _pmdEDUMgr *getEDUMgr ()
      {
         return _eduMgr ;
      }
      EDU_TYPES getType ()
      {
         return _eduType ;
      }
      void setType ( EDU_TYPES type )
      {
         SDB_ASSERT ( PMD_EDU_IDLE == _status,
                      "Type can't be changed during active" )
         _eduType = type ;
      }
      const char *getName ()
      {
         EDUCB_SLOCK
         return _Name ;
      }
      UINT64 getBeginLsn ()
      {
         return _beginLsn ;
      }
      UINT64 getEndLsn ()
      {
         return _endLsn ;
      }
      UINT32 getLsnCount ()
      {
         return _lsnNumber ;
      }
      void resetLsn ()
      {
         _beginLsn = ~0 ;
         _endLsn = ~0 ;
         _lsnNumber = 0 ;
      }
      void insertLsn ( UINT64 lsn )
      {
         if ( _beginLsn == (UINT64)~0 )
         {
            _beginLsn = lsn ;
         }

         _endLsn = lsn ;
         _lsnNumber++ ;
      }

   #if defined (_WINDOWS)
      HANDLE getThreadHandle()
   #elif defined (_LINUX )
      OSSTID getThreadHandle()
   #endif
      {
         return _threadHdl ;
      }

      void setThreadHdl
      (
   #if defined (_WINDOWS)
      HANDLE hdl
   #elif defined (_LINUX )
      OSSTID hdl
   #endif
      )
      {
         _threadHdl = hdl ;
      }

      void setTID ( UINT32 tid )
      {
         _tid = tid ;
      }

      UINT32 getTID ()
      {
         return _tid ;
      }
   #if defined (_LINUX)
      void setThreadID ( pthread_t id )
      {
         _threadID = id ;
      }
      pthread_t getThreadID ()
      {
         return _threadID ;
      }
   #endif

      void dumpInfo ( monEDUSimple &simple ) ;
      void dumpInfo ( monEDUFull &full ) ;

      void resetMon ()
      {
         _monApplCB.reset () ;
      }

      void postEvent ( pmdEDUEvent const &data )
      {
         // no need latch since _queue is already latched
         _queue.push ( data ) ;
      }

      BOOLEAN waitEvent ( pmdEDUEvent &data, INT64 millsec )
      {
         // no need latch since _queue is already latched
         // if millsec not 0, that means we want timeout
         // otherwise it's infinite wait

         BOOLEAN waitMsg   = FALSE ;
         _writingDB        = FALSE ;
         if ( PMD_EDU_IDLE != _status )
         {
            _status = PMD_EDU_WAITING ;
         }

         if ( 0 > millsec )
         {
            _queue.wait_and_pop ( data ) ;
            waitMsg = TRUE ;
         }
         else
         {
            waitMsg = _queue.timed_wait_and_pop ( data, millsec ) ;
         }

         if ( waitMsg )
         {
            ++_processEventCount ;
            if ( data._eventType == PMD_EDU_EVENT_TERM )
            {
               _ctrlFlag |= ( EDU_CTRL_DISCONNECTED|EDU_CTRL_INTERRUPTED );
            }
            else
            {
               _status = PMD_EDU_RUNNING ;
            }
         }

         return waitMsg ;
      }

      void clear ( );

      // transaction related
      void addBpEvents(pmdEDUEvent event);
      void clearBpEvents();
      void setTransID( DPS_TRANS_ID transID );
      DPS_TRANS_ID getTransID();
      void setCurTransLsn( DPS_LSN_OFFSET curLsn );
      DPS_LSN_OFFSET getCurTransLsn();
      dpsTransCBLockInfo *getTransLock( const dpsTransLockId &lockId );
      void addLockInfo( const dpsTransLockId &lockId, DPS_TRANSLOCK_TYPE lockType );
      void delLockInfo( const dpsTransLockId &lockId );
      DpsTransCBLockList *getLockList();
      INT32 createTransaction();
      void delTransaction();
      void addTransNode( MsgRouteID &routeID );
      void getTransNodeRouteID( UINT32 groupID, MsgRouteID &routeID );
      DpsTransNodeMap *getTransNodeLst();
      BOOLEAN isTransaction();
      BOOLEAN isTransNode( MsgRouteID &routeID );
      void startRollback();
      void stopRollback();
      void setTransRC( INT32 rc );
      INT32 getTransRC();
      void clearTransInfo();

      INT32 reallocCompressionBuffer ( INT32 requestedSize ) ;
      INT32 reallocUncompressionBuffer ( INT32 requestedSize ) ;
      INT32 compress ( const CHAR *pInputData, INT32 inputSize,
                       CHAR **ppData, INT32 *pDataSize ) ;
      INT32 compress ( const BSONObj &obj, const CHAR* pOIDPtr, INT32 oidLen,
                       CHAR **ppData, INT32 *pDataSize ) ;
      INT32 uncompress ( const CHAR *pInputData, INT32 inputSize,
                         CHAR **ppData, INT32 *pDataSize ) ;
   };
   typedef class _pmdEDUCB pmdEDUCB ;

   static OSS_THREAD_LOCAL _pmdEDUCB *__eduCB ;
   OSS_INLINE _pmdEDUCB *pmdGetEDUCB ()
   {
      return __eduCB ;
   }
   // this function must be called by the thread that want to create EDUCB
   OSS_INLINE _pmdEDUCB *pmdCreateEDUCB
         ( _pmdEDUMgr *mgr, EDU_TYPES type )
   {
      __eduCB = SDB_OSS_NEW _pmdEDUCB ( mgr, type ) ;
      return __eduCB ;
   }
   // this function must be called by the thread that want to delete EDUCB
   OSS_INLINE void pmdDeleteEDUCB ()
   {
      if ( __eduCB )
      {
         SDB_OSS_DEL __eduCB ;
         __eduCB = NULL ;
      }
   }
   OSS_INLINE _pmdEDUCB *pmdDeclareEDUCB ( _pmdEDUCB *p )
   {
      __eduCB = p ;
      return __eduCB ;
   }
   OSS_INLINE void pmdUndeclareEDUCB ()
   {
      __eduCB = NULL ;
   }
   typedef INT32 (*pmdEntryPoint)( pmdEDUCB *, void * ) ;

   pmdEntryPoint getEntryFuncByType ( EDU_TYPES type ) ;

   INT32 pmdAgentEntryPoint ( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdShardAgentEntryPoint ( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdReplAgentEntryPoint ( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdHTTPAgentEntryPoint ( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdTcpListenerEntryPoint ( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdHTTPListenerEntryPoint ( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdRepREntryPoint ( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdLoggWEntryPoint ( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdLoggNtyEntryPoint( pmdEDUCB *cb, void *arg ) ;
   INT32 pmdClusterEntryPoint ( pmdEDUCB *cb, void *pData ) ;
   INT32 pmdClusterShardEntryPoint ( pmdEDUCB *cb, void *pData ) ;
   INT32 pmdShardREntryPoint ( pmdEDUCB *cb, void *pData ) ;
   INT32 pmdCatMainControllerEntryPoint ( pmdEDUCB *cb, void *pData );
   INT32 pmdCatNodeManagerEntryPoint ( pmdEDUCB *cb, void *pData );
   INT32 pmdCatCatalogManagerEntryPoint ( pmdEDUCB *cb, void *pData );
   INT32 pmdCatNetWorkEntryPoint ( pmdEDUCB *cb, void *pData );
   INT32 pmdCoordNetWorkEntryPoint ( pmdEDUCB *cb, void *pData );
   INT32 pmdPreLoaderEntryPoint ( pmdEDUCB *cb, void *pData ) ;
   INT32 pmdBackgroundJobEntryPoint ( pmdEDUCB *cb, void *pData ) ;
   INT32 pmdDpsTransRollbackTaskEntryPoint( pmdEDUCB *cb, void *pData ) ;
#if defined (_WINDOWS)
   INT32 pmdWindowsListenerEntryPoint ( pmdEDUCB *cb, void *arg ) ;
#endif
   INT32 pmdEDUEntryPoint ( EDU_TYPES type, pmdEDUCB *cb, void *arg ) ;
   INT32 pmdEDUEntryPointWrapper ( EDU_TYPES type, pmdEDUCB *cb, void *arg ) ;

   INT32 pmdRecv ( CHAR *pBuffer, INT32 recvSize,
                   ossSocket *sock, pmdEDUCB *cb ) ;
   INT32 pmdSend ( const CHAR *pBuffer, INT32 sendSize,
                   ossSocket *sock, pmdEDUCB *cb ) ;
   INT32 pmdLoadWorkerEntryPoint ( pmdEDUCB *cb, void *pData ) ;
#if defined (_LINUX)
   void pmdEDUCodeTrapHandler( OSS_HANDPARMS ) ;
#endif
   INT32 pmdSyncClockEntryPoint( pmdEDUCB *cb, void *arg ) ;
}

#endif // PMDEDU_HPP__
