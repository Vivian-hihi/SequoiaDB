/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptConvertorHelper.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of Script component. This file contains structures for javascript
   engine wrapper

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/13/2013  YW Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPTCONVERTORHELPER_HPP_
#define SPTCONVERTORHELPER_HPP_

#include "core.hpp"
#include "jsapi.h"
#include <string>

INT32 JSObj2BsonRaw( JSContext *cx, JSObject *obj, CHAR **raw ) ;

INT32 JSVal2String( JSContext *cx, const jsval &val, std::string &str ) ;

BOOLEAN JSObjIsQuery( JSContext *cx, JSObject *obj ) ;

BOOLEAN JSObjIsCursor( JSContext *cx, JSObject *obj ) ;

BOOLEAN JSObjIsCS( JSContext *cx, JSObject *obj ) ;

BOOLEAN JSObjIsCL( JSContext *cx, JSObject *obj ) ;

BOOLEAN JSObjIsRN( JSContext *cx, JSObject *obj ) ;

BOOLEAN JSObjIsRG( JSContext *cx, JSObject *obj ) ;

BOOLEAN JSObjIsSdbObj( JSContext *cx, JSObject *obj ) ;

INT32 cursorNextRaw( void *cursor, CHAR **raw ) ;

INT32 JSObj2Cursor( JSContext *cx, JSObject *obj, void **cursor ) ;

BOOLEAN JSObjIsBsonobj( JSContext *cx, JSObject *obj ) ;

INT32 getBsonRawFromBsonClass( JSContext *cx, JSObject *obj, CHAR **raw ) ;

INT32 getCSNameFromObj( JSContext *cx, JSObject *obj,
                        CHAR **csName ) ;

INT32 getCLNameFromObj( JSContext *cx, JSObject *obj,
                        CHAR **clName ) ;

INT32 getRNNameFromObj( JSContext *cx, JSObject *obj,
                        CHAR **rnName ) ;

INT32 getRGNameFromObj( JSContext *cx, JSObject *obj,
                        CHAR **rgName ) ;

#endif

