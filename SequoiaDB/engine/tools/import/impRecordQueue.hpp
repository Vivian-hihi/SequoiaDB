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

   Source File Name = impRecordParser.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef IMP_RECORD_QUEUE_HPP__
#define IMP_RECORD_QUEUE_HPP__

#include "ossQueue.hpp"
#include "ossUtil.h"
#include "../client/bson/bson.h"
#include "pd.hpp"

namespace import
{
   template<typename E>
   class Array: public SDBObject
   {
   public:
      Array()
      {
         _array = NULL;
         _capacity = 0;
         _size = 0;
      }

      INT32 init(INT32 capacity)
      {
         SDB_ASSERT(NULL == _array, "already inited");

         _array = (E*)SDB_OSS_MALLOC(sizeof(E) * capacity);
         if (NULL == _array)
         {
            return SDB_OOM;
         }
         ossMemset(_array, 0, sizeof(E) * capacity);
         _capacity = capacity;

         return SDB_OK;
      }

      void free()
      {
         SAFE_OSS_FREE(_array);
      }

      inline E* array() const
      {
         return _array;
      }

      inline INT32 capacity() const
      {
         return _capacity;
      }

      inline INT32 size() const
      {
         return _size;
      }

      inline void reset()
      {
         _size = 0;
      }

      inline void inc()
      {
         _size++;
         SDB_ASSERT(_size <= _capacity, "out of range");
      }

      inline void setSize(INT32 size)
      {
         SDB_ASSERT(size >= 0 && size <= _capacity, "out of range");
         _size = size;
      }

      inline BOOLEAN empty()
      {
         return _size == 0;
      }

      inline BOOLEAN full()
      {
         return _size == _capacity;
      }

      inline E& operator[](INT32 i)
      {
         SDB_ASSERT(i >=0 && i < _capacity, "out of range");
         return _array[i];
      }

   private:
      E*    _array;
      INT32 _capacity;
      INT32 _size;
   };

   typedef class Array<bson*> RecordArray;
   typedef class ossQueue<RecordArray> RecordQueue;
}

#endif /* IMP_RECORD_QUEUE_HPP__ */

