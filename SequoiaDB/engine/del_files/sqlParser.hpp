/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlParser.hpp

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

#ifndef SQLPARSER_HPP_
#define SQLPARSER_HPP_

#include "sqlDef.hpp"

namespace engine
{
   class _sqlParser
   {
   public:
      INT32 parse( const CHAR *sql, SQL_REQ &req ) ;

   private:
      INT32 _parseSelect( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseInsert( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseUpdate(const CHAR* sql, SQL_REQ &req) ;

      INT32 _parseDelete( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseCrtTable( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseDropTable( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseCrtSpace( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseDropSpace( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseCreateIndex( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseDropIndex( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseLTable( const CHAR *sql, SQL_REQ &req ) ;

      INT32 _parseLSpace( const CHAR *sql, SQL_REQ &req ) ;
   } ;

   typedef class _sqlParser sqlParser ;
}

#endif

