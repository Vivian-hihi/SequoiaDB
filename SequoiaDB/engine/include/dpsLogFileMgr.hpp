/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsLogFileMgr.hpp

   Descriptive Name = Data Protection Services Log File Manager Header

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains declare for log file manager

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/27/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSLOGFILEMGR_H_
#define DPSLOGFILEMGR_H_

#include "core.hpp"
#include "oss.hpp"
#include "dpsLogFile.hpp"
#include "pmdOptionsMgr.hpp"
#include <vector>
using namespace std;

namespace engine
{
   #define DPS_MAX_LOG_FILE_NUM     1000
   #define DPS_LOG_FILE_SIZE_UNIT   (1024 * 1024)
   // max log file size 2GB
   #define DPS_MAX_LOG_FILE_SIZE ((UINT64)PMD_MAX_LOG_FILE_SZ*DPS_LOG_FILE_SIZE_UNIT)
   // min log file size 32MB
   #define DPS_MIN_LOG_FILE_SIZE ((UINT64)PMD_MIN_LOG_FILE_SZ*DPS_LOG_FILE_SIZE_UNIT)
   // default log file size
   #define DPS_DFT_LOG_FILE_SIZE ((UINT64)PMD_DFT_LOG_FILE_SZ*DPS_LOG_FILE_SIZE_UNIT)
   #define DPS_DFT_LOG_FILE_NUM  PMD_DFT_LOG_FILE_NUM

   class _dpsLogPage;
   class _dpsMessageBlock;
   class _dpsReplicaLogMgr ;
   class _dpsLogFileMgr : public SDBObject
   {
   private:
      vector<_dpsLogFile *> _files;
      UINT32 _work;
      UINT32 _logicalWork ;
      UINT32 _begin ;

      UINT32 _logFileSz ;
      UINT32 _logFileNum ;
      _dpsReplicaLogMgr *_replMgr ;

   public:
      _dpsLogFileMgr( class _dpsReplicaLogMgr *replMgr );
      ~_dpsLogFileMgr();

      INT32 init( const CHAR *path );

      INT32 flush( _dpsMessageBlock *mb,
                   const DPS_LSN &beginLsn,
                   BOOLEAN shutdown = FALSE );

      INT32 load( const DPS_LSN &lsn, _dpsMessageBlock *mb,
                  BOOLEAN onlyHeader = FALSE ) ;

      INT32 move( const DPS_LSN_OFFSET &offset, const DPS_LSN_VER &version ) ;

      void setLogFileSz ( UINT32 logFileSz )
      {
         UINT64 fileSize = DPS_LOG_FILE_SIZE_UNIT * (UINT64)logFileSz ;
         if ( fileSize > DPS_MAX_LOG_FILE_SIZE )
            fileSize = DPS_MAX_LOG_FILE_SIZE ;
         if ( fileSize < DPS_MIN_LOG_FILE_SIZE )
            fileSize = DPS_MIN_LOG_FILE_SIZE ;
         _logFileSz = fileSize ;
      }

      DPS_LSN getStartLSN ( BOOLEAN mustExist = TRUE ) ;

      UINT32 getLogFileSz ()
      {
         return _logFileSz ;
      }
      void setLogFileNum ( UINT32 logFileNum )
      {
         _logFileNum = logFileNum ;
      }
      UINT32 getLogFileNum ()
      {
         return _logFileNum ;
      }
      _dpsLogFile * getWorkLogFile()
      {
         return _files[_work] ;
      }

   protected:
      void _analysis () ;
      UINT32 _incFileID ( UINT32 fileID ) ;
      UINT32 _decFileID ( UINT32 fileID ) ;
      void   _incLogicalFileID () ;

   };
   typedef class _dpsLogFileMgr dpsLogFileMgr;
}


#endif

