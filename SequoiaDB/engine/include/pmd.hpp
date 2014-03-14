/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmd.hpp

   Descriptive Name = Process MoDel Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains structure kernel control block,
   which is the most critical data structure in the engine process.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMD_HPP__
#define PMD_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossIO.hpp"
#include "ossUtil.hpp"
#include "ossLatch.hpp"
#include "pmdEDUMgr.hpp"
#include "pd.hpp"
#include "monCB.hpp"
#include "pmdOptionsMgr.hpp"
#include "msg.h"
#include "msgDef.hpp"

extern PDLEVEL    _curPDLevel ;
extern CHAR       _pdDiagLogPath [ OSS_MAX_PATHSIZE + 1 ] ;

class _pdTraceCB ;

namespace engine
{
   #define PMD_DFT_CONF     "sdb.conf"
   #define PMD_DFT_CAT      "sdb.cat"
   #define PMD_DFT_DIAGLOG  "sdbdiag.log"

#if defined (_LINUX)
   #define PMD_ENGINE_NAME_BUF_LEN OSS_RENAME_PROCESS_BUFFER_LEN
   // no space between sequoiadb and (, because our sdbstart is relying on
   // /proc/<pid>/stat to check startup progress. It will fetch first 4 records
   // for pid/procname/status/ppid, so if we have space between sequoiadb and (,
   // we'll break the logic there
   #define PMD_ENGINE_NAME_PATTERN "sequoiadb(%s)"
#endif
   #define PMD_CURRENT_PATH "./"

   #define PMD_DFT_SVCPORT  OSS_DFT_SVCPORT
   // by default it's service port + 1
   #define PMD_REPL_PORT    1
   // by default it's service port + 2
   #define PMD_SHARD_PORT   2
   // by default it's service port + 3
   #define PMD_CAT_PORT     3
   // by default it's service port + 4
   #define PMD_REST_PORT    4

   // database status information
   #define PMD_DB_NORMAL        0
   #define PMD_DB_SHUTDOWN      1
   #define PMD_DB_PANIC         2
   #define PMD_DB_FREEZE        3
   #define PMD_IS_DB_NORMAL     ( PMD_DB_NORMAL == pmdGetKRCB()->getDBStatus() )
   #define PMD_IS_DB_DOWN       ( PMD_DB_SHUTDOWN ==  \
                                  pmdGetKRCB()->getDBStatus() || \
                                  PMD_DB_PANIC == pmdGetKRCB()->getDBStatus() )
   #define PMD_IS_DB_FREEZE     ( PMD_DB_FREEZE == pmdGetKRCB()->getDBStatus() )
   #define PMD_IS_DB_UP         ( !PMD_IS_DB_DOWN )

   #define PMD_FREEZE_DB        { pmdGetKRCB()->setDBStatus(PMD_DB_FREEZE) ; }
   #define PMD_UNFREEZE_DB      { pmdGetKRCB()->setDBStatus(PMD_DB_NORMAL) ; }

   #define PMD_SHUTDOWN_DB(code)  \
      do { \
         pmdGetKRCB()->setDBStatus( PMD_DB_SHUTDOWN ) ; \
         pmdGetKRCB()->setExitCode( code ) ; \
      } while ( 0 );

   // database flag
   // access flag, whenever database opens a file, need to check for this bit
   // in order to use the right open mode
   // also need to check this status whenever write activity happen
   // a database must be readable in order to operate, but it can be 
   // "readonly" db when the media/path is not writable
   #define PMD_DBFLAG_WRITEABLE     0x00000001


   #define PMD_IS_FLAG_SET(x)      ( x & pmdGetKRCB()->getDBFlag() )
   #define PMD_IS_FLAG_NOT_SET(x)  ( !PMD_IS_FLAG_SET(x) )
   #define PMD_IS_DB_READONLY      ( PMD_IS_FLAG_NOT_SET(PMD_DBFLAG_WRITEABLE) )
   #define PMD_IS_DB_READWRITE     ( PMD_IS_FLAG_SET(PMD_DBFLAG_WRITEABLE) )

   enum SDB_START_TYPE
   {
      SDB_START_NORMAL  = 0,
      SDB_START_CRASH
   };

   class _clsMgr ;
   class _clsReplicateSet ;
   class _clsShardMgr ;
   class _dpsLogWrapper ;
   class dpsTransCB ;
   class _SDB_DMSCB ;
   class _SDB_RTNCB ;
   class _bpsCB ;
   class sdbCatalogueCB ;
   class _CoordCB ;
   class _sqlCB ;
   class _authCB ;
   class aggrBuilder ;
   class _spdFMPMgr ;

   /*
    * Kernel Control Block
    * Database Kernel Variables
    */
   class _SDB_KRCB : public SDBObject
   {
   #ifdef KRCB_XLOCK
   #undef KRCB_XLOCK
   #endif
   #define KRCB_XLOCK ossScopedLock _lock(&_mutex, EXCLUSIVE);
   #ifdef KRCB_SLOCK
   #undef KRCB_SLOCK
   #endif
   #define KRCB_SLOCK ossScopedLock _lock(&_mutex, SHARED) ;
   private :
      // configured options
      CHAR           _diagLogFile  [ OSS_MAX_PATHSIZE + 1 ] ;
      CHAR           _confFile     [ OSS_MAX_PATHSIZE + 1 ] ;
      CHAR           _catFile      [ OSS_MAX_PATHSIZE + 1 ] ;
      CHAR           _groupName    [ OSS_MAX_GROUPNAME_SIZE + 1 ] ;
      SDB_ROLE       _role ;
      SDB_START_TYPE _startType ;

      UINT32         _dbStatus ;
      UINT32         _dbFlag ;

      BOOLEAN        _businessOK ;
      INT32          _exitCode ;

      ossSpinSLatch  _mutex ;
      _pmdEDUMgr     _eduMgr ;

      _clsMgr        *_clsCB ;
      dpsTransCB     *_dpsTransCB ;
      _dpsLogWrapper *_dpscb ;
      _SDB_DMSCB     *_dmscb ;
      _SDB_RTNCB     *_rtncb ;
      _bpsCB         *_bpscb ;
      sdbCatalogueCB *_catlogueCB;
      _CoordCB       *_coordcb;
      _sqlCB         *_sql ;
      _authCB        *_auth ;
      _pdTraceCB     *_traceCB ;
      aggrBuilder    *_aggrCB;
      _spdFMPMgr     *_fmpCB ;

      monConfigCB    _monCfgCB ;
      monDBCB        _monDBCB ;
      _pmdOptionsMgr _optioncb ;
      ossTick        _curTime ;

   public :

      SDB_START_TYPE getStartType () const
      {
         return _startType ;
      }
      void setStartType ( SDB_START_TYPE startType )
      {
         _startType = startType ;
      }
      const CHAR* getDiagLogPath () const
      {
         return _optioncb.krcbDiagLogPath() ;
      }
      CHAR *getDiagLogPath ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, getDiagLogPath(), size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getLogPath () const
      {
         return _optioncb.krcbLogPath() ;
      }
      CHAR *getLogPath ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, getLogPath(), size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getDiagLogFile () const
      {
         return _diagLogFile ;
      }
      CHAR *getDiagLogFile ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, _diagLogFile, size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getConfPath () const
      {
         return _optioncb.krcbConfPath() ;
      }
      CHAR *getConfPath ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, getConfPath(), size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getConfFile () const
      {
         return _confFile ;
      }
      CHAR *getConfFile ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, _confFile, size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getCatFile () const
      {
         return _catFile ;
      }
      CHAR *getCatFile ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, _catFile, size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getDBPath () const
      {
         return _optioncb.krcbDbPath() ;
      }
      CHAR *getDBPath ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, getDBPath(), size  ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getIndexPath () const
      {
         return _optioncb.krcbIndexPath() ;
      }
      CHAR *getIndexPath ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, getIndexPath(), size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getBkupPath () const
      {
         return _optioncb.krcbBkupPath() ;
      }
      CHAR *getBkupPath ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, getBkupPath(), size  ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getRestAddr () const
      {
         return _optioncb.restService() ;
      }
      CHAR *getRestAddr ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, getRestAddr(), size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      UINT32 getDBStatus () const
      {
         return _dbStatus ;
      }
      UINT32 getDBFlag () const
      {
         return _dbFlag ;
      }
      SDB_ROLE getDBRole () const
      {
         return _role ;
      }
      UINT32 getMaxPooledEDU () const
      {
         return _optioncb.krcbMaxPool() ;
      }
      pmdEDUMgr *getEDUMgr ()
      {
         return &_eduMgr ;
      }
      UINT16 getServicePort () const
      {
         return _optioncb.krcbSvcPort() ;
      }
      const CHAR *getServiceAddr () const
      {
         return _optioncb.krcbService() ;
      }
      CHAR *getServiceAddr ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL ;
         ossStrncpy ( pBuffer, getServiceAddr(), size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      const CHAR *getGroupName () const
      {
         return _groupName ;
      }
      CHAR *getGroupName ( CHAR *pBuffer, UINT32 size ) const
      {
         if ( !pBuffer || 0 == size )
            return NULL;
         ossStrncpy ( pBuffer, _groupName, size ) ;
         pBuffer[ size - 1 ] = 0 ;
         return pBuffer ;
      }
      UINT32 getLogBufSize () const
      {
         return _optioncb.logBuffSize() ;
      }

      UINT32 getSortBufSize() const
      {
         return _optioncb.sortBufSize() ;
      }

      UINT32 getHjBufSize() const
      {
         return _optioncb.hjBufSize() ;
      }

      const CHAR *getTmpPath() const
      {
         return _optioncb.dmsTmpPath() ;
      }

      inline _SDB_DMSCB *getDMSCB ()
      {
         return _dmscb ;
      }
      inline _SDB_RTNCB *getRTNCB ()
      {
         return _rtncb ;
      }

      inline monConfigCB * getMonCB()
      {
         return & _monCfgCB ;
      }

      inline monDBCB * getMonDBCB ()
      {
         return &_monDBCB ;
      }
      inline _clsMgr *getClsCB ()
      {
         return _clsCB ;
      }
      _clsReplicateSet *getReplCB () ;
      _clsShardMgr *getShardCB () ;

      inline _dpsLogWrapper* getDPSCB ()
      {
         return _dpscb ;
      }
      inline _bpsCB *getBPSCB ()
      {
         return _bpscb ;
      }
      inline _pdTraceCB *getTraceCB ()
      {
         return _traceCB ;
      }

      UINT32 getLogFileSz () ;
      UINT32 getLogFileNum () ;

      inline _pmdOptionsMgr *getOptionCB()
      {
         return &_optioncb;
      }
      inline sdbCatalogueCB *getCATLOGUECB()
      {
         return _catlogueCB;
      }

      inline _CoordCB *getCoordCB()
      {
         return _coordcb ;
      }

      inline _sqlCB *getSqlCB()
      {
         return _sql ;
      }

      inline _authCB *getAuthCB()
      {
         return _auth ;
      }

      inline dpsTransCB *getTransCB()
      {
         return _dpsTransCB;
      }

      inline aggrBuilder *getAggrCB()
      {
         return _aggrCB;
      }

      inline _spdFMPMgr *getFMPCB()
      {
         return _fmpCB ;
      }

      INT32 isBusinessOK() const
      {
         return _businessOK ;
      }
      INT32 getExitCode() const
      {
         return _exitCode ;
      }
      void setExitCode( INT32 exitCode )
      {
         if ( exitCode && _exitCode )
         {
            return ;
         }
         _exitCode = exitCode ;
      }
      void setBusinessOK( BOOLEAN businessOK )
      {
         _businessOK = businessOK ;
      }
      void enforceDiagLevel ( PDLEVEL level )
      {
         _curPDLevel = level ;
      }

      INT32 enforceDiagLogPath () ;

      INT32 enforceConfPath () ;

      void setDBStatus ( UINT32 status )
      {
         _dbStatus = status ;
      }

      void setDBFlag ( UINT32 addedFlag )
      {
         _dbFlag |= addedFlag ;
      }
      void enforceDBRole ( SDB_ROLE role )
      {
         _role = role ;
      }
      void unsetDBFlag ( UINT32 removedFlag )
      {
         _dbFlag &= ~removedFlag ;
      }
      void replaceDBFlag ( UINT32 newFlag )
      {
         _dbFlag = newFlag ;
      }
      void setMonCB( monConfigCB & monCB )
      {
         _monCfgCB = monCB ;
      }

      void setMonDBCB ( monDBCB & cb )
      {
         _monDBCB = cb ;
      }

      void setMonTimestampSwitch( BOOLEAN flag )
      {
          _monCfgCB.timestampON = flag ;
      }

      void enforceNodeInfo ( const _MsgRouteID &id , const CHAR * host ) ;
      void enforceReplAddr( UINT16 serviceID, const CHAR *replService ) ;
      void enforceShardAddr ( UINT16 serviceID, const CHAR *shardService ) ;
      void setGroupName ( const CHAR *groupName )
      {
         ossMemset( _groupName, 0, sizeof(_groupName) ) ;
         ossStrncpy ( _groupName, groupName, sizeof(_groupName)-1 );
      }
      void enforceCatAddr( const _MsgRouteID &id,
                           const CHAR *host,
                           const CHAR *service ) ;
      void updateCatRouteID ( const _MsgRouteID &id ) ;

      void enforceCataLogGrpAddrs( const _MsgRouteID &id,
                                   const CHAR *host,
                                   const CHAR *service ) ;

      void enforceLogFileSz ( UINT32 logFileSz ) ;
      void enforceLogFileNum ( UINT32 logFileNum ) ;

      ossTick getCurTime() ;
      void syncCurTime() ;

   public:
      _SDB_KRCB () ;
      ~_SDB_KRCB () ;

      INT32 init () ;

      void destroy () ;

   } ;
   typedef class _SDB_KRCB pmdKRCB ;

   /*
    * Get global kernel control block
    * This variable is unique per process
    */
   extern pmdKRCB pmd_krcb ;
   inline pmdKRCB* pmdGetKRCB()
   {
      return &pmd_krcb ;
   }
}

#endif //PMD_HPP__

