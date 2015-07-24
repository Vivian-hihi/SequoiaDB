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

   Source File Name = impRoutine.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef IMP_ROUTINE_HPP_
#define IMP_ROUTINE_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "impOptions.hpp"
#include "impWorker.hpp"
#include "impCoord.hpp"
#include "impRecordQueue.hpp"
#include "impLogFile.hpp"
#include "ossAtomic.hpp"
#include <queue>

using namespace std;

namespace import
{
   class Routine
   {
   public:
      Routine(const Options& options);
      ~Routine();
      INT32 startParser();
      INT32 waitParserStop();
      inline BOOLEAN isParserStopped() const 
      { return _parserStopped; }

      INT32 startImporters(INT32 num);
      INT32 stopImporters();
      inline BOOLEAN isImportersStopped()
      { return 0 == _importesLivingNum.fetch(); }

      void printStatistics();

   private:
      const Options&    _options;
      RecordQueue       _workQueue;
      RecordQueue       _idleQueue;

      // coords
      Coords            _coords;

      // parser
      Worker*           _parser;
      BOOLEAN           _parserStopped;
      LogFile           _parserLogFile;

      // importers
      queue<Worker*>    _importers;
      ossAtomicSigned32 _importesLivingNum;
      LogFile           _importerLogFile;

      // statistics
      INT64             _parsedNum;
      INT64             _parseFailureNum;
      ossAtomicSigned64 _importedNum;
      ossAtomicSigned64 _importFailureNum;
   };
}

#endif /* IMP_ROUTINE_HPP_ */
