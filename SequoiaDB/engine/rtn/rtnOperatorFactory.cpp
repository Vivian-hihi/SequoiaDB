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

   Source File Name = rtnOperatorFactory.cpp

   Descriptive Name = Factory for rtn operators.

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   DMS storage unit and its methods.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          14/04/2017  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#include "rtnOperatorFactory.hpp"
#include "rtnQueryOperator.hpp"
#include "rtnSimpleCondParser.hpp"

namespace engine
{
   _rtnOperatorFactory::_rtnOperatorFactory()
   {
   }

   _rtnOperatorFactory::~_rtnOperatorFactory()
   {
   }

   INT32 _rtnOperatorFactory::create( const rtnQueryOptions &options,
                                      rtnOperator **pOperator )
   {
      INT32 rc = SDB_OK ;
      rtnOperator *opr = NULL ;
      rtnQueryType type = RTN_QUERY_NORMAL ;

      // Analyze the message, and create the operator.
      rc = _getQueryType( options.getQuery(), type ) ;
      PD_RC_CHECK( rc, PDERROR, "Get query type failed[ %d ]", rc ) ;

      if ( RTN_QUERY_TEXT == type )
      {
         opr = SDB_OSS_NEW rtnTSQueryOperator() ;
         if ( !opr )
         {
            PD_LOG( PDERROR, "Create text search query operator failed" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

      *pOperator = opr ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _rtnOperatorFactory::release( rtnOperator **pOperator )
   {
      if ( pOperator && *pOperator )
      {
         SDB_OSS_DEL (*pOperator) ;
         *pOperator = NULL ;
      }
   }

   INT32 _rtnOperatorFactory::_getQueryType( const BSONObj &query,
                                             rtnQueryType &qType )
   {
      INT32 rc = SDB_OK ;
      rtnSimpleCondParseTree condTree ;

      qType = RTN_QUERY_NORMAL ;
      rc = condTree.parse( query ) ;
      PD_RC_CHECK( rc, PDERROR, "Parse query failed[ %d ]", rc ) ;

      if ( condTree.hasTextCond())
      {
         qType = RTN_QUERY_TEXT ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   rtnOperatorFactory* rtnGetOperatorFactory()
   {
      static rtnOperatorFactory s_factory ;
      return &s_factory ;
   }
}

