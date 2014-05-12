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
#ifndef MIGIMPORT_HPP__
#define MIGIMPORT_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossIO.hpp"
#include "../client/client.h"
#include "../client/jstobs.h"
#include "../client/bson/bson.h"
#include "utilParseData.hpp"
#include "../util/csv2rawbson.hpp"
#include <string>
#include <vector>
using namespace std ;

#define MIG_MAX_READ_BUFFER 256*1024*1024
#define MIG_INC_READ_BUFFER 4194304

#define MIG_DEFAULT_DELCHAR '\"'
#define MIG_DEFAULT_DELFIELD ','
#define MIG_DEFAULT_DELRECORD '\n'
enum migImportTypes
{
   MIG_IMPORT_TYPE_CSV = 0,
   MIG_IMPORT_TYPE_BIN,
   MIG_IMPORT_TYPE_JSON
} ;

class _migParser : public SDBObject
{
protected:
   _utilDataParser *_parser ;
   CHAR _delChar[2] ;
   CHAR _delField[2] ;
   CHAR _delRecord[2] ;
   sdbCollectionHandle _collection ;
   BOOLEAN _stringType ;
   INT32 _importRecord ( bson **bsonObj, SINT32 num ) ;
   virtual INT32 _getRecord ( bson &record ) = 0 ;
public:
   _migParser () ;
   virtual ~_migParser () ;
   virtual INT32 run ( INT32 &total, INT32 &succeed ) ;
} ;

class _migCSVParser : public _migParser
{
private:
   csvParser _csvParser ;
protected :
   BOOLEAN _autoAddField ;
   BOOLEAN _autoCompletion ;
   INT32 _getRecord ( bson &record ) ;
public:
   INT32 init ( sdbCollectionHandle collection,
                const CHAR *pInputFile,
                CHAR *fields,
                BOOLEAN isHeaderline,
                BOOLEAN autoAddField,
                BOOLEAN autoCompletion,
                BOOLEAN linePriority,
                BOOLEAN stringType,
                const CHAR *pDelChar,
                const CHAR *pDelField,
                const CHAR *pDelRecord ) ;
} ;
typedef class _migCSVParser migCSVParser ;

class _migJSONParser : public _migParser
{
protected :
   INT32 _getRecord ( bson &record ) ;
public:
   INT32 init ( sdbCollectionHandle collection,
                const CHAR *pInputFile,
                BOOLEAN linePriority,
                BOOLEAN bMongoCompatible,
                const CHAR *pDelRecord ) ;
} ;
typedef class _migJSONParser migJSONParser ;

#endif
