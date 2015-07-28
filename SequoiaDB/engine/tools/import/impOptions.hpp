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

   Source File Name = impOptions.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/7/2015  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef IMP_OPTIONS_HPP_
#define IMP_OPTIONS_HPP_

#include "core.hpp"
#include "oss.hpp"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

namespace import
{

   enum INPUT_TYPE
   {
      INPUT_FILE = 0,
      INPUT_STDIN,
      INPUT_EXEC
   };

   enum INPUT_FORMAT
   {
      FORMAT_CSV = 0,
      FORMAT_JSON
   };

   class Options: public SDBObject
   {
   public:
      Options();
      ~Options();
      INT32 parse(INT32 argc, CHAR* argv[]);
      void printHelpInfo();
      BOOLEAN hasHelp();
      BOOLEAN hasVersion();

      /* general */
      inline const string& hostname() const { return _hostname; }
      inline const string& svcname() const { return _svcname; }
      inline const string& user() const { return _user; }
      inline const string& password() const { return _password; }
      inline const string& csname() const { return _csName; }
      inline const string& clname() const { return _clName; }
      inline BOOLEAN errorStop() const { return _errorStop; }
      inline BOOLEAN useSSL() const { return _useSSL; }

      /* import */
      inline INT32 batchSize() const { return _batchSize; }
      inline INT32 jobs() const { return _jobs; }

      /* input */
      inline const string& file() const { return _file; }
      inline const string& exec() const { return _exec; }
      inline const string& execArgs() const { return _execArgs; }
      inline INPUT_TYPE inputType() const { return _inputType; }
      inline INPUT_FORMAT inputFormat() const { return _inputFormat; }
      inline BOOLEAN linePriority() const { return _linePriority; }
      inline const string& recordDelimiter() const { return _recordDelimiter; }
      inline BOOLEAN force() const { return _force; }

      /* csv */
      inline const string& stringDelimiter() const { return _stringDelimiter; }
      inline const string& fieldDelimiter() const { return _fieldDelimiter; }
      inline const string& fields() const { return _fields; }
      inline BOOLEAN hasHeaderLine() const { return _hasHeaderLine; }
      inline BOOLEAN autoAddField() const { return _autoAddField; }
      inline BOOLEAN autoCompletion() const { return _autoCompletion; }

      /* hidden */
      inline INT32 bufferSize() const { return _bufferSize; }
      inline BOOLEAN dryRun() const { return _dryRun; }
      inline BOOLEAN verbose() const { return _verbose; }
      inline BOOLEAN enableSharding() const { return _enableSharding; }
      inline BOOLEAN enableCoord() const { return _enableCoord; }

   private:
      BOOLEAN has(CHAR* option);
      template<typename T>
      T get(CHAR* option);
      INT32 setOptions();

   private:
      po::options_description    _allDesc;
      po::variables_map          _vm;
      BOOLEAN                    _parsed;

      /* general */
      string         _hostname;
      string         _svcname;
      string         _user;
      string         _password;
      string         _csName;
      string         _clName;
      BOOLEAN        _errorStop;
      BOOLEAN        _useSSL;

      /* import */
      INT32          _batchSize;
      INT32          _jobs;

      /* input */
      string         _file;
      string         _exec;
      string         _execArgs;
      INPUT_TYPE     _inputType;
      INPUT_FORMAT   _inputFormat;
      BOOLEAN        _linePriority;
      string         _recordDelimiter;
      BOOLEAN        _force;

      /* csv */
      string         _stringDelimiter;
      string         _fieldDelimiter;
      string         _fields;
      BOOLEAN        _hasHeaderLine;
      BOOLEAN        _autoAddField;
      BOOLEAN        _autoCompletion;

      /* hidden */
      INT32          _bufferSize;
      BOOLEAN        _dryRun;
      BOOLEAN        _verbose;
      BOOLEAN        _enableSharding;
      BOOLEAN        _enableCoord;
   };
}

#endif /* IMP_OPTIONS_HPP_ */
