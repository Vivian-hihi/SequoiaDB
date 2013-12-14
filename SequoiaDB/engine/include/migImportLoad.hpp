/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

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

#include "utilParseData.hpp"
#include "core.hpp"
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
   virtual INT32 run ( INT32 &total, INT32 &succeed ) ;
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
                migImportAccess accessModel ) ;
} ;
typedef class _migJSONParser migJSONParser ;

#endif
