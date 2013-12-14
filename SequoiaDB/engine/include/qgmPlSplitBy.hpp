/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlSplitBy.hpp

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

#ifndef QGMPLSPLITBY_HPP_
#define QGMPLSPLITBY_HPP_

#include "qgmPlan.hpp"
#include "msgDef.h"

namespace engine
{
   class _qgmPlSplitBy : public _qgmPlan
   {
   public:
      _qgmPlSplitBy( const _qgmDbAttr &split,
                     const _qgmField &alias ) ;
      virtual ~_qgmPlSplitBy() ;

   public:
      virtual string toString() const ;

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext ( qgmFetchOut &next ) ;

      void _clear() ;

   private:
      _qgmDbAttr _splitby ;
      qgmFetchOut _fetch ;
      BSONObjIterator _itr ;
      std::string _fieldName ;
   } ;
   typedef class _qgmPlSplitBy qgmPlSplitBy ;
}

#endif

