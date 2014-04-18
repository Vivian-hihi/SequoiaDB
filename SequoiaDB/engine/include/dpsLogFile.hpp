/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossTypes.hpp

   Descriptive Name = Operating System Services Types Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSLOGFILE_H_
#define DPSLOGFILE_H_

#include "core.hpp"
#include "oss.hpp"
#include "ossIO.hpp"
#include "ossUtil.hpp"
#include "dpsLogDef.hpp"

namespace engine
{
#define DPS_LOG_HEADER_EYECATCHER "SDBLOGHD"
#define DPS_LOG_HEADER_EYECATCHER_LEN 8

#define DPS_INVALID_LOG_FILE_ID     0xFFFFFFFF

   class _dpsLogHeader : public SDBObject
   {
   public :
      CHAR _eyeCatcher [ DPS_LOG_HEADER_EYECATCHER_LEN ] ;
      DPS_LSN _firstLSN ;
      UINT32 _logID ;
      CHAR _padding [ DPS_LOG_HEAD_LEN -
                      DPS_LOG_HEADER_EYECATCHER_LEN - // _eyeCatcher
                      sizeof(DPS_LSN) -               // _firstLSN
                      sizeof(UINT32)                  // _logID
                    ] ;

      _dpsLogHeader ()
      {
         ossMemcpy(_eyeCatcher, DPS_LOG_HEADER_EYECATCHER, 
            DPS_LOG_HEADER_EYECATCHER_LEN) ;
         _logID = DPS_INVALID_LOG_FILE_ID ;
         ossMemset(_padding, 0, sizeof(_padding)) ;
      }
   } ;
   typedef class _dpsLogHeader dpsLogHeader ;
   class _dpsLogFile : public SDBObject
   {
   private:
      _OSS_FILE *_file ;
      // 32 bit size, so we can support up to 4GB file size
      UINT32 _fileSize ;
      UINT32 _idleSize ;
      dpsLogHeader _logHeader ;
   public:
      _dpsLogFile();

      ~_dpsLogFile();

   public:
      OSS_INLINE UINT32 size()
      {
         return _fileSize;
      }

      OSS_INLINE dpsLogHeader &header()
      {
         return _logHeader ;
      }

      OSS_INLINE void idleSize( UINT32 size )
      {
         _idleSize = size ;
      }
   public:
      // initialize file
      INT32 init ( const CHAR *path, UINT32 fileSize ) ;
      // write into file
      INT32 write ( const CHAR *content, UINT32 len ) ;
      // read from file
      INT32 read ( const DPS_LSN_OFFSET &lOffset, UINT32 len, CHAR *buf ) ;
      // close the file
      INT32 close();
      // get how much space we still able to write
      UINT32 getIdleSize() { return _idleSize ; }
      UINT32 getLength () { return _fileSize - _idleSize ; }
      // reset metadata
      INT32 reset ( UINT32 logID, const DPS_LSN_OFFSET &offset,
                    const DPS_LSN_VER &version ) ;
      // get first lsn
      DPS_LSN getFirstLSN ( BOOLEAN mustExist = TRUE ) ;

   private:
      void _initHead( UINT32 logID )
      {
         ossMemcpy ( &_logHeader._eyeCatcher, DPS_LOG_HEADER_EYECATCHER,
                     DPS_LOG_HEADER_EYECATCHER_LEN ) ;
         _logHeader._logID = logID ;
      }
      // flush log file header
      INT32 _flushHeader() ;
      // restore header
      INT32 _readHeader () ;
      // restore from file
      INT32 _restore () ;
   };
   typedef class _dpsLogFile dpsLogFile;
}

#endif

