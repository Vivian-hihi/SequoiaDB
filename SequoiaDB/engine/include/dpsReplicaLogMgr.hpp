/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsReplicaLogMgr.hpp

   Descriptive Name = Data Protection Services Replica Log Manager Header

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains code logic for replica manager

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/27/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSREPLICALOGMGR_H_
#define DPSREPLICALOGMGR_H_


#include "core.hpp"
#include "oss.hpp"
#include "dpsLogPage.hpp"
#include "ossQueue.hpp"
#include "ossLatch.hpp"
#include "dpsMergeBlock.hpp"
#include "ossAtomic.hpp"
// #include "dpsLogIndex.hpp"
#include "dpsLogFileMgr.hpp"
#include "ossUtil.hpp"
#include "ossEvent.hpp"

namespace engine
{
   // we ALWAYS search for MEM first, because we may have LSN stay in buffer but
   // not on disk
   const UINT8 DPS_SEARCH_MEM  = 0 ;
   // indicating also search in file
   const UINT8 DPS_SEARCH_FILE = 1 ;

   class _pmdEDUCB ;

   class _dpsReplicaLogMgr : public SDBObject
   {
   private:
      ossQueue<_dpsLogPage *> _queue;
      //_dpsLogIndex _index;
      _dpsLogFileMgr _logger;
      _dpsLogPage *_pages;
      _ossSpinXLatch _mtx;
      _ossAtomic32 _idleSize;
      DPS_LSN _lsn;
      DPS_LSN _currentLsn;
      UINT32 _totalSize;
      UINT32 _waterMark;
      UINT32 _work;
      UINT32 _begin ;
      BOOLEAN _rollFlag ;
      UINT32 _pageNum;
      BOOLEAN _restoreFlag ;
      ossAutoEvent _allocateEvent ;

   public:
      _dpsReplicaLogMgr();

      ~_dpsReplicaLogMgr();

   public:
      inline UINT32 idleSize()
      {
         return _idleSize.peek();
      }

      inline DPS_LSN expectLsn()
      {
         DPS_LSN lsn ;
         _mtx.get();
         lsn = _lsn;
         _mtx.release();
         return lsn;
      }

      inline DPS_LSN currentLsn()
      {
         DPS_LSN lsn ;
         _mtx.get();
         lsn = _currentLsn;
         _mtx.release();
         return lsn;
      }

      inline DPS_LSN_VER incVersion()
      {
         DPS_LSN_VER version = DPS_INVALID_LSN_VERSION ;
         _mtx.get();
         version = ++_lsn.version ;
         _mtx.release();
         return version ;
      }

   public:
      DPS_LSN getStartLsn ( BOOLEAN logBufOnly ) ;
      void getLsnWindow( DPS_LSN &fileBeginLsn,
                         DPS_LSN &memBeginLsn,
                         DPS_LSN &endLsn ) ;

      void getLsnWindow( DPS_LSN &fileBeginLsn,
                         DPS_LSN &memBeginLsn,
                         DPS_LSN &endLsn,
                         DPS_LSN &expected ) ;

      INT32 init( const CHAR *path, UINT32 pageNum );

      INT32 merge( _dpsMergeBlock &block ) ;
      //INT32 merge( _dpsMergeBlock &block, DPS_LSN &lsn );

      // first step: allocate pages and product lsn
      INT32 preparePages ( dpsMergeInfo &info ) ;
      // secondary step: write data to pages
      void  writeData ( dpsMergeInfo &info ) ;

      INT32 search( const DPS_LSN &minLsn, _dpsMessageBlock *mb,
                    UINT8 type, BOOLEAN onlyHeader );
      INT32 run( _pmdEDUCB *cb );
      INT32 tearDown();
      INT32 flushAll() ;

      /// any other interfaces should not be called
      /// when this interface is beging called.
      INT32 move( const DPS_LSN_OFFSET &offset,
                  const DPS_LSN_VER &version ) ;

      void setLogFileSz ( UINT32 logFileSz )
      {
         _logger.setLogFileSz ( logFileSz ) ;
      }
      UINT32 getLogFileSz ()
      {
         return _logger.getLogFileSz () ;
      }
      void setLogFileNum ( UINT32 logFileNum )
      {
         _logger.setLogFileNum ( logFileNum ) ;
      }
      UINT32 getLogFileNum ()
      {
         return _logger.getLogFileNum () ;
      }

      BOOLEAN isInRestore ()
      {
         return _restoreFlag;
      }

   private:
      void _allocate( UINT32 len,
                      dpsPageMeta &allocated ) ;
      void _push2SendQueue( const dpsPageMeta &allocated );
      void _mergeLogs( _dpsMergeBlock &block,
                       const dpsPageMeta &meta ) ;
      void _mergePage( const CHAR *src,
                       UINT32 len,
                       UINT32 &workSub,
                       UINT32 &offset );
      INT32 _flushPage( _dpsLogPage *page, BOOLEAN shutdown = FALSE );
      INT32 _flushAll() ;
      INT32 _search ( const DPS_LSN &lsn, _dpsMessageBlock *mb,
                      BOOLEAN onlyHeader ) ;
      DPS_LSN _getStartLsn () ;
      INT32 _parse( UINT32 sub, UINT32 offset, UINT32 len, CHAR *out ) ;

      INT32  _movePages ( const DPS_LSN_OFFSET &offset,
                          const DPS_LSN_VER &version ) ;
      INT32 _restore () ;

      UINT32 _decPageID ( UINT32 pageID )
      {
         return pageID ? pageID - 1 : _pageNum - 1 ;
      }
      UINT32 _incPageID ( UINT32 pageID )
      {
         ++pageID ;
         return pageID >= _pageNum ? 0 : pageID ;
      }
   };
   typedef class _dpsReplicaLogMgr dpsReplicaLogMgr;
}


#endif
