/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlBsonBuilder.hpp

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

#ifndef SQLBSONBUILDER_HPP_
#define SQLBSONBUILDER_HPP_

#include "sqlDef.hpp"
#include "sqlGrammarBase.hpp"
#include "../bson/bson.h"

namespace engine
{
   typedef SQL_CONTAINER::const_iterator SQL_CON_ITR ;

   class _sqlBsonBuilder
   {
   public:
      INT32 buildCondition( const SQL_CONTAINER &c,
                            BSONObj &obj ) ;

      INT32 buildSelector( const SQL_CONTAINER &c,
                           BSONObj &obj ) ;

      /// specially, it is string.
      INT32 buildFullName( const SQL_CONTAINER &c,
                           string &fullName ) ;

      INT32 buildOrder( const SQL_CONTAINER &c,
                        BSONObj &obj ) ;

      INT32 buildInsertObj( const SQL_CONTAINER &fields,
                            const SQL_CONTAINER &values,
                            BSONObj &obj ) ;

      INT32 buildSet( const SQL_CONTAINER &c,
                      BSONObj &obj ) ;

      INT32 buildCreateIxm( const SQL_CONTAINER &c1,
                            const SQL_CONTAINER &c2,
                            BSONObj &obj,
                            string &fullName ) ;

      INT32 buildDropIxm( const SQL_CONTAINER &c,
                          BSONObj &obj,
                          string &fullName ) ;

   private:
      INT32 _buildCondition( const SQL_CON_ITR &c,
                            string &s ) ;

      INT32 _buildSet( const SQL_CON_ITR &c,
                       stringstream &ss ) ;

      INT32 _buildSelector( const SQL_CON_ITR &c,
                            BSONObjBuilder &builder ) ;
   } ;

   typedef class _sqlBsonBuilder sqlBsonBuilder ;
}

#endif

