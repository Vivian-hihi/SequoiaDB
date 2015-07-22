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

   Source File Name = sdbImport.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "impOptions.hpp"
#include "impRoutine.hpp"
#include "impUtil.hpp"
#include "ossVer.h"
#include "pd.hpp"

using namespace import;

#define IMP_LOG_PATH "sdbimport.log"

int main(int argc, char* argv[])
{
   Options options;
   INT32 rc = SDB_OK;

   sdbEnablePD(IMP_LOG_PATH);
   setPDLevel(PDWARNING);

   rc = options.parse(argc, argv);
   if (SDB_OK != rc)
   {
      PD_LOG(PDERROR, "failed to parse options, rc=%d", rc);
      goto error;
   }

   if (options.hasHelp())
   {
      options.printHelpInfo();
      goto done;
   }

   if (options.hasVersion())
   {
      ossPrintVersion("sdbimport");
      goto done;
   }

   {
      Routine routine(options);

      rc = routine.startParser();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to start parser, rc=%d", rc);
         goto error;
      }

      rc = routine.startImporters(options.jobs());
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to start importers, rc=%d", rc);
         goto error;
      }

      while (!routine.isParserStopped() && !routine.isImportersStopped())
      {
         ossSleep(100);
      }

      rc = routine.waitParserStop();
      if (SDB_OK != rc)
      {
         PD_LOG(PDERROR, "failed to wait parser stop, rc=%d", rc);
      }

      if (!routine.isImportersStopped())
      {
         rc = routine.stopImporters();
         if (SDB_OK != rc)
         {
            PD_LOG(PDERROR, "failed to stop importers, rc=%d", rc);
         }
      }

      routine.printStatistics();
   }

done:
   return RC2ShellRC(rc);
error:
   goto done;
}

