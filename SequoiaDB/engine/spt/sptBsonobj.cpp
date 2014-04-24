/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptBsonobj.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptBsonobj.hpp"

namespace engine
{
JS_CONSTRUCT_FUNC_DEFINE( _sptBsonobj, construct )
JS_DESTRUCT_FUNC_DEFINE( _sptBsonobj, destruct)
JS_MEMBER_FUNC_DEFINE( _sptBsonobj, toJson )

JS_BEGIN_MAPPING( _sptBsonobj, "BSONObj" )
  JS_ADD_MEMBER_FUNC( "toJson", toJson )
  JS_ADD_CONSTRUCT_FUNC( construct )
  JS_ADD_DESTRUCT_FUNC( destruct )
JS_MAPPING_END()

   _sptBsonobj::_sptBsonobj()
   {

   }

   _sptBsonobj::_sptBsonobj( const bson::BSONObj &obj )
   {
      _obj = obj.copy() ;
   }

   _sptBsonobj::~_sptBsonobj()
   {

   }

   INT32 _sptBsonobj::construct( const _sptArguments &arg,
                                 _sptReturnVal &rval,
                                 bson::BSONObj &detail)
   {
      detail = BSON( SPT_ERR << "new BSONObj is forbidden." ) ;
      return SDB_INVALIDARG ;
   }

   INT32 _sptBsonobj::toJson( const _sptArguments &arg,
                              _sptReturnVal &rval,
                               bson::BSONObj &detail )
   {
      rval.setNativeVal( "", bson::String, _obj.toString().c_str()) ;
      return SDB_OK ;      
   }

   INT32 _sptBsonobj::destruct()
   {
      return SDB_OK ;
   }
}

