/*******************************************************************************

   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

*******************************************************************************/

#include "core.hpp"
#include "oss.hpp"
#include "ossTypes.hpp"
#include "ossUtil.hpp"
#include "sptCommon.hpp"

#include <set>
#include <string>
#include <map>

using namespace std;

#ifndef PATH_LEN
#define PATH_LEN 255
#endif

class manHelp : public SDBObject
{
public:
   static manHelp* createInstance( const CHAR *path ) ;
   static void destroyInstance() ;
   ~manHelp() ;
   INT32 getFileHelp( const CHAR *cmd ) ;
   INT32 getFileHelp( const CHAR *category, const CHAR *cmd ) ;
private:
   typedef std::set<string> sset ;
   typedef std::map<string, string> ssmap ;
   manHelp( const CHAR* path ) ;
   INT32 scanFile() ;
   ssmap& getCategoryMap( const CHAR *category ) ;
   INT32 displayMethod( const CHAR *category ) ;
   INT32 displayManual( const CHAR *category, const CHAR *cmd ) ;

   static manHelp *m_pInstance ;
   sset nset ;
   CHAR filePath[ OSS_MAX_PATHSIZE + 1 ] ;
   ssmap mdb ;
   ssmap mcs ;
   ssmap mcl ;
   ssmap mrg ;
   ssmap mnode ;
   ssmap mcursor ;
   ssmap mclcount;
   ssmap mempty ;

} ;


