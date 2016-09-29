/*******************************************************************************

   Copyright (C) 2011-2015 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = rplReplayer.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          18/9/2016  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "rplReplayer.hpp"
#include "pd.hpp"
#include "dpsArchiveFile.hpp"
#include "dpsOp2Record.hpp"
#include "oss.h"
#include "ossEDU.hpp"
#include "../bson/bsonobj.h"
#include <sstream>
#include <iostream>

using namespace engine;
using namespace sdbclient;

namespace replay
{
   #define RPL_WATCH_INTERVAL (10 * 1000) // seconds

   BOOLEAN _isRunning = FALSE;

   static void _stop(INT32 sigNum)
   {
      if (0 != sigNum)
      {
         PD_LOG(PDEVENT, "Recieved signal[%d], stop...", sigNum);
      }

      _isRunning = FALSE;
   }

   static INT32 _regSignalHandler()
   {
      INT32 rc = SDB_OK;
#if defined (_LINUX)
      ossSigSet sigSet ;
      sigSet.sigAdd(SIGHUP);
      sigSet.sigAdd(SIGINT);
      sigSet.sigAdd(SIGTERM);
      sigSet.sigAdd(SIGPWR);
      rc = ossRegisterSignalHandle(sigSet, (SIG_HANDLE)(&_stop));
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to register signals, rc=%d",
                rc);
         goto error;
      }

   done:
      return rc ;
   error:
      goto done ;
#else
      return rc ;
#endif
   }

   Replayer::Replayer()
   {
      _options = NULL;
      _buf = NULL;
      _bufSize = 0;
   }

   Replayer::~Replayer()
   {
      _sdb.disconnect();

      if (!_tmpFile.empty())
      {
         ossFile::deleteFile(_tmpFile);
         _tmpFile = "";
      }

      if (_bufSize > 0)
      {
         SAFE_OSS_FREE(_buf);
         _bufSize = 0;
      }
   }

   INT32 Replayer::init(Options& options)
   {
      INT32 rc = SDB_OK;

      _options = &options;

      rc = _filter.init(_options->filter());
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to init filter, rc=%d", rc);
         goto error;
      }

      _path = _options->path();
      SDB_ASSERT(SDB_OSS_FIL == _options->pathType() ||
                 SDB_OSS_DIR == _options->pathType(),
                 "path can only be file or directory");

      rc = _connectSdb();
      if (SDB_OK != rc)
      {
         goto error;
      }

      {
         stringstream ss;
         ss << "sdbreplay.tmp."
            << ossGetCurrentProcessID();
         _tmpFile = ss.str();

         // test
         ossFile file;
         rc = file.open(_tmpFile,
                        OSS_CREATEONLY | OSS_READWRITE,
                        OSS_DEFAULTFILE);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to open temporary file[%s], rc=%d",
                   _tmpFile.c_str(), rc);
            goto error;
         }
      }

      rc = _regSignalHandler();
      if (SDB_OK != rc)
      {
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::run()
   {
      INT32 rc = SDB_OK;

      _isRunning = TRUE;

      if (SDB_OSS_FIL == _options->pathType())
      {
         rc = _replayFile(_path);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to replay file[%s], rc=%d",
                   _path.c_str(), rc);
            goto error;
         }
      }
      else if (SDB_OSS_DIR == _options->pathType())
      {
         _archiveFileMgr.setArchivePath(_path);

         rc = _replayDir();
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to replay log files in directory[%s], rc=%d",
                   _path.c_str(), rc);
            goto error;
         }
      }
      else
      {
         SDB_ASSERT(FALSE, "invalid path type");
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "invalid path type: %d", _options->pathType());
         goto error;
      }

   done:
      PD_LOG(PDINFO, "replay result:\n%s", _monitor.dump().c_str());
      _isRunning = FALSE;
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_replayFile(const string& file)
   {
      INT32 rc = SDB_OK;
      dpsArchiveHeader* archiveHeader = NULL;
      dpsLogHeader* logHeader = NULL;
      dpsArchiveFile archiveFile;
      dpsLogFile logFile;
      string filePath = file;

      PD_LOG(PDEVENT, "Begin to replay archive log file[%s]",
             file.c_str());

      rc = _setLastFileTime(file);
      if (SDB_OK != rc)
      {
         goto error;
      }

      rc = archiveFile.init(filePath, TRUE);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to init archive log file[%s], rc=%d",
                filePath.c_str(), rc);
         goto error;
      }

      if (_filter.isFiltered(archiveFile))
      {
         PD_LOG(PDINFO, "Archive log file[%s] is filtered",
                file.c_str());
         goto done;
      }

      archiveHeader = archiveFile.getArchiveHeader();
      logHeader = archiveFile.getLogHeader();

      if (_monitor.getNextFileId() != DPS_INVALID_LOG_FILE_ID)
      {
         if (logHeader->_logID < _monitor.getNextFileId())
         {
            PD_LOG(PDINFO, "Archive log file[%s] is already replayed",
                file.c_str());
            goto done;
         }
      }

      if (archiveHeader->hasFlag(DPS_ARCHIVE_COMPRESSED))
      {
         ossFile::deleteFile(_tmpFile);
         rc = _archiveFileMgr.copyArchiveFile(file, _tmpFile,
                                              DPS_ARCHIVE_COPY_UNCOMPRESS);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to uncompress archive file[%s], rc=%d",
                   file.c_str(), rc);
            goto error;
         }

         archiveFile.close();
         archiveHeader = NULL;
         filePath = _tmpFile;
         rc = archiveFile.init(filePath, TRUE);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to init archive log file[%s], rc=%d",
                   filePath.c_str(), rc);
            goto error;
         }

         archiveHeader = archiveFile.getArchiveHeader();
         logHeader = archiveFile.getLogHeader();
         archiveHeader->unsetFlag(DPS_ARCHIVE_COMPRESSED);
      }

      // dpsLogFile requires strict file size,
      // so we extend the file if less than required size
      rc = _ensureFileSize(filePath, (INT64)logHeader->_fileSize);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to ensure file size[%s], rc=%d",
                filePath.c_str(), rc);
         goto error;
      }

      rc = logFile.init(filePath.c_str(),
                        (UINT32)logHeader->_fileSize,
                        logHeader->_fileNum);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to init log file[%s], rc=%d",
                filePath.c_str(), rc);
         goto error;
      }

      rc = _replayLogFile(logFile,
                          archiveHeader->startLSN.offset,
                          archiveHeader->endLSN.offset);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to replay log file[%s], rc=%d",
                filePath.c_str(), rc);
         goto error;
      }

      PD_LOG(PDEVENT, "Replay archive log file[%s] successfully",
             file.c_str());

      if (_options->remove())
      {
         rc = ossFile::deleteFile(file);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to delete log file[%s], rc=%d",
                   file.c_str(), rc);
            goto error;
         }
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_replayLogFile(engine::dpsLogFile& logFile,
                                  DPS_LSN_OFFSET startLSN, DPS_LSN_OFFSET endLSN)
   {
      INT32 rc = SDB_OK;
      dpsLogRecord log;
      dpsLogRecordHeader& logHeader = log.head();
      DPS_LSN_OFFSET currentLSN;

      SDB_ASSERT(startLSN <= endLSN, "invalid start LSN");

      currentLSN = logFile.getFirstLSN().offset;
      if (currentLSN < startLSN)
      {
         currentLSN = startLSN;
      }

      if (_monitor.getNextLSN() != DPS_INVALID_LSN_OFFSET)
      {
         if (currentLSN < _monitor.getNextLSN())
         {
            currentLSN = _monitor.getNextLSN();
         }
      }

      if (currentLSN >= endLSN)
      {
         PD_LOG(PDDEBUG, "No need to replay, currentLSN[%lld], endLSN[%lld]",
                currentLSN, endLSN);
         goto done;
      }

      PD_LOG(PDEVENT, "Replay from LSN[%lld] to LSN[%lld]",
             currentLSN, endLSN);

      while (currentLSN < endLSN)
      {
         if (!_isRunning)
         {
            rc = SDB_INTERRUPT;
            PD_LOG(PDINFO, "Replay is interrupted");
            goto done;
         }

         rc = logFile.read(currentLSN,
                           sizeof(dpsLogRecordHeader),
                           (CHAR*)&logHeader);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to read log file[%s], lsn[%lld], rc:%d",
                   logFile.path().c_str(), currentLSN, rc);
            goto error;
         }

         if (logHeader._lsn != currentLSN)
         {
            rc = SDB_DPS_INVALID_LSN;
            PD_LOG(PDERROR, "Invalid LSN, expect[%lld], real[%lld]",
                   currentLSN, logHeader._lsn);
            goto error;
         }

         rc = _ensureBufSize(logHeader._length);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to ensure buf size, rc=%d", rc);
            goto error;
         }

         rc = logFile.read(currentLSN, logHeader._length, _buf);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to read log file[%s], lsn[%lld], rc=%d",
                   logFile.path().c_str(), currentLSN, rc);
            goto error;
         }

         log.clear();
         rc = log.load(_buf);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to load log, lsn[%lld], rc=%d",
                   currentLSN, rc);
            goto error;
         }

         if (_filter.isFiltered(log))
         {
            currentLSN += logHeader._length;
            _monitor.setNextLSN(currentLSN);
            continue;
         }

         if (_options->dump())
         {
            _dumpLog(log);
         }
         else
         {
            rc = _replayLog(_buf);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "Failed to replay log, lsn[%lld], rc=%d",
                      logHeader._lsn, rc);
               goto error;
            }
         }

         currentLSN += logHeader._length;
         _monitor.opCount(logHeader._type);
         _monitor.setNextLSN(currentLSN);
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_replayLog(const CHAR* log)
   {
      INT32 rc = SDB_OK;
      const dpsLogRecordHeader& header = *(const dpsLogRecordHeader*)log;
      UINT16 type = header._type;
      switch(type)
      {
      case LOG_TYPE_DATA_INSERT:
         rc = _replayInsert(log);
         break;
      case LOG_TYPE_DATA_UPDATE:
         rc = _replayUpdate(log);
         break;
      case LOG_TYPE_DATA_DELETE:
         rc = _replayDelete(log);
         break;
      case LOG_TYPE_CL_TRUNC:
         rc = _replayTruncateCL(log);
         break;
      default:
         SDB_ASSERT(FALSE, "invalid log type");
      }

      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to replay log, type=%u, rc=%d",
                type, rc);
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_replayInsert(const CHAR* log)
   {
      INT32 rc = SDB_OK;
      const dpsLogRecordHeader& header = *(const dpsLogRecordHeader*)log;
      const CHAR* fullName = NULL;
      BSONObj obj;
      sdbCollection cl;

      SDB_ASSERT(LOG_TYPE_DATA_INSERT == header._type, "not data insert log");

      rc = dpsRecord2Insert((CHAR*)&header, &fullName, obj);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to parse log record, lsn[%lld], rc=%d",
                header._lsn, rc);
         goto error;
      }

      rc = _sdb.getCollection(fullName, cl);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to get collection:%s, lsn[%lld], rc=%d",
                fullName, header._lsn, rc ) ;
         goto error ;
      }

      rc = cl.insert(obj);
      if (SDB_OK != rc)
      {
         if (SDB_IXM_DUP_KEY == rc)
         {
            /* If duplicate key was found, just skip. */
            rc = SDB_OK;
         }
         else
         {
            PD_LOG(PDERROR, "Failed to insert record(%s), lsn[%lld], rc=%d",
                   obj.toString(FALSE, TRUE).c_str(), header._lsn, rc);
            goto error;
         }
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_replayUpdate(const CHAR* log)
   {
      INT32 rc = SDB_OK;
      const dpsLogRecordHeader& header = *(const dpsLogRecordHeader*)log;
      sdbCollection cl;
      const CHAR *fullName = NULL;
      BSONObj match;
      BSONObj oldObj;
      BSONObj newMatch;
      BSONObj modifier;
      BSONObj hint = BSON( "" << "$id" );

      SDB_ASSERT(LOG_TYPE_DATA_UPDATE == header._type, "not data update log");

      rc = dpsRecord2Update((CHAR*)&header, &fullName,
                             match, oldObj, newMatch, modifier);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to parse log record[%lld], rc:%d",
                header._lsn, rc);
         goto error;
      }

      rc = _sdb.getCollection(fullName, cl);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to get collection:%s, lsn[%lld], rc=%d",
                fullName, header._lsn, rc ) ;
         goto error ;
      }

      rc = cl.update(modifier, match, hint);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to update record[%s:%s], lsn[%lld], rc=%d",
                modifier.toString(FALSE, TRUE).c_str(), 
                match.toString(FALSE, TRUE).c_str(), header._lsn, rc);
         goto error ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_replayDelete(const CHAR* log)
   {
      INT32 rc = SDB_OK;
      const dpsLogRecordHeader& header = *(const dpsLogRecordHeader*)log;
      sdbCollection cl;
      const CHAR *fullName = NULL;
      BSONObj obj;
      BSONObj hint = BSON( "" << "$id" );

      SDB_ASSERT(LOG_TYPE_DATA_DELETE == header._type, "not data delete log");

      rc = dpsRecord2Delete((CHAR*)&log, &fullName, obj);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to parse log record[%lld], rc=%d",
                header._lsn, rc);
         goto error;
      }

      rc = _sdb.getCollection(fullName, cl);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to get collection: %s, lsn[%lld], rc=%d", 
                fullName, header._lsn, rc);
         goto error;
      }

      rc = cl.del(obj, hint);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to delete record[%s], lsn[%lld], rc=%d",
                obj.toString(FALSE, TRUE).c_str(), header._lsn, rc);
         goto error ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_replayTruncateCL(const CHAR* log)
   {
      INT32 rc = SDB_OK;
      const dpsLogRecordHeader& header = *(const dpsLogRecordHeader*)log;
      sdbCollection cl;
      const CHAR *fullName = NULL;

      SDB_ASSERT(LOG_TYPE_CL_TRUNC == header._type, "not trunc cl log");

      rc = dpsRecord2CLTrunc((CHAR*)&log, &fullName);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to parse log record[%lld], rc=%d",
                header._lsn, rc);
         goto error;
      }

      rc = _sdb.getCollection(fullName, cl);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to get collection: %s, lsn[%lld], rc=%d", 
                fullName, header._lsn, rc);
         goto error;
      }

      rc = cl.truncate();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to truncate collection[%s], lsn[%lld], rc=%d",
                fullName, header._lsn, rc);
         goto error ;
      }

   done:
      return rc;
   error:
      goto done;
   }

   void Replayer::_dumpLog(const engine::dpsLogRecord& log)
   {
      const INT32 len = 4096 ;
      static CHAR buf[len] = {0};

      log.dump(buf, len - 1, DPS_DMP_OPT_FORMATTED);
      std::cout << buf << std::endl;
   }

   INT32 Replayer::_replayDir()
   {
      INT32 rc = SDB_OK;

      for (;;)
      {
         UINT32 minFileId = DPS_INVALID_LOG_FILE_ID;
         UINT32 maxFileId = DPS_INVALID_LOG_FILE_ID;

         if (!_isRunning)
         {
            rc = SDB_INTERRUPT;
            PD_LOG(PDINFO, "Replay is interrupted");
            goto done;
         }

         rc = _scanDir(minFileId, maxFileId);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to scan directory[%s], rc=%d",
                   _path.c_str(), rc);
            goto error;
         }

         if (DPS_INVALID_LOG_FILE_ID != minFileId)
         {
            rc = _replayFiles(minFileId, maxFileId);
            if (SDB_OK != rc)
            {
               goto error;
            }
         }

         if (!_options->watch())
         {
            break;
         }

         if (!_isRunning)
         {
            rc = SDB_INTERRUPT;
            PD_LOG(PDINFO, "Replay is interrupted");
            goto done;
         }

         ossSleep(RPL_WATCH_INTERVAL);
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_scanDir(UINT32& minFileId, UINT32& maxFileId)
   {
      INT32 rc = SDB_OK;

      rc = _archiveFileMgr.scanArchiveFiles(minFileId, maxFileId);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to scan archive directory[%s], rc=%d",
                _path.c_str(), rc);
         goto error;
      }

      // no archive log
      if (DPS_INVALID_LOG_FILE_ID == minFileId)
      {
         goto done;
      }

      SDB_ASSERT(DPS_INVALID_LOG_FILE_ID != maxFileId, "invalid max file id");

      if (DPS_INVALID_LOG_FILE_ID == _monitor.getNextFileId())
      {
         goto done;
      }

      if (maxFileId < _monitor.getNextFileId())
      {
         PD_LOG(PDDEBUG, "Max log file id[%u] is less than last[%u]",
                maxFileId, _monitor.getNextFileId());
         minFileId = DPS_INVALID_LOG_FILE_ID;
         maxFileId = DPS_INVALID_LOG_FILE_ID;
         goto done;
      }

      if (maxFileId == _monitor.getNextFileId())
      {
         BOOLEAN exist = FALSE;
         time_t maxFileTime = 0;

         string maxFilePath = _archiveFileMgr.getPartialFilePath(maxFileId);
         rc = ossFile::exists(maxFilePath, exist);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to access file[%s], rc=%d",
                   maxFilePath.c_str(), rc);
            goto error;
         }

         if (exist)
         {
            rc = ossFile::getLastWriteTime(maxFilePath, maxFileTime);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "Failed to get last write of file[%s], rc=%d",
                      maxFilePath.c_str(), rc);
               goto error;
            }

            PD_LOG(PDDEBUG, "Last wirte time of file[%s] is %d, last file time is %u",
                   maxFilePath.c_str(), maxFileTime, _monitor.getLastFileTime());

            if (maxFileTime == _monitor.getLastFileTime())
            {
               minFileId = DPS_INVALID_LOG_FILE_ID;
               maxFileId = DPS_INVALID_LOG_FILE_ID;
               goto done;
            }
         }

         maxFilePath = _archiveFileMgr.getFullFilePath(maxFileId);
         rc = ossFile::exists(maxFilePath, exist);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to access file[%s], rc=%d",
                   maxFilePath.c_str(), rc);
            goto error;
         }

         if (exist)
         {
            rc = ossFile::getLastWriteTime(maxFilePath, maxFileTime);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "Failed to get last write of file[%s], rc=%d",
                      maxFilePath.c_str(), rc);
               goto error;
            }

            PD_LOG(PDDEBUG, "Last wirte time of file[%s] is %d, last file time is %u",
                   maxFilePath.c_str(), maxFileTime, _monitor.getLastFileTime());

            if (maxFileTime == _monitor.getLastFileTime())
            {
               minFileId = DPS_INVALID_LOG_FILE_ID;
               maxFileId = DPS_INVALID_LOG_FILE_ID;
               goto done;
            }
         }
      }

      minFileId = _monitor.getNextFileId();

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_replayFiles(UINT32 minFileId, UINT32 maxFileId)
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(DPS_INVALID_LOG_FILE_ID != minFileId, "invalid min file id");
      SDB_ASSERT(DPS_INVALID_LOG_FILE_ID != maxFileId, "invalid max file id");

      for (UINT32 i = minFileId; i <= maxFileId; i++)
      {
         BOOLEAN exist = FALSE;

         if (!_isRunning)
         {
            rc = SDB_INTERRUPT;
            PD_LOG(PDINFO, "Replay is interrupted");
            goto done;
         }

         string file = _archiveFileMgr.getFullFilePath(i);

         rc = ossFile::exists(file, exist);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to access file[%s], rc=%d",
                   file.c_str(), rc);
            goto error;
         }

         if (exist)
         {
            rc = _replayFile(file);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "Failed to replay file[%s], rc=%d",
                      file.c_str(), rc);
               goto error;
            }

            // full file
            _monitor.setNextFileId(i + 1);

            PD_LOG(PDINFO, "current replay:\n%s", _monitor.dump().c_str());
            continue;
         }

         file = _archiveFileMgr.getPartialFilePath(i);

         rc = ossFile::exists(file, exist);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to access file[%s], rc=%d",
                   file.c_str(), rc);
            goto error;
         }

         if (exist)
         {
            rc = _replayFile(file);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "Failed to replay file[%s], rc=%d",
                      file.c_str(), rc);
               goto error;
            }

            // partial file
            _monitor.setNextFileId(i);

            PD_LOG(PDINFO, "current replay:\n%s", _monitor.dump().c_str());
            continue;
         }

         // no archive log file for i
         break;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_ensureFileSize(const string& filePath, INT64 fileSize)
   {
      INT32 rc = SDB_OK;
      INT64 realSize = 0;

      rc = ossFile::getFileSize(filePath, realSize);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to get file size[%s], rc=%d",
                filePath.c_str(), rc);
         goto error;
      }

      realSize -= DPS_LOG_HEAD_LEN;

      if (realSize < fileSize)
      {
         INT64 increment = fileSize - realSize;
         rc = ossFile::extend(filePath, increment);
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "Failed to extend file[%s], rc=%d",
                   filePath.c_str(), rc);
            goto error;
         }
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_ensureBufSize(UINT32 size)
   {
      INT32 rc = SDB_OK;

      if (_bufSize >= size)
      {
         goto done;
      }

      if (0 != _bufSize)
      {
         SAFE_OSS_FREE(_buf);
         _bufSize = 0;
      }

      SDB_ASSERT(NULL == _buf, "_buf should be NULL");
      _buf = (CHAR*)SDB_OSS_MALLOC(size);
      if (NULL == _buf)
      {
         rc = SDB_OOM;
         PD_LOG(PDERROR, "Failed to malloc, size=%u", size);
         goto error;
      }
      _bufSize = size;

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_setLastFileTime(const string& filePath)
   {
      time_t lastTime = 0;
      INT32 rc = SDB_OK;

      rc = ossFile::getLastWriteTime(filePath, lastTime);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to get last wirte time of file[%s], rc=%d",
                filePath.c_str(), rc);
         goto error;
      }

      _monitor.setLastFileTime(lastTime);

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Replayer::_connectSdb()
   {
      INT32 rc = SDB_OK;

      if (_options->dump())
      {
         goto done;
      }

      rc = _sdb.connect(_options->hostName().c_str(),
                        _options->serviceName().c_str(),
                        _options->user().c_str(),
                        _options->password().c_str());
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "Failed to connect to sdb[%s:%s], rc=%d",
                _options->hostName().c_str(),
                _options->serviceName().c_str(),
                rc);
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }
}

