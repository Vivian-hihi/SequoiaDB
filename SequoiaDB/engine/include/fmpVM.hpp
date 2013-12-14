/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = fmpVM.hpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/19/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef FMPVM_HPP_
#define FMPVM_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "fmpDef.hpp"
#include "../bson/bson.h"

using namespace bson ;

class _fmpVM : public SDBObject
{
public:
   _fmpVM() ;
   virtual ~_fmpVM() ;

public:
   virtual INT32 init( const BSONObj &param ) ;

//   virtual INT32 compile( const BSONElement &func, const CHAR * ) = 0 ;

   virtual INT32 eval( const BSONObj &func,
                       BSONObj &res ) = 0 ;

   virtual INT32 fetch( BSONObj &res ) = 0 ;

   virtual INT32 initGlobalDB( BSONObj &res ) = 0 ;

   inline BOOLEAN ok() const { return _ok ;}

protected:
   inline void _setContext( SINT64 contextID )
   {
      _contextID = contextID ;
      return ;
   }
   inline SINT64 _getContext(){return _contextID ;}

   inline const BSONObj &_getParam() const
   {
      return _param ;
   }

   inline void _setOK( BOOLEAN isOK )
   {
      _ok = isOK ;
      return ;
   }

private:
   BSONObj _param ;
   SINT64 _contextID ;
   BOOLEAN _ok ;
} ;

#endif

