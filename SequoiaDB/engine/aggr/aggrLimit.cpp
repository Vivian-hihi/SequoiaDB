/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = aggrLimit.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/04/2013  JHL  Initial Draft

   Last Changed =

******************************************************************************/
#include "aggrLimit.hpp"
#include "qgmOptiSelect.hpp"
#include "aggrDef.hpp"

using namespace bson;

namespace engine
{

   INT32 aggrLimitParser::buildNode( const BSONElement &elem, const CHAR *pCLName,
                              qgmOptiTreeNode *&pNode, _qgmPtrTable *pTable,
                              _qgmParamTable *pParamTable )
   {
      INT32 rc = SDB_OK;
      qgmOptiSelect *pSelect = SDB_OSS_NEW qgmOptiSelect( pTable, pParamTable );
      PD_CHECK( pSelect!=NULL, SDB_OOM, error, PDERROR,
               "malloc failed!" );
      PD_CHECK( elem.isNumber(), SDB_INVALIDARG, error, PDERROR,
               "failed to parse the parameter(%s),type must be number!",
               elem.fieldName() );
      try
      {
         qgmOpField selectAll;
         selectAll.type = SQL_GRAMMAR::WILDCARD;
         pSelect->_selector.push_back( selectAll );
         pSelect->_limit = elem.number();
         pSelect->_skip = 0;
         pSelect->_type = QGM_OPTI_TYPE_SELECT;
         pSelect->_hasFunc = FALSE;
         rc = pTable->getOwnField( AGGR_CL_DEFAULT_ALIAS, pSelect->_alias );
         PD_RC_CHECK( rc, PDERROR, "failed to get the field(%s)", AGGR_CL_DEFAULT_ALIAS );
         if ( pCLName != NULL )
         {
            qgmField clValAttr;
            qgmField clValRelegation;
            rc = pTable->getOwnField( pCLName, clValAttr );
            PD_RC_CHECK( rc, PDERROR, "failed to get the field(%s)", pCLName );
            rc = pTable->getOwnField( AGGR_CL_DEFAULT_ALIAS, pSelect->_collection.alias );
            PD_RC_CHECK( rc, PDERROR, "failed to get the field(%s)", AGGR_CL_DEFAULT_ALIAS );
            pSelect->_collection.value = qgmDbAttr( clValRelegation, clValAttr );
            pSelect->_collection.type = SQL_GRAMMAR::DBATTR;
         }
      }
      catch ( std::exception &e )
      {
         PD_CHECK( SDB_INVALIDARG, SDB_INVALIDARG, error, PDERROR,
                  "failed to parse the \"limit\", received unexpected error:%s",
                  e.what() );
      }
      pNode = pSelect;
   done:
      return rc;
   error:
      SAFE_OSS_DELETE( pSelect );
      goto done;
   }

}
