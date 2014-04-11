/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = migExport.hpp

   Descriptive Name = Migration Export Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for export operation

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef MIGEXPORT_HPP__
#define MIGEXPORT_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "../client/client.h"
#include "ossIO.hpp"
#include <string>
#include <vector>
using namespace std ;
#define MIG_MAX_EXTRACT_BUFFER 256*1024*1024
#define MIG_INC_EXTRACT_BUFFER 4194304

#define MIG_DEFAULT_DELCHAR '"'
#define MIG_DEFAULT_DELFIELD ','
#define MIG_DEFAULT_DELRECORD '\n'

#define INC_BUF_SIZE 4096
#define INIT_BUF_SIZE 1024

enum migExportTypes
{
   MIG_EXPORT_TYPE_CSV = 0,
   MIG_EXPORT_TYPE_BIN,
   MIG_EXPORT_TYPE_JSON
} ;

class _migExtractor : public SDBObject
{
protected :
   migExportTypes _exportType ;
   OSSFILE _file ;
   CHAR _delChar[2] ;
   CHAR _delField[2] ;
   CHAR _delRecord[2] ;
   sdbCollectionHandle _collection ;
   sdbCursorHandle _cursor ;
   BOOLEAN _isOpened ;
   BOOLEAN _init ;
   CHAR *_pExtractBuffer ;
   CHAR *_pCurPtr ;
   UINT32 _bufSize ;
   virtual INT32 _extractRecord () = 0 ;
   BOOLEAN _hexStrToDel ( const CHAR *pHexStr,
                          CHAR &del ) ;
   BOOLEAN _strToDel ( const CHAR *pDelStr,
                       CHAR &del ) ;
   INT32 _reallocMem ( UINT32 requiredSize ) ;
   INT32 _appendStr ( const CHAR *pStr ) ;
   INT32 _flushBuf () ;
   inline ossValuePtr _bufOccupied ()
   {
      return _pCurPtr - _pExtractBuffer ;
   }
   inline ossValuePtr _bufFree ()
   {
      return _bufSize - _bufOccupied() ;
   }
public :
   _migExtractor () ;
   virtual ~_migExtractor () ;
   INT32 run () ;
} ;

class _migCSVExtractor : public _migExtractor
{
private :
   vector<string> _fieldList ;
   BOOLEAN _incHead ;
protected :
   INT32 _extractRecord () ;
   INT32 _extractString ( CHAR *str, INT32 strLen, BOOLEAN trim, CHAR delChar ) ;
public :
   _migCSVExtractor ( vector<string> &fieldList, BOOLEAN incHead ) ;
   ~_migCSVExtractor () ;
   INT32 init ( sdbCollectionHandle collection,
                const CHAR *pOutputFile,
                const CHAR *pDelChar,
                const CHAR *pDelField,
                const CHAR *pDelRecord ) ;
} ;
typedef class _migCSVExtractor migCSVExtractor ;

class _migJSONExtractor : public _migExtractor
{
private:
   BOOLEAN _hasID ;
   vector<string> _fieldList ;
private :
   BOOLEAN _reallocateBuffer ( CHAR **pBuf, INT32 *bufSize ) ;
protected :
   INT32 _extractRecord () ;
public :
   _migJSONExtractor ( vector<string> &fieldList ) ;
   ~_migJSONExtractor () ;
   INT32 init ( sdbCollectionHandle collection,
                const CHAR *pOutputFile,
                const CHAR *pDelRecord,
                BOOLEAN hasID ) ;
} ;
typedef class _migJSONExtractor migJSONExtractor ;

#endif
