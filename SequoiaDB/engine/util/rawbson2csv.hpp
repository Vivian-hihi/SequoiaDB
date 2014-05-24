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

   Source File Name = csv2rawbson.hpp

   Descriptive Name = CSV To Raw BSON

   When/how to use: this program may be used on binary and text-formatted
   versions of UTIL component. This file contains declare of json2rawbson. Note
   this function should NEVER be directly called other than fromjson.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/04/2014  JWH Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef UTIL_BSON_2_CSV_HPP__
#define UTIL_BSON_2_CSV_HPP__

#include "core.hpp"
#include "oss.hpp"
#include <vector>

/* csv type */
#define CSV_STR_INT        "int"
#define CSV_STR_INTEGER    "integer"
#define CSV_STR_LONG       "long"
#define CSV_STR_BOOL       "bool"
#define CSV_STR_BOOLEAN    "boolean"
#define CSV_STR_DOUBLE     "double"
#define CSV_STR_STRING     "string"
#define CSV_STR_TIMESTAMP  "timestamp"
#define CSV_STR_DATE       "date"
#define CSV_STR_NULL       "null"
#define CSV_STR_UNDEFINED  "undefined"
#define CSV_STR_MINKEY     "minKey"
#define CSV_STR_MAXKEY     "maxKey"

/* type value */
#define CSV_STR_TRUE       "true"
#define CSV_STR_FALSE      "false"

/* key word */
#define CSV_STR_DEFAULT    "default"
#define CSV_STR_FIELD      "field"

/* string size */
#define CSV_STR_INT_SIZE         ( sizeof( CSV_STR_INT ) - 1 )
#define CSV_STR_INTEGER_SIZE     ( sizeof( CSV_STR_INTEGER ) - 1 )
#define CSV_STR_LONG_SIZE        ( sizeof( CSV_STR_LONG ) - 1 )
#define CSV_STR_BOOL_SIZE        ( sizeof( CSV_STR_BOOL ) - 1 )
#define CSV_STR_BOOLEAN_SIZE     ( sizeof( CSV_STR_BOOLEAN ) - 1 )
#define CSV_STR_DOUBLE_SIZE      ( sizeof( CSV_STR_DOUBLE ) - 1 )
#define CSV_STR_STRING_SIZE      ( sizeof( CSV_STR_STRING ) - 1 )
#define CSV_STR_TIMESTAMP_SIZE   ( sizeof( CSV_STR_TIMESTAMP ) - 1 )
#define CSV_STR_DATE_SIZE        ( sizeof( CSV_STR_DATE ) - 1 )
#define CSV_STR_NULL_SIZE        ( sizeof( CSV_STR_NULL ) - 1 )
#define CSV_STR_UNDEFINED_SIZE   ( sizeof( CSV_STR_UNDEFINED ) - 1 )
#define CSV_STR_MINKEY_SIZE      ( sizeof( CSV_STR_MINKEY ) - 1 )
#define CSV_STR_MAXKEY_SIZE      ( sizeof( CSV_STR_MAXKEY ) - 1 )

#define CSV_STR_TRUE_SIZE        ( sizeof( CSV_STR_TRUE ) - 1 )
#define CSV_STR_FALSE_SIZE       ( sizeof( CSV_STR_FALSE ) - 1 )

#define CSV_STR_DEFAULT_SIZE     ( sizeof( CSV_STR_DEFAULT ) - 1 )
#define CSV_STR_FIELD_SIZE       ( sizeof( CSV_STR_FIELD ) - 1 )

#define CSV_STR_FIELD_MAX_SIZE 1024

class csvEncode : public SDBObject
{
private:
   struct csvEncodeObj : public SDBObject
   {
      INT32  csvSize ;
      INT32  csvBufferSize ;
      INT32  tempBufferSize ;
      CHAR  *pCSVBuffer ;
      CHAR  *pTempBuffer ;
      csvEncodeObj() : csvSize(0),
                       csvBufferSize(0),
                       tempBufferSize(0),
                       pCSVBuffer(NULL),
                       pTempBuffer(NULL)
      {
      }
   } ;
private:
   CHAR     _delChar ;
   CHAR     _delField ;
   CHAR     _delRecord ;
   std::vector<CHAR *> _vField ;
   std::vector<csvEncodeObj *> _vObjList ;
private:
   CHAR *_trimLeft ( CHAR *pCursor, INT32 &size ) ;
   CHAR *_trimRight ( CHAR *pCursor, INT32 &size ) ;
   CHAR *_trim ( CHAR *pCursor, INT32 &size ) ;
   INT32 _parseField( CHAR **pField, INT32 &size ) ;
   INT32 _getEncodeObj( INT32 ID, csvEncodeObj **ppCSVObj ) ;
   INT32 _estimateCSVSize( csvEncodeObj *pCSVObj, INT32 bsonSize ) ;
   INT32 _rallocCSV( CHAR **pBuffer, INT32 newSize ) ;
   INT32 _appendString( csvEncodeObj *pCSVObj,
                        const CHAR *pBuffer, INT32 size ) ;
   INT32 _appendObj( csvEncodeObj *pCSVObj, void *pBson_iterator ) ;
   INT32 _appendNonString( csvEncodeObj *pCSVObj, void *pBson_iterator ) ;
   INT32 _appendValue( csvEncodeObj *pCSVObj, void *pBson_iterator ) ;
public:
   csvEncode() ;
   ~csvEncode() ;
   INT32 parseHeader( CHAR *pFields, INT32 size ) ;
   INT32 init( CHAR delChar,
               CHAR delField,
               CHAR delRecord ) ;
   INT32 getID( INT32 &ID ) ;
   INT32 getCSV( INT32 ID, CHAR **ppBuffer, INT32 &size ) ;
   INT32 bson2csv( INT32 ID, CHAR *pbson ) ;
} ;

#endif