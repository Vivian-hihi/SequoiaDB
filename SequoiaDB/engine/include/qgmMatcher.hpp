/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = qgmMatcher.hpp

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

#ifndef QGMMATCHER_HPP_
#define QGMMATCHER_HPP_

#include "qgmDef.hpp"

namespace engine
{
   struct _qgmConditionNode ;

   class _qgmMatcher : public SDBObject
   {
   public:
      _qgmMatcher( _qgmConditionNode *node ) ;
      virtual ~_qgmMatcher() ;

   public:
      OSS_INLINE BOOLEAN ready(){return _ready ;}

      string toString() const ;

      INT32 match( const qgmFetchOut &fetch,
                   BOOLEAN &r ) ;

   private:
      INT32 _match( const _qgmConditionNode *node,
                    const qgmFetchOut &fetch,
                    BOOLEAN &r ) ;

   private:
      _qgmConditionNode *_condition ;
      BOOLEAN _ready ;
   } ;

   typedef class _qgmMatcher qgmMatcher ;
}

#endif

