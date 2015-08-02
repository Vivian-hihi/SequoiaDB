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

   Source File Name = impSharding.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "impSharding.hpp"
#include "pd.hpp"
#include <iostream>
#include <sstream>

namespace import
{
   static INT32 _getFreeRecordArray(RecordQueue& queue, INT32 capacity,
                                    RecordArray& recordArray)
   {
      RecordArray array;
      INT32 rc = SDB_OK;

      if (queue.try_pop(recordArray))
      {
         INT32 num = recordArray.size();
         for (INT32 i = 0; i < num; i++)
         {
            bson* obj = recordArray[i];
            bson_destroy(obj);
            SDB_OSS_FREE(obj);
            recordArray[i] = NULL;
         }
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

   void _shardingRoutine(WorkerArgs* args)
   {
      Sharding* self = (Sharding*)args;
      RecordArray records;
      INT32 rc = SDB_OK;
      INT32 shardingCount = 0;

      SDB_ASSERT(NULL != args, "arg can't be NULL");

      Options* options = self->_options;
      RecordQueue* inQueue = self->_inQueue;
      RecordQueue* outQueue = self->_outQueue;
      RecordQueue* idleQueue = self->_idleQueue;
      RecordSharding* sharding = &(self->_sharding);
      ShardingGroups* groups = &(self->_groups);

      if (options->verbose())
      {
         stringstream ss;
         ss << "sharding started with "
            << sharding->getGroupNum() << " groups..."
            << std::endl;

         std::cout << ss.str();
      }

      for(;;)
      {
         INT32 size;
         inQueue->wait_and_pop(records);

         if (records.empty())
         {
            // stop
            break;
         }

         size = records.size();
         for (INT32 i = 0; i < size; i++)
         {
            bson* record = records[i];
            SubShardingGroups::iterator it;
            string cl;
            SubShardingGroups* subGroups = NULL;
            UINT32 groupId = 0;

            rc = sharding->getGroupByRecord(record, cl, groupId);
            if (SDB_OK != rc)
            {
               PD_LOG(PDERROR, "failed to get group by record, rc=%d", rc);
               idleQueue->push(records);
               goto error;
            }

            subGroups = &((*groups)[cl]);

            it = subGroups->find(groupId);
            if (it != subGroups->end())
            {
               // find the group
               RecordArray& array = it->second;

               SDB_ASSERT(!array.full(), "record array can't be full");

               array[array.size()] = record;
               array.inc();
               if (array.full())
               {
                  outQueue->push(array);
                  subGroups->erase(it);
               }
            }
            else
            {
               // add new group
               RecordArray array;

               rc = _getFreeRecordArray(*idleQueue, options->batchSize(), array);
               if (SDB_OK != rc)
               {
                  PD_LOG(PDERROR, "failed to get free record array, rc=%d", rc);
                  idleQueue->push(records);
                  goto error;
               }

               array[array.size()] = record;
               array.inc();
               if (array.full())
               {
                  outQueue->push(array);
               }
               else
               {
                  (*subGroups)[groupId] = array;
               }
            }

            shardingCount++;
         }
      }

      if (!groups->empty())
      {
         for (ShardingGroups::iterator it = groups->begin();
              it != groups->end();
              it++)
         {
            SubShardingGroups* subGroups = &(it->second);
            for (SubShardingGroups::iterator subIt = subGroups->begin();
                 subIt != subGroups->end();
                 subIt++)
            {
               RecordArray array = subIt->second;
               outQueue->push(array);
            }

            subGroups->clear();
         }

         groups->clear();
      }

   done:
      self->_stopped = TRUE;
      if (options->verbose())
      {
         stringstream ss;
         ss << "sharding stopped, sharding records "
            << shardingCount << "."
            << std::endl;

         std::cout << ss.str();
      }
      return;
   error:
      goto done;
   }

   Sharding::Sharding()
   {
      _options = NULL;
      _inQueue = NULL;
      _outQueue = NULL;
      _idleQueue = NULL;
      _inited = FALSE;
      _worker = NULL;
      _stopped = TRUE;
   }

   Sharding::~Sharding()
   {
      SAFE_OSS_DELETE(_worker);
   }

   INT32 Sharding::init(Options* options,
                       RecordQueue* inQueue,
                       RecordQueue* outQueue,
                       RecordQueue* idleQueue)
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(!_inited, "can't init again");
      SDB_ASSERT(NULL != options, "options can't be NULL");
      SDB_ASSERT(NULL != inQueue, "inQueue can't be NULL");
      SDB_ASSERT(NULL != outQueue, "outQueue can't be NULL");
      SDB_ASSERT(NULL != idleQueue, "idleQueue can't be NULL");

      _options = options;
      _inQueue = inQueue;
      _outQueue = outQueue;
      _idleQueue = idleQueue;

      rc = _sharding.init(_options->hostname(),
                          _options->svcname(),
                          _options->user(),
                          _options->password(),
                          _options->csname(),
                          _options->clname(),
                          _options->useSSL());
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to init sharding, rc=%d", rc);
         goto error;
      }

      SDB_ASSERT(_sharding.getGroupNum() >= 0,
                 "groupNum must be greater than or equals 0");

      if (_sharding.getGroupNum() <= 1 ||
          !_options->enableSharding() ||
          _options->batchSize() <= 1)
      {
         // no need to do anything
         _inited = TRUE;
         goto done;
      }

      _worker = SDB_OSS_NEW Worker(_shardingRoutine, this);
      if (NULL == _worker)
      {
         rc = SDB_OOM;
         PD_LOG(PDERROR, "failed to create sharding Worker object");
         goto error;
      }

      _inited = TRUE;

   done:
      return rc;
   error:
      goto done;
   }

   BOOLEAN Sharding::needSharding() const
   {
      SDB_ASSERT(_inited, "must be inited");

      return (_options->enableSharding() &&
              _options->batchSize() > 1 &&
              _sharding.getGroupNum() > 1);
   }

   INT32 Sharding::getGroupNum() const
   {
      SDB_ASSERT(_inited, "must be inited");

      return _sharding.getGroupNum();
   }

   INT32 Sharding::start()
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

   INT32 Sharding::stop()
   {
      INT32 rc = SDB_OK;
      RecordArray empty;

      SDB_ASSERT(_inited, "must be inited");
      SDB_ASSERT(NULL != _worker, "_worker can't be NULL");

      // push empty RecordArray as stop signal
      _inQueue->push(empty);

      rc = _worker->waitStop();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to wait the sharding stop");
      }

      SAFE_OSS_DELETE(_worker);
      return rc;
   }
}
