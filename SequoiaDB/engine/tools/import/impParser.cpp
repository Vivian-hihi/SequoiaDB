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

   Source File Name = impParser.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "impParser.hpp"
#include "impInputStream.hpp"
#include "impRecordScanner.hpp"
#include "impRecordParser.hpp"
#include "impCSVRecordParser.hpp"
#include "impMonitor.hpp"
#include "../util/text.h"
#include "pd.hpp"

namespace import
{

   void _parserRoutine(WorkerArgs* args)
   {
      Parser* self = (Parser*)args;

      CHAR* buffer = NULL;
      InputStream* input = NULL;
      RecordParser* parser = NULL;
      RecordArray* recordArray = NULL;

      INT64 readSize = 0;
      INT32 remainSize = 0;
      INT64 totalSize = 0;
      INT32 countInBatch = 0;
      BOOLEAN isFirst = TRUE;
      BOOLEAN final = FALSE;
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != args, "arg can't be NULL");

      const Options* options = self->_options;
      LogFile* logFile = &(self->_logFile);
      RecordQueue* workQueue = self->_workQueue;
      Monitor* monitor = impGetMonitor();

      SDB_ASSERT(NULL != options, "options can't be NULL");
      SDB_ASSERT(NULL != logFile, "logFile can't be NULL");
      SDB_ASSERT(NULL != workQueue, "workQueue can't be NULL");
      SDB_ASSERT(NULL != monitor, "monitor can't be NULL");


      {
         CHAR* str = "parser started...\n";

         PD_LOG(PDEVENT, "%s", str);
         if (options->verbose())
         {
            std::cout << str;
         }
      }

      RecordScanner scanner(options->recordDelimiter(),
                            options->stringDelimiter(),
                            options->linePriority());

      INT32 bufferSize = options->bufferSize() * 1024 * 1024;
      // 1 byte to ensure it's safe to terminate string
      buffer = (CHAR*)SDB_OSS_MALLOC(bufferSize + 1);
      if (NULL == buffer)
      {
         rc = SDB_OOM;
         PD_LOG(PDERROR, "failed to malloc buffer, size=%d", bufferSize + 1);
         goto error;
      }
      buffer[bufferSize] = '\0'; 

      rc = InputStream::createInstance(options->inputType(), *options, input);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to create InputStream object,"
                "rc=%d, INPUT_TYPE=%d", rc, options->inputType());
         goto error;
      }

      rc = RecordParser::createInstance(options->inputFormat(),
                                        *options, parser);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to create RecordParser object,"
                "rc=%d, INPUT_FORMAT=%d", rc, options->inputFormat());
         goto error;
      }

      rc = getRecordArray(options->batchSize(), &recordArray);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to get free RecordArray");
         goto error;
      }

      while(!self->_stopped)
      {
         rc = input->read(buffer + remainSize,
                          bufferSize - remainSize,
                          readSize);
         if (SDB_OK != rc)
         {
            if (SDB_EOF != rc)
            {
               printf("\nread error!\n");
               PD_LOG(PDERROR, "failed to read from InputStream, rc=%d", rc);
               goto error;
            }

            if (remainSize == 0)
            {
               rc = SDB_OK;
               break;
            }

            readSize = 0;
            final = TRUE;
         }

         totalSize += readSize;

         CHAR* buf = buffer;
         INT32 length = readSize + remainSize;
         for(;;)
         {
            INT32 recordLength = 0;
            rc = scanner.scan(buf, length, final, recordLength);
            if (SDB_OK == rc)
            {
               if (recordLength > 0)
               {
                  if (!options->force() && !isValidUTF8WSize(buf, recordLength))
                  {
                     rc = SDB_MIG_DATA_NON_UTF;
                     PD_LOG(PDERROR, "It is not utf-8 file, rc=%d", rc);
                     if (options->errorStop())
                     {
                        goto error;
                     }
                  }

                  if (isFirst &&
                      FORMAT_CSV == options->inputFormat() &&
                      options->hasHeaderLine())
                  {
                     if (options->fields().empty())
                     {
                        string fields = string(buf, recordLength);

                        PD_LOG(PDINFO, "fields: %s", fields.c_str());
                        if (options->verbose())
                        {
                           std::cout << "fields: " << fields
                                     << std::endl;
                        }

                        CSVRecordParser* csvParser = (CSVRecordParser*)parser;

                        rc = csvParser->parseFields(buf, recordLength);
                        if (SDB_OK != rc)
                        {
                           std::cout << "failed to parse fields" << std::endl;
                           PD_LOG(PDERROR, "failed to parse fields, rc = %d",
                                  rc);
                           goto error;
                        }

                        if (options->verbose())
                        {
                           csvParser->printFieldsDef();
                        }
                     }
                     isFirst = FALSE;
                  }
                  else
                  {
                     bson* obj = NULL;

                     SDB_ASSERT(countInBatch < recordArray->capacity(),
                                "countInBatch must be less than batchSize");

                     obj = (bson*)SDB_OSS_MALLOC(sizeof(bson));
                     if (NULL == obj)
                     {
                        rc = SDB_OOM;
                        PD_LOG(PDERROR, "failed to malloc bson");
                        goto error;
                     }
                     bson_init(obj);

                     rc = parser->parseRecord(buf, recordLength, *obj);
                     if (SDB_OK == rc)
                     {
                        SDB_ASSERT(!recordArray->full(), "can't be full");
                        recordArray->push(obj);
                        countInBatch++;
                        self->_parsedNum++;
                     }
                     else
                     {
                        bson_destroy(obj);
                        SAFE_OSS_FREE(obj);
                        self->_failedNum++;
                        logFile->write(buf, recordLength);

                        PD_LOG(PDERROR, "failed to parse record, rc=%d", rc);

                        if (options->errorStop())
                        {
                           goto error;
                        }
                     }

                     if (recordArray->full())
                     {
                        monitor->recordsMemInc(recordArray->bsonSize());
                        recordArray->finish();
                        workQueue->push(recordArray);
                        countInBatch = 0;
                        recordArray = NULL;

                        if (monitor->recordsMem() > options->recordsMem())
                        {
                           // records' memory is beyond the threshold,
                           // so wait a moment
                           PD_LOG(PDEVENT, "records memory is beyond the threshold");
                           for(;;)
                           {
                              ossSleep(100);
                              if (monitor->recordsMem() <= (options->recordsMem() / 2))
                              {
                                 break;
                              }
                           }
                           PD_LOG(PDEVENT, "records memory usage down, go on");
                        }

                        rc = getRecordArray(options->batchSize(), &recordArray);
                        if (SDB_OK != rc)
                        {
                           PD_LOG(PDERROR, "failed to get free RecordArray");
                           goto error;
                        }
                     }
                  }
               }
               else if (isFirst &&
                        FORMAT_CSV == options->inputFormat() &&
                        options->hasHeaderLine())
               {
                  rc = SDB_INVALIDARG;
                  PD_LOG(PDERROR, "the headerline is empty");
                  printf("ERROR: the headerline is empty\n");
                  goto error;
               }

               length -= recordLength + options->recordDelimiter().length();
               buf += recordLength + options->recordDelimiter().length();

               if (length <= 0)
               {
                  remainSize = 0;
                  break;
               }
            }
            else if (SDB_EOF == rc)
            {
               ossMemmove(buffer, buf, length);
               remainSize = length;
               break;
            }
            else
            {
               printf("\nscan record error!\n");
               PD_LOG(PDERROR, "failed to scan record");
               goto error;
            }
         }
      }

   done:
      if (NULL != recordArray && !recordArray->empty())
      {
         workQueue->push(recordArray);
         recordArray = NULL;
      }
      self->_stopped = TRUE;
      if (NULL != input)
      {
         InputStream::releaseInstance(input);
         input = NULL;
      }
      if (NULL != parser)
      {
         RecordParser::releaseInstance(parser);
         parser = NULL;
      }
      SAFE_OSS_FREE(buffer);

      {
         CHAR* str= "parser stopped\n";

         PD_LOG(PDEVENT, "%s", str);
         if (options->verbose())
         {
            std::cout << str;
         }
      }
      return;
   error:
      goto done;
   }

   Parser::Parser()
   {
      _options = NULL;
      _workQueue = NULL;
      _inited = FALSE;
      _worker = NULL;
      _stopped = TRUE;
      _parsedNum = 0;
      _failedNum = 0;
   }

   Parser::~Parser()
   {
      SAFE_OSS_DELETE(_worker);
   }

   INT32 Parser::init(Options* options,
                      RecordQueue* workQueue)
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != options, "options can't be NULL");
      SDB_ASSERT(NULL != workQueue, "workQueue can't be NULL");

      _options = options;
      _workQueue = workQueue;

      string parserLogFile = makeRecordLogFileName(_options->csname(),
                                                   _options->clname(),
                                                   string("parse"));

      rc = _logFile.init(parserLogFile, FALSE);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to init parser log file");
         goto error;
      }

      _worker = SDB_OSS_NEW Worker(_parserRoutine, this);
      if (NULL == _worker)
      {
         rc = SDB_OOM;
         PD_LOG(PDERROR, "failed to create parser Worker object");
         goto error;
      }

      _inited = TRUE;

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Parser::start()
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(_inited, "must be inited");

      rc = _worker->start();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to start parser");
         goto error;
      }

      _stopped = FALSE;

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Parser::stop()
   {
      INT32 rc = SDB_OK;

      if(_inited)
      {
         SDB_ASSERT(NULL != _worker, "_worker can't be NULL");

         _stopped = TRUE;

         rc = _worker->waitStop();
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to wait the parser stop");
         }

         SAFE_OSS_DELETE(_worker);
      }
      return rc;
   }
}
