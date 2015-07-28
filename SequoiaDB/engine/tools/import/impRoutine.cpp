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
   Routine::Routine(Options& options)
   : _options(options)
   {
   }

   Routine::~Routine()
   {
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

   INT32 Routine::run()
   {
      INT32 rc = SDB_OK;

      rc = _startImporter(_options.jobs());
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to start importers, rc=%d", rc);
         goto error;
      }

      rc = _startParser();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to start parser, rc=%d", rc);
         goto error;
      }

      while (!_parser.isStopped() && !_importer.isStopped())
      {
         ossSleep(100);
      }

      rc = _waitParserStop();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to wait parser stop, rc=%d", rc);
      }

      if (!_importer.isStopped())
      {
         rc = _stopImporter();
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to stop importers, rc=%d", rc);
         }
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Routine::_startImporter(INT32 workerNum)
   {
      INT32 rc = SDB_OK;

      rc = _importer.init(&_options, &_workQueue, &_idleQueue, workerNum);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to init importer, rc=%d", rc);
         goto error;
      }

      rc = _importer.start();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to start importer, rc=%d", rc);
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Routine::_stopImporter()
   {
      INT32 rc = SDB_OK;

      rc = _importer.stop();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to stop importer, rc=%d", rc);
      }

      return rc;
   }

   INT32 Routine::_startParser()
   {
      INT32 rc = SDB_OK;

      rc = _parser.init(&_options, &_workQueue, &_idleQueue);
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to init parser, rc=%d", rc);
         goto error;
      }

      rc = _parser.start();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to start parser, rc=%d", rc);
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   INT32 Routine::_waitParserStop()
   {
      INT32 rc = SDB_OK;

      rc = _parser.stop();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to wait the parser stop");
      }

      return rc;
   }

   void Routine::printStatistics()
   {
      stringstream ss;

      ss << "parsed records: " << _parser.parsedNum() << std::endl
         << "parse failure: " << _parser.failedNum() << std::endl;

      ss << "imported records: " << _importer.parsedNum() << std::endl
         << "import failure: " << _importer.failedNum() << std::endl;

      if (_parser.failedNum() > 0)
      {
         ss << "see " << _parser.logFileName()
            << " for parse failure records" << std::endl;
      }

      if (_importer.failedNum() > 0)
      {
         ss << "see " << _importer.logFileName()
            << " for import failure records" << std::endl;
      }

      string stat = ss.str();

      std::cout << stat;
      PD_LOG(PDEVENT, stat.c_str());
   }
}

