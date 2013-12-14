/*******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptConvertor.hpp

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

#ifndef SPTCONVERTOR_HPP_
#define SPTCONVERTOR_HPP_

#include "core.hpp"
#include "jsapi.h"
#include "../client/bson/bson.h"
#include <string>

class sptConvertor
{
public:
   sptConvertor( JSContext *cx )
   :_cx( cx )
   {

   }

   ~sptConvertor()
   {
      _cx = NULL ;
   }

public:
   INT32 toBson( JSObject *obj , bson **bs ) ;

   static INT32 toString( JSContext *cx,
                          const jsval &val,
                          std::string &str ) ;

private:
   INT32 _traverse( JSObject *obj , bson *bs ) ;

   INT32 _appendToBson( const std::string &name,
                        const jsval &val,
                        bson *bs ) ;

   BOOLEAN _addSpecialObj( JSObject *obj,
                           const CHAR *key,
                           bson *bs ) ;

   BOOLEAN _getProperty( JSObject *obj,
                         const CHAR *fieldName,
                         JSType type,
                         jsval &val ) ;

   INT32 _toString( const jsval &val, std::string &str ) ;

   INT32 _toInt( const jsval &val, INT32 &iN ) ;

   INT32 _toDouble( const jsval &val, FLOAT64 &fV ) ;

   INT32 _toBoolean( const jsval &val, BOOLEAN &bL ) ;

private:
   JSContext *_cx ;
} ;

#endif

