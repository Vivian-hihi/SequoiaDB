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

   Source File Name = rplOptions.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          7/9/2016  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef REPLAY_OPTIONS_HPP_
#define REPLAY_OPTIONS_HPP_

#include "ossTypes.hpp"
#include "ossIO.hpp"
#include "utilOptions.hpp"
#include "../bson/bsonobj.h"

using namespace std;
using namespace bson;

namespace replay
{
   class Options: public engine::utilOptions
   {
   public:
      Options();
      ~Options();
      INT32 parse(INT32 argc, CHAR* argv[]);
      void  printHelp();
      void  printHelpfull();
      BOOLEAN hasHelp();
      BOOLEAN hasVersion();
      BOOLEAN hasHelpfull();
      string  buildCmd(INT32 argc, CHAR* argv[]);

   public:
      OSS_INLINE const string& hostName() const { return _hostName; }
      OSS_INLINE const string& serviceName() const { return _serviceName; }
      OSS_INLINE const string& user() const { return _user; }
      OSS_INLINE const string& password() const { return _password; }
      OSS_INLINE const string& path() const { return _path; }
      OSS_INLINE SDB_OSS_FILETYPE pathType() const { return _pathType; }
      OSS_INLINE const BSONObj& filter() const { return _filter; }
      OSS_INLINE BOOLEAN dump() const { return _dump; }
      OSS_INLINE BOOLEAN remove() const { return _delete; }
      OSS_INLINE BOOLEAN hold() const { return _hold; }
      OSS_INLINE BOOLEAN debug() const { return _debug; }

   private:
      INT32 setOptions();

   private:
      string            _hostName;
      string            _serviceName;
      string            _user;
      string            _password;
      string            _path;
      SDB_OSS_FILETYPE  _pathType;
      BSONObj           _filter;
      BOOLEAN           _dump;
      BOOLEAN           _delete;
      BOOLEAN           _hold;
      BOOLEAN           _debug;
   };
}

#endif /* REPLAY_OPTIONS_HPP_ */
