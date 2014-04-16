/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptScope.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptScope.hpp"
#include "pd.hpp"
#include "sptObjDesc.hpp"
#include "ossUtil.hpp"

namespace engine
{
   _sptScope::OBJ_DESCS _sptScope::_descs ;

   _sptScope::_sptScope()
   {

   }

   _sptScope::~_sptScope()
   {

   }

   INT32 _sptScope::loadUsrDefObj( _sptObjDesc *desc )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != desc, "desc can not be NULL" )
      SDB_ASSERT( NULL != desc->getJSClassName() &&
                  0 < ossStrlen(desc->getJSClassName()),
                  "obj name can not be empty" )

      if ( 0 < _descs.count( desc->getJSClassName() ))
      {
         PD_LOG( PDERROR, "%s has already been registered" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = _loadUsrDefObj( desc ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to load object defined by user:%d", rc ) ;
         goto error ;
      }

      _descs.insert( std::make_pair( desc->getJSClassName(), desc ) ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
}
