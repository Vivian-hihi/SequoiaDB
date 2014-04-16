/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptSPScope.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_SPSCOPE_HPP_
#define SPT_SPSCOPE_HPP_

#include "sptScope.hpp"
#include "jsapi.h"

namespace engine
{
   class _sptSPScope : public _sptScope
   {
   public:
      _sptSPScope() ;
      virtual ~_sptSPScope() ;

   public:
      virtual INT32 start() ;

      virtual void shutdown() ;

   public:
      virtual INT32 eval(const CHAR *code, UINT32 len, bson::BSONObj &detail ) ;

   private:
      virtual INT32 _loadUsrDefObj( _sptObjDesc *desc ) ;

   private:
      JSRuntime *_runtime ;
      JSContext *_context ;
      JSObject *_global ;
      JSErrorReporter _errReporter ;
   } ;
   typedef class _sptSPScope sptSPScope ;
}

#endif

