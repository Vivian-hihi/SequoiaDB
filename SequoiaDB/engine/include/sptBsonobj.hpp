/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptBsonobj.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_BSONOBJ_HPP_
#define SPT_BSONOBJ_HPP_

#include "sptApi.hpp"
#include "../bson/bson.hpp"

namespace engine
{
   class _sptBsonobj : public SDBObject
   {
   JS_DECLARE_CLASS( _sptBsonobj )

   public:
      _sptBsonobj() ;
      _sptBsonobj( const bson::BSONObj &obj ) ;
      virtual ~_sptBsonobj() ;

   public:
      INT32 construct( const _sptArguments &arg,
                       _sptReturnVal &rval,
                       bson::BSONObj &detail) ;

      INT32 toJson( const _sptArguments &arg,
                    _sptReturnVal &rval,
                    bson::BSONObj &detail ) ;

      INT32 destruct() ;

   private:
      bson::BSONObj _obj ;
   } ;
   typedef class _sptUsrBsonobj sptUsrBsonobj ;
}

#endif

