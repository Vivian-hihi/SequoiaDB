#ifndef BSONGEN_H__
#define BSONGEN_H__


#include "core.hpp"
#include "../../bson/bson.h"
using namespace bson ;
INT32 genRandomRecord(UINT32 maxFieldNum,
                   UINT32 maxFieldNameLength,
                   UINT32 maxStringLength,
                   UINT32 maxDepth,
                   BSONObj&,
                   BOOLEAN forceOID = FALSE ) ;

UINT32 genString(CHAR *str,UINT32 MaxStringLength);

#endif
