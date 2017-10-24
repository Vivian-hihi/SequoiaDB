/*******************************************************************************


   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = seAdptUtil.cpp

   Descriptive Name = Search Engine Adapter Util.

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/01/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#include "seAdptUtil.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"

namespace engine
{
   _seAdptNameParser::_seAdptNameParser()
   {
      ossMemset( _targetIdxName, 0, SDB_SEADPT_MAX_IDXNAME_SZ + 1 ) ;
   }

   _seAdptNameParser::~_seAdptNameParser()
   {
   }

   INT32 _seAdptNameParser::parse( const CHAR *clFullName, const CHAR *idxName )
   {
      INT32 rc = SDB_OK ;
      const CHAR dot = '.' ;
      const CHAR *pos1 = NULL ;
      const CHAR *pos2 = NULL ;
      UINT32 writePos = 0 ;
      UINT32 writeLen = 0 ;

      if ( !clFullName || !idxName )
      {
         PD_LOG( PDERROR, "Collection or index name is empty" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      pos1 = ossStrchr( clFullName, dot ) ;
      pos2 = ossStrrchr( clFullName, dot ) ;
      // Make sure there is one, and only one, dot at the middle of the name.
      if ( !pos1 || ( pos1 != pos2 ) || ( pos1 == clFullName )
           || ( pos1 == ( clFullName + ossStrlen( clFullName ) - 1 ) ) )
      {
                  PD_LOG( PDERROR, "Collection name format is wrong: %s",
                 clFullName ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( ossStrlen( clFullName ) - 1 + ossStrlen( idxName )
           > SDB_SEADPT_MAX_IDXNAME_SZ )
      {
         PD_LOG( PDERROR, "Names are too long, collection: %s, index: %s",
                 clFullName, idxName ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      writeLen = pos1 - clFullName ;
      ossStrncpy( _targetIdxName, clFullName, writeLen ) ;
      writePos += writeLen ;
      // Skip the dot.
      writeLen = ossStrlen( clFullName ) - ( pos1 + 1 - clFullName ) ;
      ossStrncat( _targetIdxName + writePos, pos1 + 1, writeLen ) ;
      writePos += writeLen ;
      ossStrncat( _targetIdxName + writePos, idxName, ossStrlen( idxName ) ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _seAdptNameParser::reset()
   {
      ossMemset( _targetIdxName, 0, SDB_SEADPT_MAX_IDXNAME_SZ + 1 ) ;
   }

   const CHAR* _seAdptNameParser::getTargetIdxName()
   {
      return _targetIdxName ;
   }
}

