/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ixm.hpp

   Descriptive Name = Index Management Header

   When/how to use: this program may be used on binary and text-formatted
   versions of index management component. This file contains structure for
   Index Record ID (IRID) and Index Control Block.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          03/07/2014  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef IXM_COMMON_HPP_
#define IXM_COMMON_HPP_

#include "core.hpp"
#include "../bson/bson.h"

using namespace std ;
using namespace bson ;

#define IXM_NAME_FIELD              IXM_FIELD_NAME_NAME
#define IXM_KEY_FIELD               IXM_FIELD_NAME_KEY
#define IXM_V_FIELD                 IXM_FIELD_NAME_V
#define IXM_UNIQUE_FIELD            IXM_FIELD_NAME_UNIQUE
#define IXM_ENFORCED_FIELD          IXM_FIELD_NAME_ENFORCED
#define IXM_DROPDUP_FIELD           IXM_FIELD_NAME_DROPDUPS
#define IXM_2DRANGE_FIELD           IXM_FIELD_NAME_2DRANGE

namespace engine
{
   /*
      _ixmHashValue define
   */
   union _ixmHashValue
   {
      struct
      {
         UINT32   hash1 ;
         UINT32   hash2 ;
      } columns ;
      UINT64 hash ;
   } ;
   typedef _ixmHashValue ixmHashValue ;

   /*
      Make object's array field hash value
   */
   void ixmMakeHashValue( const BSONObj &obj, const BSONObj &key,
                          ixmHashValue &hashValue ) ;

   void ixmMakeHashValue( const BSONElement &eleArray,
                          ixmHashValue &hashValue ) ;

}

#endif /* IXM_COMMON_HPP_ */

