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

   Source File Name = impRoutine.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "impRoutine.hpp"
#include "impInputStream.hpp"
#include "impRecordScanner.hpp"
#include "impRecordParser.hpp"
#include "impCSVRecordParser.hpp"
#include "impRecordImporter.hpp"
#include "../util/text.h"
#include <iostream>
#include <sstream>

namespace import
{
   struct ImporterArgs: public WorkerArgs
   {
      INT32          id;
      string         hostname;
      string         svcname;
      string         user;
      string         password;
      string         csname;
      string         clname;
      BOOLEAN        useSSL;
      BOOLEAN        dryRun;
      BOOLEAN        verbose;
      RecordQueue*   workQueue;
      RecordQueue*   idleQueue;
      LogFile*       logFile;
      ossAtomicSigned64* importedNum;
      ossAtomicSigned64* importFailureNum;
      ossAtomicSigned32* importesLivingNum;

      ImporterArgs()
      {
         id = -1;
         useSSL = FALSE;
         dryRun = FALSE;
         verbose = FALSE;
         workQueue = NULL;
         idleQueue = NULL;
         logFile = NULL;
         importedNum = NULL;
         importFailureNum = NULL;
      }

      ~ImporterArgs()
      {
         workQueue = NULL;
         idleQueue = NULL;
         logFile = NULL;
         importedNum = NULL;
         importFailureNum = NULL;
      }
   };

   static void _importerRoutine(WorkerArgs* args)
   {
      ImporterArgs* impArgs = (ImporterArgs*)args;
      RecordArray records;
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != args, "arg can't be NULL");

      ossAtomicSigned64* importedNum = impArgs->importedNum;
      ossAtomicSigned64* importFailureNum = impArgs->importFailureNum;
      BOOLEAN dryRun = impArgs->dryRun;
      LogFile* logFile = impArgs->logFile;
      RecordQueue* workQueue = impArgs->workQueue;
      RecordQueue* idleQueue = impArgs->idleQueue;
      RecordImporter importer(impArgs->hostname,
                              impArgs->svcname,
                              impArgs->user,
                              impArgs->password,
                              impArgs->csname,
                              impArgs->clname,
                              impArgs->useSSL);

      SDB_ASSERT(NULL != workQueue, "workQueue can't be NULL");
      SDB_ASSERT(NULL != idleQueue, "idelQueue can't be NULL");
      SDB_ASSERT(NULL != logFile, "logFile can't be NULL");
      SDB_ASSERT(NULL != importedNum, "importedNum can't be NULL");
      SDB_ASSERT(NULL != importFailureNum, "importFailureNum can't be NULL");
      SDB_ASSERT(NULL != impArgs->importesLivingNum,
                 "impArgs->importesLivingNum can't be NULL");

      if (impArgs->verbose)
      {
         stringstream ss;
         ss << "importer [" << impArgs->id << "] with "
            << impArgs->hostname << ":" << impArgs->svcname
            << " started..." << std::endl;
         std::cout << ss.str();
      }

      rc = importer.connect();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to connect, rc=%d", rc);
         goto error;
      }

      for(;;)
      {
         workQueue->wait_and_pop(records);

         if (records.empty())
         {
            // stop
            break;
         }

         if (!dryRun)
         {
            rc = importer.import(records.array(), records.size());
            if (SDB_OK != rc)
            {
               importFailureNum->add(records.size());
               for (INT32 i = 0; i < records.size(); i++)
               {
                  bson* obj = records[i];
                  if (SDB_OK != logFile->write(obj))
                  {
                     break;
                  }
               }
               PD_LOG(PDERROR, "failed to import records, rc=%d", rc);
               goto error;
            }
            importedNum->add(records.size());
         }

         records.reset();
         idleQueue->push(records);
      }

   done:
      if (impArgs->verbose)
      {
         stringstream ss;
         ss << "importer [" << impArgs->id << "] stop" << std::endl;
         std::cout << ss.str();
      }
      impArgs->importesLivingNum->dec();
      return;
   error:
      goto done;
   }

   static INT32 _getFreeRecordArray(RecordQueue& queue, INT32 capacity,
                                    RecordArray& recordArray)
   {
      RecordArray array;
      INT32 rc = SDB_OK;

      if (queue.try_pop(recordArray))
      {
         recordArray.reset();
         goto done;
      }

      rc = array.init(capacity);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to init RecordArray, rc=%d", rc);
         goto error;
      }

      recordArray = array;

   done:
      return rc;
   error:
      goto error;
   }

   struct ParserArgs: public WorkerArgs
   {
      INT32          id;
      const Options* options;
      LogFile*       logFile;
      RecordQueue*   workQueue;
      RecordQueue*   idleQueue;
      BOOLEAN*       stopped;
      INT64*         parsedNum;
      INT64*         parseFailureNum;

      ParserArgs()
      {
         id = -1;
         options = NULL;
         logFile = NULL;
         workQueue = NULL;
         idleQueue = NULL;
         stopped = NULL;
         parsedNum = NULL;
         parseFailureNum = NULL;
      }

      ~ParserArgs()
      {
         options = NULL;
         logFile = NULL;
         workQueue = NULL;
         idleQueue = NULL;
         stopped = NULL;
         parsedNum = NULL;
         parseFailureNum = NULL;
      }
   };

   static void _parserRoutine(WorkerArgs* args)
   {
      ParserArgs* parserArgs = (ParserArgs*)args;
      CHAR* buffer = NULL;
      InputStream* input = NULL;
      RecordParser* parser = NULL;
      RecordArray recordArray;
      INT64 readSize = 0;
      INT32 remainSize = 0;
      INT64 totalSize = 0;
      INT32 recordNum = 0;
      INT32 countInBatch = 0;
      BOOLEAN isFirst = TRUE;
      BOOLEAN final = FALSE;
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != args, "arg can't be NULL");

      const Options* options = parserArgs->options;
      LogFile* logFile = parserArgs->logFile;
      RecordQueue* workQueue = parserArgs->workQueue;
      RecordQueue* idleQueue = parserArgs->idleQueue;
      BOOLEAN* stopped = parserArgs->stopped;
      INT64* parsedNum = parserArgs->parsedNum;
      INT64* parseFailureNum = parserArgs->parseFailureNum;

      SDB_ASSERT(NULL != options, "options can't be NULL");
      SDB_ASSERT(NULL != logFile, "logFile can't be NULL");
      SDB_ASSERT(NULL != workQueue, "workQueue can't be NULL");
      SDB_ASSERT(NULL != idleQueue, "idelQueue can't be NULL");
      SDB_ASSERT(NULL != stopped, "stopped can't be NULL");
      SDB_ASSERT(NULL != parsedNum, "parsedNum can't be NULL");
      SDB_ASSERT(NULL != parseFailureNum, "parseFailureNum can't be NULL");

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

      rc = _getFreeRecordArray(*idleQueue, options->batchSize(), recordArray);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to get free RecordArray");
         goto error;
      }

      while(!(*stopped))
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
                        std::cout << "fields: " << fields
                                  << std::endl << std::endl;

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

                     SDB_ASSERT(countInBatch < recordArray.capacity(),
                                "countInBatch must be less than batchSize");

                     obj = recordArray[countInBatch];
                     if (NULL == obj)
                     {
                        obj = (bson*)SDB_OSS_MALLOC(sizeof(bson));
                        if (NULL == obj)
                        {
                           rc = SDB_OOM;
                           PD_LOG(PDERROR, "failed to malloc bson");
                           goto error;
                        }

                        recordArray[countInBatch] = obj;
                     }

                     rc = parser->parseRecord(buf, recordLength, *obj);
                     if (SDB_OK == rc)
                     {
                        recordArray.inc();
                        countInBatch++;
                        recordNum++;
                        (*parsedNum)++;
                     }
                     else
                     {
                        (*parseFailureNum)++;
                        logFile->write(buf, recordLength);

                        PD_LOG(PDERROR, "failed to parse record, rc=%d", rc);

                        if (options->errorStop())
                        {
                           goto error;
                        }
                     }

                     if (recordArray.full())
                     {
                        workQueue->push(recordArray);
                        countInBatch = 0;
                        rc = _getFreeRecordArray(*idleQueue,
                                                 options->batchSize(),
                                                 recordArray);
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

      if (!recordArray.empty())
      {
         if (options->dryRun())
         {
            idleQueue->push(recordArray);
         }
         else
         {
            workQueue->push(recordArray);
         }
      }

   done:
      *stopped = TRUE;
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
      return;
   error:
      goto done;
   }
   
   Routine::Routine(const Options& options)
   : _options(options),
     _coords(options.hostname(),
             options.svcname(),
             options.user(),
             options.password(),
             options.useSSL()),
     _importesLivingNum(0),
     _importedNum(0),
     _importFailureNum(0)
   {
      _parser = NULL;
      _parserStopped = FALSE;

      _parsedNum = 0;
      _parseFailureNum = 0;
   }

   Routine::~Routine()
   {
      SAFE_OSS_DELETE(_parser);

      // importers
      {
         INT32 num = _importers.size();
         for (INT32 i = 0; i < num; i++)
         {
            Worker* importer = _importers.front();
            _importers.pop();
            SDB_OSS_DEL(importer);
         }
      }

      // workQueue
      {
         if (!_workQueue.empty())
         {
            RecordArray array;
            while (_workQueue.try_pop(array))
            {
               bson** objs = array.array();
               for (INT32 i = 0; i < array.capacity(); i++)
               {
                  bson* obj = objs[i];
                  if (NULL != obj)
                  {
                     bson_destroy(obj);
                     SDB_OSS_FREE(obj);
                     objs[i] = NULL;
                  }
               }
               array.free();
            }
         }
      }

      // idleQueue
      {
         if (!_idleQueue.empty())
         {
            RecordArray array;
            while (_idleQueue.try_pop(array))
            {
               bson** objs = array.array();
               for (INT32 i = 0; i < array.capacity(); i++)
               {
                  bson* obj = objs[i];
                  if (NULL != obj)
                  {
                     bson_destroy(obj);
                     SDB_OSS_FREE(obj);
                     objs[i] = NULL;
                  }
               }
               array.free();
            }
         }
      }

   }

   INT32 Routine::startImporters(INT32 num)
   {
      INT32 rc = SDB_OK;

      string importerLogFile = makeRecordLogFileName(_options.csname(),
                                                   _options.clname(),
                                                   string("import"));

      rc = _importerLogFile.init(importerLogFile, TRUE);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to init importer log file, rc=%d", rc);
         goto error;
      }

      if (_options.enableCoord())
      {
         rc = _coords.init();
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to init coords, rc=%d", rc);
            goto error;
         }
      }

      for (INT32 i = 0; i < num; i++)
      {
         string hostname;
         string svcname;
         ImporterArgs* args = NULL;

         if (_options.enableCoord())
         {
            rc = _coords.getRandomCoord(hostname, svcname);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "failed to get coord, rc=%d", rc);
               goto error;
            }
         }
         else
         {
            hostname = _options.hostname();
            svcname = _options.svcname();
         }

         args = SDB_OSS_NEW ImporterArgs();
         if (NULL == args)
         {
            rc = SDB_OOM;
            goto error;
         }

         args->id = i;
         args->hostname = hostname;
         args->svcname = svcname;
         args->user = _options.user();
         args->password = _options.password();
         args->csname = _options.csname();
         args->clname = _options.clname();
         args->useSSL = _options.useSSL();
         args->dryRun = _options.dryRun();
         args->verbose = _options.verbose();
         args->workQueue = &_workQueue;
         args->idleQueue = &_idleQueue;
         args->logFile = &_importerLogFile;
         args->importedNum = &_importedNum;
         args->importFailureNum = &_importFailureNum;
         args->importesLivingNum = &_importesLivingNum;

         Worker* importer = SDB_OSS_NEW Worker(_importerRoutine, args, TRUE);
         if (NULL == importer)
         {
            SDB_OSS_DEL(args);
            rc = SDB_OOM;
            PD_LOG(PDERROR, "failed to create importer Worker object");
            goto error;
         }
         args = NULL;

         rc = importer->start();
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to start importer");
            SDB_OSS_DEL(importer);
            goto error;
         }

         _importers.push(importer);
         _importesLivingNum.inc();
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Routine::stopImporters()
   {
      INT32 rc = SDB_OK;
      INT32 num = _importers.size();

      if (0 == num)
      {
         goto done;
      }

      for (INT32 i = 0; i < num; i++)
      {
         RecordArray empty;

         // push empty RecordArray as stop signal
         _workQueue.push(empty);
      }

      for (INT32 i = 0; i < num; i++)
      {
         Worker* importer = _importers.front();

         rc = importer->waitStop();
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to wait import stop");
         }

         _importers.pop();
         SDB_OSS_DEL(importer);
      }

   done:
      return rc;
   }

   INT32 Routine::startParser()
   {
      ParserArgs* args = NULL;
      Worker* parser = NULL;
      INT32 rc = SDB_OK;

      string parserLogFile = makeRecordLogFileName(_options.csname(),
                                                   _options.clname(),
                                                   string("parse"));

      rc = _parserLogFile.init(parserLogFile, FALSE);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to init parser log file");
         goto error;
      }

      args = SDB_OSS_NEW ParserArgs();
      if (NULL == args)
      {
         rc = SDB_OOM;
         goto error;
      }

      args->id = 0;
      args->options = &_options;
      args->logFile = &_parserLogFile;
      args->workQueue = &_workQueue;
      args->idleQueue = &_idleQueue;
      args->stopped = &_parserStopped;
      args->parsedNum = &_parsedNum;
      args->parseFailureNum = &_parseFailureNum;

      parser = SDB_OSS_NEW Worker(_parserRoutine, args, TRUE);
      if (NULL == parser)
      {
         SDB_OSS_DEL(args);
         rc = SDB_OOM;
         PD_LOG(PDERROR, "failed to create parser Worker object");
         goto error;
      }
      args = NULL;

      rc = parser->start();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to start parser");
         SDB_OSS_DEL(parser);
         goto error;
      }

      _parser = parser;

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Routine::waitParserStop()
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(NULL != _parser, "_parser can't be NULL");

      _parserStopped = TRUE;

      rc = _parser->waitStop();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to wait the parser stop");
      }

      SAFE_OSS_DELETE(_parser);
      return rc;
   }

   void Routine::printStatistics()
   {
      stringstream ss;

      ss << "parsed records: " << _parsedNum << std::endl
         << "parse failure: " << _parseFailureNum << std::endl;

      ss << "imported records: " << _importedNum.fetch() << std::endl
         << "import failure: " << _importFailureNum.fetch() << std::endl;

      if (_parseFailureNum > 0)
      {
         ss << "see " << _parserLogFile.filename()
            << " for parse failure records" << std::endl;
      }

      if (_importFailureNum.fetch() > 0)
      {
         ss << "see " << _importerLogFile.filename()
            << " for import failure records" << std::endl;
      }

      string stat = ss.str();

      std::cout << stat;
      PD_LOG(PDEVENT, stat.c_str());
   }
}

