/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = migImport.hpp

   Descriptive Name = Migration Import Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for import operation

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          05/06/2013  HTL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MIGIMPORTLOAD_HPP__
#define MIGIMPORTLOAD_HPP__

#include "core.hpp"
#include "utilParseData.hpp"
#include "../client/client.h"
#include "../client/jstobs.h"
#include "../client/bson/bson.h"

#define MIG_DEFAULT_DELCHAR '\"'
#define MIG_DEFAULT_DELFIELD ','
#define MIG_DEFAULT_DELRECORD '\n'
enum migImportTypes
{
   MIG_IMPORT_TYPE_CSV = 0,
   MIG_IMPORT_TYPE_JSON,
   MIG_IMPORT_TYPE_END
} ;

enum migImportAccess
{
   MIG_IMPORT_GET_IO = 0,
   MIG_IMPORT_GET_HDFS,
   MIG_IMPORT_GET_END
} ;

class _migParser : public SDBObject
{
protected:
   _utilDataParser *_parser ;
   sdbCollectionHandle _collection ;
   INT32 _importRecord ( bson **bsonObj, SINT32 num ) ;
   virtual INT32 _getRecord ( bson &record ) = 0 ;
public:
   _migParser () ;
   virtual ~_migParser () ;
   virtual INT32 run ( INT32 &total, INT32 &succeed, INT32 insertNum ) ;
} ;

class _migCSVParser : public _migParser
{
protected :
   BOOLEAN _autoAddField ;
   BOOLEAN _autoCompletion ;
   INT32 _getRecord ( bson &record ) ;
public:
   INT32 init ( sdbCollectionHandle collection,
                const CHAR *pInputFile,
                const CHAR *pPath,
                const CHAR *pDelChar,
                const CHAR *pDelField,
                const CHAR *pDelRecord,
                const CHAR *sourceHost,
                const CHAR *sourceUser,
                UINT16 port,
                CHAR *fields,
                BOOLEAN isHeaderline,
                BOOLEAN autoAddField,
                BOOLEAN autoCompletion,
                BOOLEAN linePriority,
                migImportAccess accessModel ) ;
} ;
typedef class _migCSVParser migCSVParser ;

class _migJSONParser : public _migParser
{
protected :
   INT32 _getRecord ( bson &record ) ;
public:
   INT32 init ( sdbCollectionHandle collection,
                const CHAR *pInputFile,
                const CHAR *pPath,
                const CHAR *sourceHost,
                const CHAR *sourceUser,
                UINT16 port,
                BOOLEAN bMongoCompatible,
                BOOLEAN linePriority,
                migImportAccess accessModel,
                const CHAR *pDelRecord ) ;
} ;
typedef class _migJSONParser migJSONParser ;

#endif
