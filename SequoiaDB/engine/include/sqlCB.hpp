/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sqlCB.hpp

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

#ifndef SQLCB_HPP_
#define SQLCB_HPP_

#include "sqlGrammar.hpp"
#include "qgmBuilder.hpp"

namespace engine
{
   class _pmdEDUCB ;
   class _qgmPlanContainer ;
   class _rtnSQLFunc ;

   class _sqlCB : public SDBObject
   {
   public:
      _sqlCB() ;
      virtual ~_sqlCB() ;
   public:
      INT32 exec( const CHAR *sql, _pmdEDUCB *cb,
                  SINT64 &contextID ) ;

      INT32 getFunc( const CHAR *name,
                     UINT32 paramNum,
                     _rtnSQLFunc *&func ) ;

   private:
      INT32 _createContext( _qgmPlanContainer *container,
                            _pmdEDUCB *cb, SINT64 &contextID ) ;

   private:
      SQL_GRAMMAR _grammar ;
   } ;

   typedef class _sqlCB SQL_CB ;
}

#endif

