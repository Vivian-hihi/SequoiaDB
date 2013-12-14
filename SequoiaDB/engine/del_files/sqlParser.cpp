/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlParser.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sqlParser.hpp"
#include "sqlWhere.hpp"
#include "sqlFrom.hpp"
#include "sqlSelect.hpp"
#include "sqlOrder.hpp"
#include "sqlInsert.hpp"
#include "sqlInsertFields.hpp"
#include "sqlValue.hpp"
#include "sqlUpdate.hpp"
#include "sqlSet.hpp"
#include "sqlDelete.hpp"
#include "sqlCrtTable.hpp"
#include "sqlDropTable.hpp"
#include "sqlCrtSpace.hpp"
#include "sqlDropSpace.hpp"
#include "sqlCreateIndex.hpp"
#include "sqlDropIndex.hpp"
#include "sqlIndexFields.hpp"
#include "sqlListTable.hpp"
#include "sqlListSpace.hpp"
#include "sqlBsonBuilder.hpp"
#include "pd.hpp"
#include "sqlUtil.hpp"
#include "pdTrace.hpp"
#include "sqlTrace.hpp"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS_PARSE, "_sqlParser::parse" )
   INT32 _sqlParser::parse( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS_PARSE );
      SDB_ASSERT( NULL != sql, "impossible" )

      try
      {
         /// operation with high frequency is preferred.
         /// select > insert > update > delete > ...
         rc = _parseSelect( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_SELECT ;
            goto done ;
         }

         rc = _parseInsert( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_INSERT ;
            goto done ;
         }

         rc = _parseUpdate( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_UPDATE ;
            goto done ;
         }

         rc = _parseDelete( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_DELETE ;
            goto done ;
         }

         rc = _parseCreateIndex( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_CREATE_INDEX ;
            goto done ;
         }

         rc = _parseDropIndex( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_DROP_INDEX ;
            goto done ;
         }

         rc = _parseLTable( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_LIST_TABLE ;
            goto done ;
         }

         rc = _parseLSpace( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_LIST_SPACE ;
            goto done ;
         }

         rc = _parseCrtTable( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_CRT_CL ;
            goto done ;
         }

         rc = _parseDropTable( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_DROP_CL ;
            goto done ;
         }

         rc = _parseCrtSpace( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_CRT_CS ;
            goto done ;
         }

         rc = _parseDropSpace( sql, req ) ;
         if ( SDB_OK == rc )
         {
            req.type = SQL_TYPE_DROP_CS ;
            goto done ;
         }


      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected exception:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      PD_LOG( PDDEBUG,
              "sql [%s] type[%d] fullname[%s] condition[%s] "
              "selector[%s] order[%s]",
              sql, req.type, req.fullName.c_str(),
              req.condition.toString().c_str(),
              req.selector.toString().c_str(),
              req.order.toString().c_str() ) ;
      PD_TRACE_EXITRC ( SDB__SQLPAS_PARSE, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASCRTINX, "_sqlParser::_parseCreateIndex" )
   INT32 _sqlParser::_parseCreateIndex( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASCRTINX );
      sqlBsonBuilder builder ;
      SQL_CRTIXM_GRAMMAR g1 ;
      SQL_AST ast ;
      SQL_AST ast2 ;
      ast = SQL_PARSE( sql, g1 ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         SQL_IXMFIELDS_GRAMMAR g2 ;
         ast2 = SQL_PARSE( ast.stop, g2 ) ;
         sqlDumpAst( ast2.trees ) ;
         if ( !ast2.match )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         else if ( !ast2.full )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         else
         {
            rc = builder.buildCreateIxm( ast.trees, ast2.trees,
                                         req.selector, req.fullName ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASCRTINX, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASLTAB, "_sqlParser::_parseLTable" )
   INT32 _sqlParser::_parseLTable( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASLTAB );
      SQL_LTABLE_GRAMMAR list ;
      SQL_AST ast ;
      ast = SQL_PARSE( sql, list ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         /// do nothing.
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASLTAB, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASLSPC, "_sqlParser::_parseLSpace" )
   INT32 _sqlParser::_parseLSpace( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASLSPC );
      SQL_LSPACE_GRAMMAR list ;
      SQL_AST ast ;
      ast = SQL_PARSE( sql, list ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         /// do nothing.
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASLSPC, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASDROPINX, "_sqlParser::_parseDropIndex" )
   INT32 _sqlParser::_parseDropIndex( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASDROPINX );
      SQL_DROPIXM_GRAMMAR drop ;
      sqlBsonBuilder builder ;
      SQL_AST ast ;
      ast = SQL_PARSE( sql, drop ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildDropIxm( ast.trees, req.selector,
                                    req.fullName ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASDROPINX, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASCRTSPC, "_sqlParser::_parseCrtSpace" )
   INT32 _sqlParser::_parseCrtSpace( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASCRTSPC );
      sqlBsonBuilder builder ;
      SQL_CRTSPACE_GRAMMAR crt ;
      SQL_AST ast ;
      ast = SQL_PARSE( sql, crt ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildFullName( ast.trees, req.fullName ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASCRTSPC, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASDROPSPC, "_sqlParser::_parseDropSpace" )
   INT32 _sqlParser::_parseDropSpace( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASDROPSPC );
      sqlBsonBuilder builder ;
      SQL_DROPSPACE_GRAMMAR drop ;
      SQL_AST ast ;
      ast = SQL_PARSE( sql, drop ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildFullName( ast.trees, req.fullName ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASDROPSPC, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASCRTTAB, "_sqlParser::_parseCrtTable" )
   INT32 _sqlParser::_parseCrtTable( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASCRTTAB );
      sqlBsonBuilder builder ;
      SQL_CRTTABLE_GRAMMAR crt ;
      SQL_AST ast ;
      ast = SQL_PARSE( sql, crt ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildFullName( ast.trees, req.fullName ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASCRTTAB, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__pASDROPTAB, "_sqlParser::_parseDropTable" )
   INT32 _sqlParser::_parseDropTable( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__pASDROPTAB );
      sqlBsonBuilder builder ;
      SQL_DROPTABLE_GRAMMAR drop ;
      SQL_AST ast ;
      ast = SQL_PARSE( sql, drop ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildFullName( ast.trees, req.fullName ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__pASDROPTAB, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASDEL, "_sqlParser::_parseDelete" )
   INT32 _sqlParser::_parseDelete( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASDEL );
      sqlBsonBuilder builder ;
      SQL_AST ast ;
      {
      SQL_DELETE_GRAMMAR del ;
      ast = SQL_PARSE( sql, del ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildFullName( ast.trees, req.fullName ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      if ( ast.full )
      {
         goto done ;
      }
      }

      {
      SQL_WHERE_GRAMMAR where ;
      ast = SQL_PARSE( ast.stop, where ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildCondition( ast.trees, req.condition ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASDEL, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASUPD, "_sqlParser::_parseUpdate" )
   INT32 _sqlParser::_parseUpdate( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASUPD );
      sqlBsonBuilder builder ;
      SQL_AST ast ;

      /// parse fullname
      {
      SQL_UPDATE_GRAMMAR update ;
      ast = SQL_PARSE( sql, update ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildFullName( ast.trees, req.fullName ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      }

      /// parse set
      {
      SQL_SET_GRAMMAR set ;
      ast = SQL_PARSE( ast.stop, set ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      /// reuse req.selector.
      rc = builder.buildSet( ast.trees, req.selector ) ;
      if ( SDB_OK != rc )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( ast.full )
      {
         goto done ;
      }
      }

      /// parse where
      {
      SQL_WHERE_GRAMMAR where ;
      ast = SQL_PARSE( ast.stop, where ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildCondition( ast.trees, req.condition ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASUPD, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASINS, "_sqlParser::_parseInsert" )
   INT32 _sqlParser::_parseInsert( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASINS );
      sqlBsonBuilder builder ;
      SQL_AST ast ;

      /// parse fullname
      {
      SQL_INSERT_GRAMMAR insert ;
      ast = SQL_PARSE( sql, insert ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildFullName( ast.trees, req.fullName ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      }

      /// parse field
      {
      SQL_IFIELDS_GRAMMAR fields ;
      ast = SQL_PARSE( ast.stop, fields ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         /// do nothing.
      }
      }

      {
      SQL_AST valueAst ;
      SQL_VALUE_GRAMMAR value ;
      valueAst = SQL_PARSE( ast.stop, value ) ;
      if ( !valueAst.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( !valueAst.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         /// re use req.selector for insert obj.
         rc = builder.buildInsertObj( ast.trees,
                                      valueAst.trees,
                                      req.selector ) ;
      }
      }
   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASINS, rc );
      return rc ;
   error:
      goto done ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__SQLPAS__PASSEL, "_sqlParser::_parseSelect" )
   INT32 _sqlParser::_parseSelect( const CHAR *sql, SQL_REQ &req )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__SQLPAS__PASSEL );
      sqlBsonBuilder builder ;
      SQL_AST ast ;
      /// parse selector
      {
      SQL_SELECT_GRAMMAR selector ;
      ast = SQL_PARSE( sql, selector ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( ast.full )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         /// do nothing.
      }

      rc = builder.buildSelector( ast.trees, req.selector ) ;
      if ( SDB_OK != rc )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      }

      /// parse fullname
      {
      SQL_FROM_GRAMMAR from ;
      ast = SQL_PARSE( ast.stop, from ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildFullName( ast.trees, req.fullName ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }

      if ( ast.full )
      {
         goto done ;
      }
      }

      /// parse where.
      {
      SQL_WHERE_GRAMMAR where ;
      ast = SQL_PARSE( ast.stop, where ) ;
      if ( !ast.match )
      {
         /// possibly it is order by
      }
      else
      {
         rc = builder.buildCondition( ast.trees, req.condition ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         if ( ast.full )
         {
            goto done ;
         }
      }
      }

      /// parse order by
      {
      SQL_ORDER_GRAMMAR order ;
      ast = SQL_PARSE( ast.stop, order ) ;
      if ( !ast.match )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = builder.buildOrder( ast.trees, req.order ) ;
         if ( SDB_OK != rc )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
         /// order by should be the last part of the sql.
         if ( !ast.full )
         {
            rc = SDB_INVALIDARG ;
            goto error ;
         }
      }
      }

   done:
      PD_TRACE_EXITRC ( SDB__SQLPAS__PASSEL, rc );
      return rc ;
   error:
      goto done ;
   }
}


