/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPtrTable.hpp

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

#ifndef QGMPTRTABLE_HPP_
#define QGMPTRTABLE_HPP_

#include "qgmDef.hpp"
#include <set>
#include "ossMem.hpp"

namespace engine
{
   typedef std::set<qgmField> PTR_TABLE ;
   typedef std::vector<CHAR*> STR_TABLE ;

   class _qgmPtrTable : public SDBObject
   {
   public:
      _qgmPtrTable() ;
      virtual ~_qgmPtrTable() ;

   public:
      INT32 getField( const SQL_CON_ITR &itr,
                      qgmField &field )
      {
         const CHAR *begin = NULL ;
         UINT32 size = 0 ;
         QGM_VALUE_PTR( itr, begin, size )
         return getField( begin, size, field ) ;
      }

      INT32 getField( const CHAR *begin, UINT32 size,
                      qgmField &field ) ;

      INT32 getOwnField( const CHAR *begin, qgmField &field ) ;

      INT32 getAttr( const SQL_CON_ITR &itr,
                     qgmDbAttr &attr ) ;

      INT32 getAttr( const CHAR *begin, UINT32 size,
                     qgmDbAttr &attr ) ;

      INT32 getUniqueFieldAlias( qgmField &field ) ;
      INT32 getUniqueTableAlias( qgmField &field ) ;

   private:
      PTR_TABLE _table ;
      STR_TABLE _stringTable ;
      UINT32    _uniqueFieldID ;
      UINT32    _uniqueTableID ;

   } ;

   typedef class _qgmPtrTable qgmPtrTable ;
}

#endif

