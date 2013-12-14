/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = fmpJSVM.hpp

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

#ifndef FMPJSVM_HPP_
#define FMPJSVM_HPP_

#include "fmpVM.hpp"
#include <string>

namespace engine
{
   class ScriptEngine ;
   class Scope ;
}

class _fmpJSVM : public _fmpVM
{
public:
   _fmpJSVM() ;
   virtual ~_fmpJSVM() ;

public:
   virtual INT32 eval( const BSONObj &func,
                       BSONObj &res ) ;

   virtual INT32 fetch( BSONObj &res ) ;

   virtual INT32 initGlobalDB( BSONObj &res ) ;

  // virtual INT32 compile( const BSONElement &func, const CHAR *name ) ;

private:
   INT32 _transCode2Str( const BSONElement &ele, std::string &str ) ;

private:
   engine::ScriptEngine *_engine ;
   engine::Scope *_scope ;
   std::string _cmd ;
   void *_cursor ;
} ;

#endif

