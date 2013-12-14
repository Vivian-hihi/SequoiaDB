/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLAddToSet.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/05/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#include "rtnSQLAddToSet.hpp"
#include "ossMem.hpp"
#include "ossTypes.h"
#include "ossErr.h"

using namespace bson;

namespace engine
{
   rtnSQLAddToSet::rtnSQLAddToSet()
   {
      _pArrBuilder = SDB_OSS_NEW BSONArrayBuilder();
   }

   rtnSQLAddToSet::~rtnSQLAddToSet()
   {
      if ( _pArrBuilder )
      {
         SDB_OSS_DEL _pArrBuilder;
      }
   }

   INT32 rtnSQLAddToSet::result( BSONObjBuilder &builder )
   {
      INT32 rc = SDB_OK;
      PD_CHECK( !_alias.empty(), SDB_INVALIDARG, error, PDERROR,
               "no aliases for function!" );
      try
      {
         builder.appendArray( _alias.toString(), _pArrBuilder->arr() );
         SDB_OSS_DEL _pArrBuilder;
         _fieldSet.clear();
         _objVec.clear();
         _pArrBuilder = SDB_OSS_NEW BSONArrayBuilder();
         PD_CHECK( _pArrBuilder != NULL, SDB_OOM, error, PDERROR,
                  "malloc failed" );
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "received unexpected error:%s", e.what() );
         rc = SDB_SYS;
         goto error;
      }
   done:
      return rc;
   error:
      goto done;
   }

   INT32 rtnSQLAddToSet::_push( const RTN_FUNC_PARAMS &param )
   {
      INT32 rc = SDB_OK;
      SDB_ASSERT( _pArrBuilder != NULL, "_pArrBuilder can't be NULL!" );
      try
      {
         const BSONElement &ele = *(param.begin());
         if ( !ele.eoo() && !ele.isNull()
               && _fieldSet.find( ele ) == _fieldSet.end() )
         {
            _pArrBuilder->append( ele );
            BSONObjBuilder builder;
            builder.append( ele );
            BSONObj obj = builder.obj();
            _fieldSet.insert( obj.firstElement() );
            _objVec.push_back( obj );
         }
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "received unexpected error:%s", e.what() );
         rc = SDB_SYS;
         goto error;
      }
   done:
      return rc;
   error:
      goto done;
   }
}

