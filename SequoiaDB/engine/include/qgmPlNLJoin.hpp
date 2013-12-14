/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmPlNLJoin.hpp

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

#ifndef QGMPLNLJOIN_HPP_
#define QGMPLNLJOIN_HPP_

#include "qgmPlJoin.hpp"

namespace engine
{
   class _qgmPlNLJoin : public _qgmPlJoin
   {
   public:
      _qgmPlNLJoin( INT32 type ) ;
      virtual ~_qgmPlNLJoin() ;

   private:
      virtual INT32 _execute( _pmdEDUCB *eduCB ) ;

      virtual INT32 _fetchNext ( qgmFetchOut &next ) ;

     INT32 _modifyInnerCondition( BSONObj &obj ) ;

   private:
      INT32 _init() ;

   private:
      BOOLEAN _makeOuterInner ;
      BOOLEAN _innerEnd ;
      BOOLEAN _notMatched ;
      qgmFetchOut _outerF ;
      qgmFetchOut *_innerF ;
   } ;

   typedef class _qgmPlNLJoin qgmPlNLJoin ;
}

#endif

