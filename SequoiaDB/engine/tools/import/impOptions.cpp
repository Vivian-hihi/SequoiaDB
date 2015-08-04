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

   Source File Name = impOptions.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "impOptions.hpp"
#include "utilParam.hpp"
#include "ossUtil.h"
#include "pd.hpp"
#include <iostream>

using namespace engine;
using namespace std;

namespace import
{
   #define IMP_OPTION_HELP              "help"
   #define IMP_OPTION_VERSION           "version"
   #define IMP_OPTION_HOSTNAME          "hostname"
   #define IMP_OPTION_SVCNAME           "svcname"
   #define IMP_OPTION_USER              "user"
   #define IMP_OPTION_PASSWORD          "password"
   #define IMP_OPTION_COLLECTSPACE      "csname"
   #define IMP_OPTION_COLLECTION        "clname"
   #define IMP_OPTION_DELCHAR           "delchar"
   #define IMP_OPTION_DELFIELD          "delfield"
   #define IMP_OPTION_DELRECORD         "delrecord"
   #define IMP_OPTION_FILENAME          "file"
   #define IMP_OPTION_EXTRA             "extra"
   #define IMP_OPTION_SPARSE            "sparse"
   #define IMP_OPTION_LINEPRIORITY      "linepriority"
   #define IMP_OPTION_FIELDS            "fields"
   #define IMP_OPTION_HEADERLINE        "headerline"
   #define IMP_OPTION_TYPE              "type"
   #define IMP_OPTION_BATCHSIZE         "insertnum"
   #define IMP_OPTION_ERRORSTOP         "errorstop"
   #define IMP_OPTION_FORCE             "force"
   #define IMP_OPTION_SSL               "ssl"
   #define IMP_OPTION_JOBS              "jobs"
   #define IMP_OPTION_BUFFERSIZE        "buffer"
   #define IMP_OPTION_DRYRUN            "dryrun"
   #define IMP_OPTION_VERBOSE           "verbose"
   #define IMP_OPTION_EXEC              "exec"
   #define IMP_OPTION_SHARDING          "sharding"
   #define IMP_OPTION_COORD             "coord"
   #define IMP_OPTION_HELPFUL           "helpful"
   #define IMP_OPTION_RECORDSMEM        "recordsmem"

   #define IMP_EXPLAIN_HELP             "print help information"
   #define IMP_EXPLAIN_VERSION          "print version"
   #define IMP_EXPLAIN_HOSTNAME         "host name, default: localhost"
   #define IMP_EXPLAIN_SVCNAME          "service name, default: 11810"
   #define IMP_EXPLAIN_USER             "username"
   #define IMP_EXPLAIN_PASSWORD         "password"
   #define IMP_EXPLAIN_DELCHAR          "string delimiter, default: '\"' ( csv only )"
   #define IMP_EXPLAIN_DELFIELD         "field delimiter, default: ',' ( csv only )"
   #define IMP_EXPLAIN_DELRECORD        "record delimiter, default: '\\n'"
   #define IMP_EXPLAIN_COLLECTSPACE     "collection space name"
   #define IMP_EXPLAIN_COLLECTION       "collection name"
   #define IMP_EXPLAIN_BATCHSIZE        "batch insert records number, minimun 1, maximum 100000, default: 100"
   #define IMP_EXPLAIN_FILENAME         "input file name, use standard input if both --exec and --file are not specified"
   #define IMP_EXPLAIN_TYPE             "type of record to load, default: csv (json,csv)"
   #define IMP_EXPLAIN_FIELDS           "field name, separated by comma (',')(e.g. \"--fields name,age\"). "\
                                        "field type and default value can be specified for csv input (e.g. \"--fields name string,age int default 18\")"
   #define IMP_EXPLAIN_HEADERLINE       "for csv input, whether the first line defines field name. if --fields is defined, the first line will be ignored if this options is true"
   #define IMP_EXPLAIN_SPARSE           "for csv input, whether to add missing field, default: true"
   #define IMP_EXPLAIN_EXTRA            "for csv input, whether to add missing value, default: false"
   #define IMP_EXPLAIN_LINEPRIORITY     "reverse the priority for record and character delimiter, default: true"
   #define IMP_EXPLAIN_ERRORSTOP        "whether stop by hitting error, default false"
   #define IMP_EXPLAIN_FORCE            "force to insert the records that are not in utf-8 format, default: false"
   #define IMP_EXPLAIN_SSL              "use SSL connection (arg: [true|false], e.g. \"--ssl true\")"
   #define IMP_EXPLAIN_JOBS             "importing job num at once, default is 1"
   #define IMP_EXPLAIN_BUFFER           "set buffer size(unit:MB), default is 64MB"
   #define IMP_EXPLAIN_DRYRUN           "only parse record, don't import to database"
   #define IMP_EXPLAIN_VERBOSE          "print run time details"
   #define IMP_EXPLAIN_EXEC             "execute external program to get data, the program should output data to standard outpupt"
   #define IMP_EXPLAIN_SHARDING         "repackage records by sharding, default is true"
   #define IMP_EXPLAIN_COORD            "find coordinators automatically, default is true"
   #define IMP_EXPLAIN_HELPFUL          "print all options"
   #define IMP_EXPLAIN_RECORDSMEM       "the maximum memory size used by records, the unit is MB, range is [128~8192], default is 1024"

   #define _TYPE(T) po::value<T>()

   #define IMP_DEFAULT_HOSTNAME "localhost"
   #define IMP_DEFAULT_SVCNAME  "11810"

   #define IMP_GENERAL_OPTIONS \
      (IMP_OPTION_HELP",h",             /* no arg */     IMP_EXPLAIN_HELP) \
      (IMP_OPTION_VERSION",V",          /* no arg */     IMP_EXPLAIN_VERSION) \
      (IMP_OPTION_HOSTNAME",s",        _TYPE(string),    IMP_EXPLAIN_HOSTNAME) \
      (IMP_OPTION_SVCNAME",p",         _TYPE(string),    IMP_EXPLAIN_SVCNAME) \
      (IMP_OPTION_USER",u",            _TYPE(string),    IMP_EXPLAIN_USER) \
      (IMP_OPTION_PASSWORD",w",        _TYPE(string),    IMP_EXPLAIN_PASSWORD) \
      (IMP_OPTION_COLLECTSPACE",c",    _TYPE(string),    IMP_EXPLAIN_COLLECTSPACE) \
      (IMP_OPTION_COLLECTION",l",      _TYPE(string),    IMP_EXPLAIN_COLLECTION) \
      (IMP_OPTION_ERRORSTOP,           _TYPE(string),    IMP_EXPLAIN_ERRORSTOP) \
      (IMP_OPTION_SSL,                 _TYPE(string),    IMP_EXPLAIN_SSL) \
      (IMP_OPTION_VERBOSE",v",          /* no arg */     IMP_EXPLAIN_VERBOSE) \

   #define IMP_IMPORT_OPTIONS \
      (IMP_OPTION_BATCHSIZE",n",       _TYPE(INT32),     IMP_EXPLAIN_BATCHSIZE) \
      (IMP_OPTION_JOBS",j",            _TYPE(INT32),     IMP_EXPLAIN_JOBS) \
      (IMP_OPTION_COORD,               _TYPE(string),    IMP_EXPLAIN_COORD) \
      (IMP_OPTION_SHARDING,            _TYPE(string),    IMP_EXPLAIN_SHARDING) \

   #define IMP_INPUT_OPTIONS \
      (IMP_OPTION_FILENAME,            _TYPE(string),    IMP_EXPLAIN_FILENAME) \
      (IMP_OPTION_EXEC,                _TYPE(string),    IMP_EXPLAIN_EXEC) \
      (IMP_OPTION_TYPE,                _TYPE(string),    IMP_EXPLAIN_TYPE) \
      (IMP_OPTION_LINEPRIORITY,        _TYPE(string),    IMP_EXPLAIN_LINEPRIORITY) \
      (IMP_OPTION_DELRECORD",r",       _TYPE(string),    IMP_EXPLAIN_DELRECORD) \
      (IMP_OPTION_FORCE,               _TYPE(string),    IMP_EXPLAIN_FORCE) \

   #define IMP_CSV_OPTIONS \
      (IMP_OPTION_DELCHAR",a",         _TYPE(string),    IMP_EXPLAIN_DELCHAR) \
      (IMP_OPTION_DELFIELD",e",        _TYPE(string),    IMP_EXPLAIN_DELFIELD) \
      (IMP_OPTION_FIELDS,              _TYPE(string),    IMP_EXPLAIN_FIELDS) \
      (IMP_OPTION_HEADERLINE,          _TYPE(string),    IMP_EXPLAIN_HEADERLINE) \
      (IMP_OPTION_SPARSE,              _TYPE(string),    IMP_EXPLAIN_SPARSE) \
      (IMP_OPTION_EXTRA,               _TYPE(string),    IMP_EXPLAIN_EXTRA) \

   #define IMP_HELPFUL_OPTIONS \
      (IMP_OPTION_HELPFUL,              /* no arg */     IMP_EXPLAIN_HELPFUL) \
      (IMP_OPTION_BUFFERSIZE,          _TYPE(INT32),     IMP_EXPLAIN_BUFFER) \
      (IMP_OPTION_DRYRUN,               /* no arg */     IMP_EXPLAIN_DRYRUN) \
      (IMP_OPTION_RECORDSMEM,          _TYPE(INT32),     IMP_EXPLAIN_RECORDSMEM) \


   Options::Options()
   {
      _parsed = FALSE;
      _hostname = IMP_DEFAULT_HOSTNAME;
      _svcname = IMP_DEFAULT_SVCNAME;
      _recordDelimiter = "\n";
      _inputType = INPUT_STDIN;
      _inputFormat = FORMAT_CSV;
      _linePriority = TRUE;
      _errorStop = FALSE;
      _force = FALSE;
      _useSSL = FALSE;
      _verbose = FALSE;

      _batchSize = 100;
      _jobs = 1;
      _enableSharding = TRUE;
      _enableCoord = TRUE;

      _stringDelimiter = "\"";
      _fieldDelimiter = ",";
      _hasHeaderLine = FALSE;
      _autoAddField = TRUE;
      _autoCompletion = FALSE;

      _bufferSize = 64;
      _dryRun = FALSE;
      _recordsMem = (1024 * 1024 * 1024); // 1GB
   }

   Options::~Options()
   {
      _parsed = FALSE;
   }

   INT32 Options::parse(INT32 argc, CHAR* argv[])
   {
      INT32 rc = SDB_OK;

      SDB_ASSERT(!_parsed, "can't parse again");

      _allDesc.add_options()
         IMP_GENERAL_OPTIONS
         IMP_IMPORT_OPTIONS
         IMP_INPUT_OPTIONS
         IMP_CSV_OPTIONS
         IMP_HELPFUL_OPTIONS
      ;

      rc = utilReadCommandLine( argc, argv, _allDesc, _vm, FALSE );
      if (SDB_OK != rc)
      {
         goto error;
      }

      _parsed = TRUE;

      if (has(IMP_OPTION_HELP) ||
          has(IMP_OPTION_VERSION) ||
          has(IMP_OPTION_HELPFUL))
      {
         goto done;
      }

      rc = setOptions();

   done:
      return rc;
   error:
      goto done;
   }

   BOOLEAN Options::hasHelp()
   {
      return has(IMP_OPTION_HELP);
   }

   BOOLEAN Options::hasVersion()
   {
      return has(IMP_OPTION_VERSION);
   }

   BOOLEAN Options::hasHelpful()
   {
      return has(IMP_OPTION_HELPFUL);
   }

   BOOLEAN Options::has(CHAR* option)
   {
      SDB_ASSERT(_parsed, "must be used after parsed");
      SDB_ASSERT(NULL != option, "");

      return (_vm.count(option) > 0);
   }

   template<typename T>
   T Options::get(CHAR* option)
   {
      SDB_ASSERT(_parsed, "must be used after parsed");
      SDB_ASSERT(NULL != option, "");
      return _vm[option].as<T>();
   }

   void Options::printHelpInfo()
   {
      po::options_description general;
      po::options_description import;
      po::options_description input;
      po::options_description csv;

      SDB_ASSERT(_parsed, "must be used after parsed");

      general.add_options()
         IMP_GENERAL_OPTIONS
      ;

      input.add_options()
         IMP_INPUT_OPTIONS
      ;

      csv.add_options()
         IMP_CSV_OPTIONS
      ;

      import.add_options()
         IMP_IMPORT_OPTIONS
      ;

      std::cout << "General Options:" << std::endl;
      std::cout << general << std::endl;

      std::cout << "Input Options:" << std::endl;
      std::cout << input << std::endl;

      std::cout << "CSV Options:" << std::endl;
      std::cout << csv << std::endl;

      std::cout << "Import Options:" << std::endl;
      std::cout << import << std::endl;
   }

   void Options::printHelpfulInfo()
   {
      po::options_description helpful;

      helpful.add_options()
         IMP_HELPFUL_OPTIONS
      ;

      printHelpInfo();

      std::cout << "Helpful Options:" << std::endl;
      std::cout << helpful << std::endl;
   }

   INT32 Options::setOptions()
   {
      INT32 rc = SDB_OK;

      if (has(IMP_OPTION_HOSTNAME))
      {
         _hostname = get<string>(IMP_OPTION_HOSTNAME);
      }

      if (has(IMP_OPTION_SVCNAME))
      {
         _svcname = get<string>(IMP_OPTION_SVCNAME);
      }

      if (has(IMP_OPTION_USER))
      {
         _user = get<string>(IMP_OPTION_USER);
      }

      if (has(IMP_OPTION_PASSWORD))
      {
         _password = get<string>(IMP_OPTION_PASSWORD);
      }

      if (has(IMP_OPTION_COLLECTSPACE))
      {
         _csName = get<string>(IMP_OPTION_COLLECTSPACE);
      }

      if (_csName.empty())
      {
         std::cerr << IMP_OPTION_COLLECTSPACE " must be specified"  << std::endl;
         rc = SDB_INVALIDARG;
         goto error;
      }

      if (has(IMP_OPTION_COLLECTION))
      {
         _clName = get<string>(IMP_OPTION_COLLECTION);
      }

      if (_clName.empty())
      {
         std::cerr << IMP_OPTION_COLLECTION " must be specified" << std::endl;
         rc = SDB_INVALIDARG;
         goto error;
      }

      if (has(IMP_OPTION_FILENAME) && has(IMP_OPTION_EXEC))
      {
         std::cerr << IMP_OPTION_FILENAME 
                   << " can't be specified with " 
                   << IMP_OPTION_EXEC
                   << std::endl;
         rc = SDB_INVALIDARG;
         goto error;
      }

      if (has(IMP_OPTION_FILENAME))
      {
         _file = get<string>(IMP_OPTION_FILENAME);
         _inputType = INPUT_FILE;
      }
      else if (has(IMP_OPTION_EXEC))
      {
         _exec = get<string>(IMP_OPTION_EXEC);
         _inputType = INPUT_EXEC;
      }

      if (has(IMP_OPTION_TYPE))
      {
         string type = get<string>(IMP_OPTION_TYPE);
         if ("csv" == type)
         {
            _inputFormat = FORMAT_CSV;
         }
         else if ("json" == type)
         {
            _inputFormat = FORMAT_JSON;
         }
         else
         {
            std::cerr << "invalid argument of [" IMP_OPTION_TYPE "]: "
                      << type
                      << std::endl;
            rc = SDB_INVALIDARG;
            goto error;
         }
      }

      if (has(IMP_OPTION_DELRECORD))
      {
         _recordDelimiter = get<string>(IMP_OPTION_DELRECORD);
      }

      if (has(IMP_OPTION_BATCHSIZE))
      {
         _batchSize = get<INT32>(IMP_OPTION_BATCHSIZE);
         if (_batchSize <= 0 || _batchSize > 100000)
         {
            std::cerr << IMP_OPTION_BATCHSIZE " is out of range [1-1000000]: "
                      << _batchSize
                      << std::endl;
            rc = SDB_INVALIDARG;
            goto error;
         }
      }

      if (has(IMP_OPTION_LINEPRIORITY))
      {
         string linePriority = get<string>(IMP_OPTION_LINEPRIORITY);
         ossStrToBoolean(linePriority.c_str(), &_linePriority);
      }

      if (has(IMP_OPTION_ERRORSTOP))
      {
         string errorStop = get<string>(IMP_OPTION_ERRORSTOP);
         ossStrToBoolean(errorStop.c_str(), &_errorStop);
      }

      if (has(IMP_OPTION_FORCE))
      {
         string force = get<string>(IMP_OPTION_FORCE);
         ossStrToBoolean(force.c_str(), &_force);
      }

      if (has(IMP_OPTION_JOBS))
      {
         _jobs = get<INT32>(IMP_OPTION_JOBS);
         if (_jobs <= 0 || _jobs > 1000)
         {
            std::cerr << IMP_OPTION_JOBS " is out of range [1, 1000]: "
                      << _jobs
                      << std::endl;
            rc = SDB_INVALIDARG;
            goto error;
         }
      }

      if (has(IMP_OPTION_SSL))
      {
         string ssl = get<string>(IMP_OPTION_SSL);
         ossStrToBoolean(ssl.c_str(), &_useSSL);
      }

      if (has(IMP_OPTION_DELCHAR))
      {
         _stringDelimiter = get<string>(IMP_OPTION_DELCHAR);
      }

      if (has(IMP_OPTION_DELFIELD))
      {
         _fieldDelimiter = get<string>(IMP_OPTION_DELFIELD);
      }

      if (has(IMP_OPTION_FIELDS))
      {
         _fields = get<string>(IMP_OPTION_FIELDS);
      }

      if (has(IMP_OPTION_HEADERLINE))
      {
         string headerline = get<string>(IMP_OPTION_HEADERLINE);
         ossStrToBoolean(headerline.c_str(), &_hasHeaderLine);
      }

      if (FORMAT_CSV == _inputFormat)
      {
         if (_fields.empty() && !_hasHeaderLine)
         {
            std::cerr << IMP_OPTION_FIELDS " or " IMP_OPTION_HEADERLINE
                      << " must be specified when type is csv"
                      << std::endl;
            rc = SDB_INVALIDARG;
            goto error;
         }
      }

      if (has(IMP_OPTION_SPARSE))
      {
         string sparse = get<string>(IMP_OPTION_SPARSE);
         ossStrToBoolean(sparse.c_str(), &_autoAddField);
      }

      if (has(IMP_OPTION_EXTRA))
      {
         string extra = get<string>(IMP_OPTION_EXTRA);
         ossStrToBoolean(extra.c_str(), &_autoCompletion);
      }

      if (has(IMP_OPTION_BUFFERSIZE))
      {
         _bufferSize = get<INT32>(IMP_OPTION_BUFFERSIZE);
         if (_bufferSize < 32 || _bufferSize > 2048)
         {
            std::cerr << IMP_OPTION_BUFFERSIZE " is out of range [32, 2048]: "
                      << _bufferSize
                      << std::endl;
            rc = SDB_INVALIDARG;
            goto error;
         }
      }

      if (has(IMP_OPTION_DRYRUN))
      {
         _dryRun = TRUE;
      }

      if (has(IMP_OPTION_VERBOSE))
      {
         _verbose = TRUE;
      }

      if (has(IMP_OPTION_SHARDING))
      {
         string sharding = get<string>(IMP_OPTION_SHARDING);
         ossStrToBoolean(sharding.c_str(), &_enableSharding);
      }

      if (has(IMP_OPTION_COORD))
      {
         string coord = get<string>(IMP_OPTION_COORD);
         ossStrToBoolean(coord.c_str(), &_enableCoord);
      }

      if (has(IMP_OPTION_RECORDSMEM))
      {
         INT32 recordsMem = get<INT32>(IMP_OPTION_RECORDSMEM);
         if (recordsMem < 128 || recordsMem > 8192)
         {
            std::cerr << IMP_OPTION_RECORDSMEM " is out of range [128, 8192]: "
                      << recordsMem
                      << std::endl;
            rc = SDB_INVALIDARG;
            goto error;
         }

         _recordsMem = recordsMem * 1024 * 1024; // convert MB to Byte
      }

   done:
      return rc;
   error:
      goto done;
   }
}
