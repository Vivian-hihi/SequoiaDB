/*******************************************************************************

   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = utilDecodeRawbson.hpp

   Descriptive Name = BSON decode

   When/how to use: this program may be used on binary and text-formatted
   versions of UTIL component. This file contains declare of json2rawbson. Note
   this function should NEVER be directly called other than fromjson.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/03/2014  JWH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTIL_DECODE_BSON_HPP__
#define UTIL_DECODE_BSON_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"
#include "pd.hpp"
#include <vector>

struct fieldResolve : public SDBObject
{
   CHAR *pField ;
   fieldResolve *pSubField ;
   fieldResolve() : pField(NULL),
                    pSubField(NULL)
   {
   }
} ;

class utilDecodeBson : public SDBObject
{
private:
   CHAR _delChar ;
   CHAR _delField ;
public:
   std::vector<fieldResolve *> _vFields ;
private:
   CHAR *_trimLeft( CHAR *pCursor, INT32 &size ) ;
   CHAR *_trimRight( CHAR *pCursor, INT32 &size ) ;
   CHAR *_trim( CHAR *pCursor, INT32 &size ) ;
   void  _freeFieldList( fieldResolve *pFieldRe ) ;
   INT32 _filterString( CHAR **pField, INT32 &size ) ;
   INT32 _parseSubField( CHAR *pField, fieldResolve *pParent ) ;
   INT32 _appendBsonElement( void *pObj, fieldResolve *pFieldRe,
                             const CHAR *pData ) ;
public:
   utilDecodeBson() ;
   ~utilDecodeBson() ;
   INT32 init( CHAR delChar, CHAR delField ) ;
   INT32 parseFields( CHAR *pFields, INT32 size ) ;
   INT32 parseCSVSize( CHAR *pbson, INT32 *pCSVSize ) ;
   INT32 parseJSONSize( CHAR *pbson, INT32 *pJSONSize ) ;
   INT32 bsonCovertCSV( CHAR *pbson, CHAR **ppBuffer, INT32 *pCSVSize ) ;
   INT32 bsonCovertJson( CHAR *pbson, CHAR **ppBuffer, INT32 *pJSONSize ) ;
} ;

#endif