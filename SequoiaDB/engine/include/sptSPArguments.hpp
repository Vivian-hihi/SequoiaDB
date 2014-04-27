/******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptSPArguments.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_SPARGUMENTS_HPP_
#define SPT_SPARGUMENTS_HPP_

#include "sptArguments.hpp"
#include "jsapi.h"

namespace engine
{
   class _sptSPArguments : public _sptArguments
   {
   public:
      _sptSPArguments( JSContext *context, uintN argc, jsval *vp ) ;
      virtual ~_sptSPArguments() ;

   public:
      virtual INT32 getNative( UINT32 pos, void *value ) const ;
      virtual INT32 getString( UINT32 pos, std::string &value ) const ;
      virtual INT32 getBsonobj( UINT32 pos, bson::BSONObj &value ) const ;
      virtual UINT32 argc() const
      {
         return _argc ;
      }
   
   private:
      jsval *_getValAtPos( UINT32 pos ) const ;

   private:
      JSContext *_context ;
      uintN _argc ;
      jsval *_vp ;
   } ;
}

#endif

