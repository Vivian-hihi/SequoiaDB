/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmSelector.hpp

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

******************************************************************************/

#ifndef QGMSELECTOR_HPP_
#define QGMSELECTOR_HPP_

#include "qgmDef.hpp"

namespace engine
{
   class _qgmSelector : public SDBObject
   {
   public:
      _qgmSelector() ;
      virtual ~_qgmSelector() ;

   public:
      OSS_INLINE BOOLEAN empty()const{ return _selector.empty() ;}

      OSS_INLINE BOOLEAN hasAlias()const{return _hasAlias ;}

      INT32 load( const qgmOPFieldVec &op ) ;

      INT32 select( const BSONObj &src, BSONObj &out ) const;

      INT32 select( const qgmFetchOut &src, BSONObj &out ) const ;

      BSONObj selector() const;

      string toString() const ;

   private:
      qgmOPFieldVec _selector ;
      BOOLEAN _hasAlias ;
   } ;

   typedef class _qgmSelector qgmSelector ;
}

#endif

