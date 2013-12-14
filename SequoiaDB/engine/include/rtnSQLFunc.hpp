/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnSQLFunc.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains declare for QGM operators

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/09/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTNSQLFUNC_HPP_
#define RTNSQLFUNC_HPP_

#include "core.hpp"
#include "qgmDef.hpp"
#include "pd.hpp"
#include "../bson/bson.h"
#include <vector>

using namespace bson ;

namespace engine
{
   typedef std::vector<BSONElement> RTN_FUNC_PARAMS ;

   class _rtnSQLFunc : public SDBObject
   {
   public:
      _rtnSQLFunc()
      {
      }
      virtual ~_rtnSQLFunc()
      {
      }

   public:
      INT32 push( const RTN_FUNC_PARAMS &param  )
      {
         INT32 rc = SDB_OK ;

         if ( _alias.empty() )
         {
            PD_LOG( PDERROR, "not initialized yet." ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         if ( _param.size() != param.size() )
         {
            PD_LOG( PDERROR, "number of param should be %d rather than %d",
                    _param.size(), param.size() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         rc = _push( param ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      done:
         return rc ;
      error:
        goto done ;
      }

      INT32 init( const _qgmField &alias,
                  const vector<qgmOpField> &param )
      {
         _alias = alias ;
         _param = param ;
         return SDB_OK ;
      }

      const vector<qgmOpField> &param() const
      {
         return _param ;
      }

      const CHAR *name() const
      {
         return _name.c_str() ;
      }

      virtual BOOLEAN isAggr()
      {
         return TRUE;
      }

      virtual INT32 result( BSONObjBuilder &builder ) = 0 ;

      virtual void clear(){ return ; }

      string toString() const ;

   private:
      virtual INT32 _push( const RTN_FUNC_PARAMS &param ) = 0 ;


   protected:
      std::string _name ;
      _qgmField _alias ;
      vector<qgmOpField> _param ;
   } ;

   typedef class _rtnSQLFunc rtnSQLFunc ;
}

#endif

