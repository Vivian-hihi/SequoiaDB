/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = spt.hpp

   Descriptive Name = Script Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Script component. This file contains structures for javascript
   engine wrapper

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/13/2013  MPQ Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SPT_HPP__
#define SPT_HPP__

#include "jsapi.h"
#include "core.hpp"
#include "oss.hpp"

namespace engine
{

class Scope : public SDBObject
{
public:
   Scope();
   ~Scope();

   BOOLEAN init();

   inline JSContext *context(){ return _context ;}

   /**
    * if len > 0, then assume code is a string of length len, which may
    * contain embeded \0;
    * if len = 0, then assume code is a C-style string ending with \0
    *
    * On success, *result points to the result of evalution, otherwise, it is
    * NOT modified.
    */
   INT32 evaluate ( const CHAR *code , UINT32 len , const CHAR *filename ,
                      UINT32 lineno , CHAR ** result ) ;

   INT32 evaluate2 ( const CHAR *code, UINT32 len, UINT32 lineno,
                    jsval *rval, CHAR **errMsg ) ;

private:
   JSContext *_context;
   JSObject *_global;
};

class ScriptEngine : public SDBObject
{
public:
   ScriptEngine();
   ~ScriptEngine();

   BOOLEAN init();

   Scope *newScope();

   static ScriptEngine *globalScriptEngine();
   static void purgeGlobalScriptEngine();

   friend class Scope;
private:
   JSRuntime *_runtime;
   JSErrorReporter _errorReporter ;
};

} // namespace engine

#endif
