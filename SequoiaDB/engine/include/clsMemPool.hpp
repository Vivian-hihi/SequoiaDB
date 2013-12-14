/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = clsMemPool.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          27/11/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef CLS_MEM_POOL_HPP_
#define CLS_MEM_POOL_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "clsBase.hpp"
#include "ossAtomic.hpp"

namespace engine
{

   /*
      _clsMemPool define
   */
   class _clsMemPool : public SDBObject
   {
      public:
         _clsMemPool() ;
         ~_clsMemPool() ;

         INT32 initialize () ;
         INT32 final () ;
         void  clear() ;

         UINT64 totalSize() { return _totalMemSize.peek() ; }

      public:
         CHAR* alloc( UINT32 size, UINT32 &assignSize ) ;
         void  release ( CHAR* pBuff, UINT32 size ) ;
         CHAR* realloc ( CHAR* pBuff, UINT32 srcSize,
                         UINT32 needSize, UINT32 &assignSize ) ;

      private:
         ossAtomic64    _totalMemSize ;
   } ;
   typedef _clsMemPool clsMemPool ;

}

#endif //CLS_MEM_POOL_HPP_

