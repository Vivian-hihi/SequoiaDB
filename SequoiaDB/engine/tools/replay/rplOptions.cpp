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

   Source File Name = rplOptions.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/9/2016  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "rplOptions.hpp"
#include "../util/fromjson.hpp"
#include "ossUtil.hpp"
#include "ossFile.hpp"
#include "utilStr.hpp"
#include "pd.hpp"
#include <iostream>

namespace replay
{
   #define RPL_OPTION_HELP              "help"
   #define RPL_OPTION_VERSION           "version"
   #define RPL_OPTION_HOST              "hostname"
   #define RPL_OPTION_SVC               "svcname"
   #define RPL_OPTION_USER              "user"
   #define RPL_OPTION_PASSWD            "password"
   #define RPL_OPTION_PATH              "path"
   #define RPL_OPTION_FILTER            "filter"
   #define RPL_OPTION_DUMP              "dump"
   #define RPL_OPTION_DELETE            "delete"
   #define RPL_OPTION_HOLD              "hold"
   #define RPL_OPTION_HELPFULL          "helpfull"
   #define RPL_OPTION_DEBUG             "debug"

   #define RPL_EXPLAIN_HELP             "print help information"
   #define RPL_EXPLAIN_VERSION          "print version"
   #define RPL_EXPLAIN_HOST             "sequoiadb hostname"
   #define RPL_EXPLAIN_SVC              "service name"
   #define RPL_EXPLAIN_USER             "username"
   #define RPL_EXPLAIN_PASSWD           "password"
   #define RPL_EXPLAIN_PATH             "archivelog directory or file path"
   #define RPL_EXPLAIN_FILTER           "log filtering rule"
   #define RPL_EXPLAIN_DUMP             "dump log only, default is false"
   #define RPL_EXPLAIN_DELETE           "delete log file after replay, " \
                                        "default is false"
   #define RPL_EXPLAIN_HOLD             "continuously find and replay log files, " \
                                        "default is false"
   #define RPL_EXPLAIN_HELPFULL         "print all options"
   #define RPL_EXPLAIN_DEBUG            "log debug info"

   #define _TYPE(T) utilOptType(T)

   Options::Options()
   {
      _pathType = SDB_OSS_UNK;
      _dump = FALSE;
      _delete = FALSE;
      _hold = FALSE;
      _debug = FALSE;
   }

   Options::~Options()
   {
   }

   INT32 Options::parse(INT32 argc, CHAR* argv[])
   {
      INT32 rc = SDB_OK;

      addOptions("General Options")
         (RPL_OPTION_HELP",h",       /* no arg */     RPL_EXPLAIN_HELP)
         (RPL_OPTION_VERSION",V",    /* no arg */     RPL_EXPLAIN_VERSION)
         (RPL_OPTION_HOST,          _TYPE(string),    RPL_EXPLAIN_HOST)
         (RPL_OPTION_SVC,           _TYPE(string),    RPL_EXPLAIN_SVC)
         (RPL_OPTION_USER,          _TYPE(string),    RPL_EXPLAIN_USER)
         (RPL_OPTION_PASSWD,        _TYPE(string),    RPL_EXPLAIN_PASSWD)
         (RPL_OPTION_PATH,          _TYPE(string),    RPL_EXPLAIN_PATH)
         (RPL_OPTION_FILTER,        _TYPE(string),    RPL_EXPLAIN_FILTER)
         (RPL_OPTION_DUMP,          _TYPE(string),    RPL_EXPLAIN_DUMP)
         (RPL_OPTION_DELETE,        _TYPE(string),    RPL_EXPLAIN_DELETE)
         (RPL_OPTION_HOLD,          _TYPE(string),    RPL_EXPLAIN_HOLD)
      ;

      addOptions("Helpfull Options", TRUE)
         (RPL_OPTION_HELPFULL,       /* no arg */     RPL_EXPLAIN_HELPFULL)
         (RPL_OPTION_DEBUG,          /* no arg */     RPL_EXPLAIN_DEBUG)
      ;

      rc = engine::utilOptions::parse(argc, argv);
      if (SDB_OK != rc)
      {
         goto error;
      }

      if (has(RPL_OPTION_HELP) ||
          has(RPL_OPTION_VERSION) ||
          has(RPL_OPTION_HELPFULL))
      {
         goto done;
      }

      rc = setOptions() ;
      if (SDB_OK != rc)
      {
         goto error;
      }

   done:
      return rc;
   error:
      goto done;
   }

   void Options::printHelp()
   {
      print();
   }

   void Options::printHelpfull()
   {
      print(TRUE);
   }

   BOOLEAN Options::hasHelp()
   {
      return has(RPL_OPTION_HELP);
   }

   BOOLEAN Options::hasVersion()
   {
      return has(RPL_OPTION_VERSION);
   }

   BOOLEAN Options::hasHelpfull()
   {
      return has(RPL_OPTION_HELPFULL);
   }

   string Options::buildCmd(INT32 argc, CHAR* argv[])
   {
      stringstream ss;

      for (INT32 i = 0; i < argc; i++)
      {
         if (argv[i] == string("--"RPL_OPTION_PASSWD))
         {
            i++; // ignore password
            continue;
         }

         if (i > 0 && argv[i - 1] == string("--"RPL_OPTION_FILTER))
         {
            ss << "'" << argv[i] << "'" << " ";
         }
         else
         {
            ss << argv[i] << " ";
         }
      }

      return ss.str();
   }

   INT32 Options::setOptions()
   {
      INT32 rc = SDB_OK;

      if (has(RPL_OPTION_DUMP))
      {
         string dump = get<string>(RPL_OPTION_DUMP);
         ossStrToBoolean(dump.c_str(), &_dump);
      }

      if (has(RPL_OPTION_HOST))
      {
         _hostName = get<string>(RPL_OPTION_HOST);
      }
      else if (!_dump)
      {
         std::cerr << "Missing argument: " << RPL_OPTION_HOST << std::endl;
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "Missing argument: %s", RPL_OPTION_HOST);
         goto error;
      }

      if (has(RPL_OPTION_SVC))
      {
         _serviceName = get<string>(RPL_OPTION_SVC);
         if (!engine::utilStrIsDigit(_serviceName))
         {
            std::cerr << "Invalid argument: " << RPL_OPTION_SVC << std::endl;
            rc = SDB_INVALIDARG;
            PD_LOG(PDERROR, "Invalid argument: %s[%s]",
                   RPL_OPTION_SVC, _serviceName.c_str());
            goto error;
         }
      }
      else if (!_dump)
      {
         std::cerr << "Missing argument: " << RPL_OPTION_SVC << std::endl;
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "Missing argument: %s", RPL_OPTION_SVC);
         goto error;
      }

      if (has(RPL_OPTION_USER))
      {
         _user = get<string>(RPL_OPTION_USER);
      }

      if (has(RPL_OPTION_PASSWD))
      {
         _password = get<string>(RPL_OPTION_PASSWD);
      }

      if (has(RPL_OPTION_PATH))
      {
         _path = get<string>(RPL_OPTION_PATH);

         BOOLEAN exist = FALSE;
         rc = engine::ossFile::exists(_path, exist);
         if (SDB_OK != rc)
         {
            std::cerr << "Failed to access path: " << _path << std::endl;
            PD_LOG(PDERROR, "Failed to access path[%s], rc=%d",
                   _path.c_str(), rc);
            goto error;
         }

         if (!exist)
         {
            rc = SDB_FNE;
            std::cerr << "Path is not existing" << std::endl;
            PD_LOG(PDERROR, "Path[%s] is not existing, rc=%d",
                   _path.c_str(), rc);
            goto error;
         }

         rc = ossGetPathType(_path.c_str(), &_pathType);
         if (SDB_OK != rc)
         {
            std::cerr << "Failed to get path type" << std::endl;
            PD_LOG(PDERROR, "Failed to get path[%s] type, rc=%d",
                   _path.c_str(), rc);
            goto error;
         }

         if (SDB_OSS_FIL != _pathType && SDB_OSS_DIR != _pathType)
         {
            rc = SDB_INVALIDARG;
            std::cerr << "Path is not file or directory" << std::endl;
            PD_LOG(PDERROR, "Path[%s] is not file or directory, rc=%d",
                   _path.c_str(), rc);
            goto error;
         }
      }
      else
      {
         std::cerr << "Missing argument: " << RPL_OPTION_PATH << std::endl;
         rc = SDB_INVALIDARG;
         PD_LOG(PDERROR, "Missing argument: %s", RPL_OPTION_PATH);
         goto error;
      }

      if (has(RPL_OPTION_FILTER))
      {
         string filter = get<string>(RPL_OPTION_FILTER);
         rc = bson::fromjson(filter, _filter) ;
         if (SDB_OK != rc)
         {
            std::cerr << "invalid argument: " << RPL_OPTION_FILTER << std::endl;
            PD_LOG( PDERROR, "Failed to convert json[%s] to bson, rc=%d",
                    filter.c_str(), rc ) ;
            goto error ;
         }
      }

      if (has(RPL_OPTION_DELETE))
      {
         string del = get<string>(RPL_OPTION_DELETE);
         ossStrToBoolean(del.c_str(), &_delete);
      }

      if (has(RPL_OPTION_HOLD))
      {
         string hold = get<string>(RPL_OPTION_HOLD);
         ossStrToBoolean(hold.c_str(), &_hold);
      }

      if (has(RPL_OPTION_DEBUG))
      {
         string debug = get<string>(RPL_OPTION_DEBUG);
         ossStrToBoolean(debug.c_str(), &_debug);
      }

   done: 
      return rc;
   error:
      goto done;
   }
}
