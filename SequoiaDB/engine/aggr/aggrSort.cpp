/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = aggrSort.cpp

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
#include "aggrSort.hpp"
#include "qgmOptiSelect.hpp"
#include "aggrDef.hpp"

using namespace bson;

namespace engine
{

   INT32 aggrSortParser::buildNode( const BSONElement &elem, const CHAR *pCLName,
                              qgmOptiTreeNode *&pNode, _qgmPtrTable *pTable,
                              _qgmParamTable *pParamTable )
   {
      INT32 rc = SDB_OK;
      qgmOptiSelect *pSelect = SDB_OSS_NEW qgmOptiSelect( pTable, pParamTable );
      PD_CHECK( pSelect!=NULL, SDB_OOM, error, PDERROR,
               "malloc failed!" );
      PD_CHECK( elem.type()==Object, SDB_INVALIDARG, error, PDERROR,
               "failed to parse the parameter(%s),type must be number!",
               elem.fieldName() );
      try
      {
         qgmOpField selectAll;
         rc = buildOrderBy( elem, pSelect->_orderby, pTable, pCLName );
         PD_RC_CHECK( rc, PDERROR,
                     "failed to build \"sort\" field(rc=%d)", rc );
         selectAll.type = SQL_GRAMMAR::WILDCARD;
         pSelect->_selector.push_back( selectAll );
         pSelect->_limit = -1;
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
                  "failed to parse the \"sort\", received unexpected error:%s",
                  e.what() );
      }
      pNode = pSelect;
   done:
      return rc;
   error:
      SAFE_OSS_DELETE( pSelect );
      goto done;
   }

   INT32 aggrSortParser::buildOrderBy( const BSONElement &elem,
                                       qgmOPFieldVec &orderBy,
                                       _qgmPtrTable *pTable,
                                       const CHAR *pCLName )
   {
      INT32 rc = SDB_OK;
      try
      {
         BSONObj obj = elem.embeddedObject();
         BSONObjIterator iter( obj );
         while ( iter.more() )
         {
            BSONElement beField = iter.next();
            PD_CHECK( beField.isNumber(), SDB_INVALIDARG, error, PDERROR,
                     "type of sort-field must be number!" );
            {
               qgmField obValAttr;
               qgmField obValRelegation;
               rc = pTable->getOwnField( AGGR_CL_DEFAULT_ALIAS,
                                       obValRelegation );
               PD_RC_CHECK( rc, PDERROR, "failed to get the field(%s)",
                           AGGR_CL_DEFAULT_ALIAS );
               rc = pTable->getOwnField( beField.fieldName(), obValAttr );
               PD_RC_CHECK( rc, PDERROR, "failed to get the field(%s)",
                           beField.fieldName() );
               {
                  qgmDbAttr obVal( obValRelegation, obValAttr );
                  qgmField obAlias;
                  qgmOpField obOpField;
                  obOpField.value = obVal;
                  obOpField.alias = obAlias;
                  if ( beField.number() > 0 )
                  {
                     obOpField.type = SQL_GRAMMAR::ASC;
                  }
                  else
                  {
                     obOpField.type = SQL_GRAMMAR::DESC;
                  }
                  //obOpField.type = beField.number() > 0 ? (SQL_GRAMMAR::ASC) : (SQL_GRAMMAR::DESC);
                  orderBy.push_back( obOpField );
               }
            }
         }
      }
      catch ( std::exception &e )
      {
         PD_CHECK( SDB_INVALIDARG, SDB_INVALIDARG, error, PDERROR,
                  "failed to parse the \"sort\", received unexpected error:%s",
                  e.what() );
      }
   done:
      return rc;
   error:
      goto done;
   }

}


