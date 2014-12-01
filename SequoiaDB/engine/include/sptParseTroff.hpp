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

   Source File Name = sptParseTroff.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          11/28/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SPTPARSETROFF_HPP__
#define SPTPARSETROFF_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossTypes.hpp"
#include "ossUtil.hpp"

#include <set>
#include <string>
#include <map>

using namespace std;


class manHelp : public SDBObject
{
public:
   static manHelp& getInstance( const CHAR *path ) ;
   INT32 getFileHelp( const CHAR *cmd ) ;
   INT32 getFileHelp( const CHAR *category, const CHAR *cmd ) ;
private:
   typedef std::set<string> sset ;
   typedef std::map<string, string> ssmap ;

   manHelp( const CHAR* path ) ;
   manHelp( const manHelp& ) ;
   manHelp& operator=( const manHelp& ) ;
   ~manHelp() ;
   INT32 scanFile() ;
   ssmap& getCategoryMap( const CHAR *category ) ;
   INT32 displayMethod( const CHAR *category ) ;
   INT32 displayManual( const CHAR *category, const CHAR *cmd ) ;

   sset _nset ;
   ssmap _nmap ;
   CHAR _filePath[ OSS_MAX_PATHSIZE + 1 ] ;
   ssmap _mdb ;
   ssmap _mcs ;
   ssmap _mcl ;
   ssmap _mrg ;
   ssmap _mnode ;
   ssmap _mcursor ;
   ssmap _mclcount ;
   ssmap _mdomain ;
   ssmap _mempty ;
   ssmap _moma ;
   BOOLEAN troffFileNotEixt ;
} ;

#endif // SPTPARSETROFF_HPP__
