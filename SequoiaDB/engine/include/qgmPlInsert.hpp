/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlInsert.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/09/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef QGMPLINSERT_HPP_
#define QGMPLINSERT_HPP_

#include "qgmPlan.hpp"
#include "msgDef.h"

namespace engine
{
   class _qgmPlInsert : public _qgmPlan
   {
   public:
      _qgmPlInsert( const qgmDbAttr &collection ) ;

      virtual ~_qgmPlInsert() ;

   public:
      void addCV( const qgmOPFieldVec &columns,
                  const qgmOPFieldVec &values ) ;

      virtual string toString() const ;

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext ( qgmFetchOut &next ) ;

      inline BOOLEAN _directInsert()
      {
         return !_columns.empty() ;
      }

      INT32 _nextRecord( _pmdEDUCB *eduCB, BSONObj &obj ) ;

      INT32 _mergeObj( BSONObj &obj ) const ;

   private:
      string _fullName ;
      qgmOPFieldVec _columns ;
      qgmOPFieldVec _values ;
      BOOLEAN _got ;
      SDB_ROLE _role ;
   } ;

   typedef class _qgmPlInsert qgmPlInsert ;
}

#endif

