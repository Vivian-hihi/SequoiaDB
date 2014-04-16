/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = omManager.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_MANAGER_HPP__
#define OM_MANAGER_HPP__

#include "omDef.hpp"
#include "clsObjBase.hpp"
#include "ossLatch.hpp"

#include <vector>

namespace engine
{

   /*
      _omManager define
   */
   class _omManager : public _clsObjBase
   {
      DECLARE_OBJ_MSG_MAP()

      public:
         _omManager() ;
         virtual ~_omManager() ;

         virtual INT32    initialize() ;
         virtual INT32    active () ;
         virtual INT32    deactive() ;
         virtual INT32    final() ;

         CHAR*       allocFixBuf() ;
         INT32       getFixBufSize() const { return _fixBufSize ; }
         void        releaseFixBuf( CHAR *pBuff ) ;

      protected:

      private:
         std::vector< CHAR* >                   _vecFixBuf ;
         const INT32                            _fixBufSize ;

         ossSpinSLatch                          _omLatch ;

   } ;
   typedef _omManager omManager ;

   /*
      get the global om manager object point
   */
   omManager *sdbGetOMManager() ;

}

#endif // OM_MANAGER_HPP__

