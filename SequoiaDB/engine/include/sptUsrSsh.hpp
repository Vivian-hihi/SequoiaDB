/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptUsrSsh.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_USRSSH_HPP_
#define SPT_USRSSH_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "sptApi.hpp"

namespace engine
{
   class _sptSshSession ;

   class _sptUsrSsh : public SDBObject
   {
   JS_DECLARE_CLASS( _sptUsrSsh )

   public:
      _sptUsrSsh() ;
      virtual ~_sptUsrSsh() ;

   public:
      INT32 construct( const _sptArguments &arg,
                       _sptReturnVal &rval,
                       bson::BSONObj &detail) ;

      INT32 destruct() ;

      INT32 exec( const _sptArguments &arg,
                  _sptReturnVal &rval,
                  bson::BSONObj &detail ) ;

      INT32 copy2Remote( const _sptArguments &arg,
                         _sptReturnVal &rval,
                         bson::BSONObj &detail ) ;

      INT32 copyFromRemote( const _sptArguments &arg,
                            _sptReturnVal &rval,
                            bson::BSONObj &detail ) ;

      INT32 getLastError( const _sptArguments &arg,
                         _sptReturnVal &rval,
                          bson::BSONObj &detail ) ;
   private:
      _sptSshSession *_session ;
   } ;
   typedef class _sptUsrSsh sptUsrSsh ;
}

#endif

