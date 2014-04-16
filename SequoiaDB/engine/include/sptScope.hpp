/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptScope.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_SCOPE_HPP_
#define SPT_SCOPE_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.hpp"

namespace engine
{
   class _sptObjDesc ;

   class _sptScope : public SDBObject
   {
   public:
      _sptScope() ;
      virtual ~_sptScope() ;

   public:
      virtual INT32 start() = 0 ;

      virtual void shutdown() = 0 ;

      virtual INT32 eval( const CHAR *code, UINT32 len, bson::BSONObj &detail ) = 0 ;

   public:
      INT32 loadUsrDefObj( _sptObjDesc *desc ) ;

   private:
      virtual INT32 _loadUsrDefObj( _sptObjDesc *desc ) = 0 ;

   private:
      typedef std::map<std::string, const _sptObjDesc *> OBJ_DESCS ;

   protected:
      BOOLEAN _addDesc( const CHAR *name, const _sptObjDesc *desc )
      {
         return _descs.insert( std::make_pair( name, desc ) ).second ;
      }

      const _sptObjDesc *getDesc( const CHAR *name )
      {
         OBJ_DESCS::const_iterator itr = _descs.find( name ) ;
         return _descs.end() == itr ?
                NULL : itr->second ;
      }

   private:
      static OBJ_DESCS _descs ;
   } ;
   typedef class _sptScope sptScope ;
}

#endif

