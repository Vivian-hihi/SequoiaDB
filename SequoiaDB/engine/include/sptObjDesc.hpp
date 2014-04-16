/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptObjDesc.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_OBJDESC_HPP_
#define SPT_OBJDESC_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "sptFuncMap.hpp"
#include "jsapi.h"

namespace engine
{
   class _sptObjDesc : public SDBObject
   {
   public:
      _sptObjDesc()
      :_init(FALSE)
      {}

      virtual ~_sptObjDesc(){}
   public:
      const CHAR *getJSClassName() const
      {
         return _jsClassName.c_str() ;
      }

      const _sptFuncMap &getFuncMap()const
      {
         return _funcMap ;
      }

      const JSClass *getClassDef() const
      {
         return _init ? &_classDef : NULL ;
      }

      void setClassName( const CHAR *name )
      {
         _jsClassName.assign( name ) ;
      }

      void setClassDef( const JSClass &def )
      {
         _classDef = def ;
         _init = TRUE ;
      }
   protected:
      std::string _jsClassName ;
      _sptFuncMap _funcMap ;
      JSClass _classDef ;
      BOOLEAN _init ;
   } ;
   typedef class _sptObjDesc sptObjDesc ;
}

#endif

