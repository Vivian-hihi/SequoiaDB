/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsLogWrapper.hpp

   Descriptive Name = Data Protection Services Log Wrapper Header

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains declare for dpsLogWrapper.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/27/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSLOGWRAPPER_H_
#define DPSLOGWRAPPER_H_

#include <vector>

#include "core.hpp"
#include "oss.hpp"
#include "dpsReplicaLogMgr.hpp"
#include "../bson/bsonelement.h"
#include "../bson/bsonobj.h"

using namespace bson;

namespace engine
{
#define DPS_DFT_LOG_BUF_SZ 1024

   class _pmdEDUCB ;

   class _dpsLogWrapper : public SDBObject
   {
   private:
      _dpsReplicaLogMgr _buf;
      BOOLEAN _initialized ;
      BOOLEAN _dpslocal ;

   public:
      _dpsLogWrapper();
      ~_dpsLogWrapper();
   public:
      inline void setLogLocal( BOOLEAN dpslocal )
      {
         _dpslocal = dpslocal ;
      }

      inline BOOLEAN isLogLocal()
      {
         return _dpslocal ;
      }

      inline INT32 init( const CHAR *path, UINT32 pageNum = DPS_DFT_LOG_BUF_SZ )
      {
         INT32 rc = _buf.init( path, pageNum ) ;
         if ( SDB_OK == rc )
            _initialized = TRUE ;
         return rc ;
      }

      inline INT32 search( const DPS_LSN &minLsn, _dpsMessageBlock *mb,
                           UINT8 type = DPS_SEARCH_MEM | DPS_SEARCH_FILE )
      {
         SDB_ASSERT ( _initialized, "shouldn't call search without init" )
         return _buf.search( minLsn, mb, type );
      }

      inline INT32 run( _pmdEDUCB *cb )
      {
         if ( !_initialized )
            return SDB_OK ;
         return _buf.run( cb );
      }

      inline INT32 tearDown()
      {
         if ( !_initialized )
            return SDB_OK ;
         return _buf.tearDown();
      }

      inline BOOLEAN doLog ()
      {
         return _initialized ;
      }

      // note flushAll function is ONLY USED IN TESTCASE
      // engine should NEVER call flushAll in any situation.
      // The log write thread supposed to call run() in order to flush dirty
      // pages once at a time
      inline INT32 flushAll()
      {
         SDB_ASSERT ( _initialized, "shouldn't call flushAll without init" )
         return _buf.flushAll() ;
      }

      inline DPS_LSN getStartLsn ( BOOLEAN logBufOnly = FALSE )
      {
         if ( !_initialized )
         {
            DPS_LSN lsn ;
            return lsn ;
         }
         return _buf.getStartLsn ( logBufOnly ) ;
      }

      inline DPS_LSN  getCurrentLsn()
      {
         return _buf.currentLsn() ;
      }

      inline void getLsnWindow( DPS_LSN &fileBeginLsn,
                                DPS_LSN &memBeginLsn,
                                DPS_LSN &endLsn,
                                DPS_LSN *pExpectLsn = NULL )
      {
         if ( !_initialized )
         {
            return ;
         }

         if ( pExpectLsn )
         {
            _buf.getLsnWindow( fileBeginLsn, memBeginLsn, endLsn, *pExpectLsn ) ;
         }
         else
         {
            _buf.getLsnWindow( fileBeginLsn, memBeginLsn, endLsn ) ;
         }
      }

      inline void getLsnWindow( DPS_LSN &fileBeginLsn,
                                DPS_LSN &memBeginLsn,
                                DPS_LSN &endLsn,
                                DPS_LSN &expected )
      {
         if ( !_initialized )
            return ;
         _buf.getLsnWindow( fileBeginLsn,
                            memBeginLsn,
                            endLsn,
                            expected ) ;
      }

      inline DPS_LSN expectLsn()
      {
         if ( !_initialized )
         {
            DPS_LSN lsn ;
            return lsn ;
         }
         return _buf.expectLsn() ;
      }

      inline DPS_LSN_VER incVersion()
      {
         return _buf.incVersion() ;
      }

      inline INT32 move( const DPS_LSN_OFFSET &offset,
                         const DPS_LSN_VER &version )
      {
         return _buf.move( offset, version ) ;
      }
   public:
      void  writeData ( dpsMergeInfo &info ) ;

/*
      INT32 recordInsert( const CHAR *csName,
                          const CHAR *clName,
                          const BSONObj &obj,
                          const DPS_TRANS_ID &transID,
                          const DPS_LSN_OFFSET &preTransLsn,
                          dpsMergeInfo &info ) ;

      INT32 recordUpdate( const CHAR *csName,
                          const CHAR *clName,
                          const BSONObj &oldMatch,
                          const BSONObj &oldObj,
                          const BSONObj &newMatch,
                          const BSONObj &newObj,
                          const DPS_TRANS_ID &transID,
                          const DPS_LSN_OFFSET &preTransLsn,
                          dpsMergeInfo &info ) ;

      INT32 recordDelete( const CHAR *csName,
                          const CHAR *clName,
                          const BSONObj &oldObj,
                          const DPS_TRANS_ID &transID,
                          const DPS_LSN_OFFSET &preTransLsn,
                          dpsMergeInfo &info ) ;

      INT32 recordCScrt( const CHAR *csName, const INT32 &pageSize,
                         dpsMergeInfo &info );

      INT32 recordCSdel( const CHAR *csName, dpsMergeInfo &info );

      INT32 recordCLcrt( const CHAR *csName,
                         const CHAR *clName,
                         dpsMergeInfo &info ) ;

      INT32 recordCLdel( const CHAR *csName,
                         const CHAR *clName,
                         dpsMergeInfo &info ) ;

      INT32 recordIXcrt ( const CHAR *csName,
                          const CHAR *clName,
                          const BSONObj &index,
                          dpsMergeInfo &info ) ;

      INT32 recordIXdel ( const CHAR *csName,
                          const CHAR *clName,
                          const BSONObj &index,
                          dpsMergeInfo &info ) ;

      INT32 recordCLrename ( const CHAR *csName,
                             const CHAR *clOldName,
                             const CHAR *clNewName,
                             dpsMergeInfo &info ) ;

      INT32 recordCLtrunc ( const CHAR *csName,
                            const CHAR *clName,
                            dpsMergeInfo &info ) ;

      INT32 recordTransCommit( const DPS_TRANS_ID &transID,
                           dpsMergeInfo &info );

      INT32 recordTransRollback( const DPS_TRANS_ID &transID,
                                 const DPS_LSN_OFFSET &preTransLsn,
                                 dpsMergeInfo &info );

      INT32 recordInvalidateCata( const CHAR *clFullName,
                                  dpsMergeInfo &info ) ;
*/

      INT32 recordRow( const CHAR *row, UINT32 len );

      INT32 prepare( dpsMergeInfo &info ) ;

      void setLogFileSz ( UINT32 logFileSz )
      {
         _buf.setLogFileSz ( logFileSz ) ;
      }
      UINT32 getLogFileSz ()
      {
         return _buf.getLogFileSz () ;
      }
      void setLogFileNum ( UINT32 logFileNum )
      {
         _buf.setLogFileNum ( logFileNum ) ;
      }
      UINT32 getLogFileNum ()
      {
         return _buf.getLogFileNum () ;
      }
      BOOLEAN isInRestore();
   };
   typedef class _dpsLogWrapper SDB_DPSCB ;
}

#endif
