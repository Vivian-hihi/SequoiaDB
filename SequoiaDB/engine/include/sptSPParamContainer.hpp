/******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptSPParamContainer.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_SPPARAMCONTAINER_HPP_
#define SPT_SPPARAMCONTAINER_HPP_

#include "sptParamContainer.hpp"
#include "jsapi.h"

namespace engine
{
   class _sptSPParamContainer : public _sptParamContainer
   {
   public:
      _sptSPParamContainer( JSContext *context, uintN argc, jsval *vp ) ;
      virtual ~_sptSPParamContainer() ;

   public:
      virtual INT32 getINT32( UINT32 skip, BOOLEAN optional, INT32 &value )
      {
         return _getNative<INT32>( skip, optional, bson::NumberInt, value ) ;
      }

      virtual INT32 getFLOAT64( UINT32 skip, BOOLEAN optional, FLOAT64 &value )
      {
         return _getNative<FLOAT64>( skip, optional, bson::NumberDouble, value ) ;
      }

      virtual INT32 getString( UINT32 skip, BOOLEAN optional, std::string &value ) ;
      virtual INT32 getBSONObj( UINT32 skip, BOOLEAN optional, bson::BSONObj &value ) ;
      virtual INT32 getBOOLEAN(UINT32 skip, BOOLEAN optional, BOOLEAN &value )
      {
         return _getNative<BOOLEAN>( skip, optional, bson::Bool, value ) ;
      }

   private:
      template <typename T>
      INT32 _getNative( UINT32 skip, BOOLEAN optional,
                        bson::BSONType type, T &value ) ;

      void _getRule( UINT32 skip, BOOLEAN optional, bson::BSONType type, CHAR rule[] ) ;

   private:
      JSContext *_context ;
      uintN _argc ;
      jsval *_vp ;
   } ;
}

#endif

