/******************************************************************************
*@Name : common.h
*@Description: Common functions declaration for testcase programs
*******************************************************************************/

#ifndef COMMON_H__
#define COMMON_H__
#include <stdbool.h>
#include "client.h"
#include "lib/md5.h"

/* run localhost */
//#define HOST                  "192.168.20.43"
//#define SERVER                "11810"


#define HOST                  "localhost"
#define SERVER                "11810"
#define USER                  ""
#define PASSWD                ""

#define SDB_MD5_DIGEST_LENGTH 16      // the length for md5
// [COMMCSNAME:"C_DriverTest_GTest_CS"] is global CS in C Driver Test
#define COMMCSNAME            "C_DriverTest_GTest_CS"
#define COMMCSNAME1           "C_DriverTest_GTest_CS_1"

//check rc code
#define CHECK_RC_CODE( rc, msg )\
do\
{\
   if( SDB_OK != rc )\
   {\
      printf( "%s[%d]: %s, rc = %d\n", __FILE__, __LINE__, msg, rc ) ;\
      goto done ; \
   }\
}\
while( 0 ) ;

SDB_EXTERN_C_START


struct basicOp
{
   sdbCollectionHandle Cl ;
   bson_oid_t Oid[2] ;
   CHAR *writeLobBuf ;
   CHAR *readLobBuf ;
   UINT32 sekSize ;
} ;


/* get name have pid, add by xiaojun */
void getUniqueName( const CHAR *modName, CHAR getName[] ) ;

/*md5 file*/
int md5Code( const char *src, char *code, int size ) ;

/*<create collection>*/
INT32 lobCreateCollection( sdbConnectionHandle *db, sdbCSHandle *cs,
                           const CHAR *prefixName, sdbCollectionHandle *cl,
                           CHAR *clName, CHAR *csName ) ;
INT32 lobCreateCollection1( sdbConnectionHandle *db, sdbCSHandle *cs,
                           const CHAR *prefixName, sdbCollectionHandle *cl,
                           CHAR *clName, CHAR *csName ) ;
INT32 lobCreateCollectionPz( sdbConnectionHandle *db, sdbCSHandle *cs,
                           const CHAR *prefixName, sdbCollectionHandle *cl,
                           CHAR *clName, bson *lobPzObj ) ;

/*<Thread function>*/
void thrdWriteLob( const CHAR *prefName, CHAR *lobBuffer, const UINT64 lobSize,
                   bson_oid_t *oids, sdbCSHandle *_cs, CHAR _clName[] ) ;
void thrdReadLob( const CHAR *prefName, const UINT64 lobSize, bson_oid_t oid ) ;
void thrdSeekReadLob( const CHAR *prefName, const UINT64 lobSize,
                      bson_oid_t oid, SINT64 seekSz, SDB_LOB_SEEK whence ) ;
void thrdRemoveLob( const CHAR *prefName, const bson_oid_t oid  ) ;

/*Auto generate data*/
void genLobData( CHAR *lobWriteBuf, UINT64 size ) ;
static INT32 putData( UINT32 putSize, CHAR *buffer ) ;

/*Write/Read/Remove lob when split*/
void splitLobWrite( sdbCollectionHandle cl, CHAR *lobBuffer,
                    const UINT64 lobSize, UINT64 putNum, bson_oid_t oids[] ) ;
void splitLobRead( sdbCollectionHandle cl, CHAR *lobBuffer, const UINT64 lobSize,
                   UINT64 putNum, bson_oid_t *oids, CHAR md5[] ) ;
void splitLobRemove( sdbCollectionHandle cl, CHAR *lobBuffer,
                     const UINT64 lobSize, UINT64 putNum, bson_oid_t *oids ) ;

/*Split About*/

INT32 lobGetCLrgName( sdbConnectionHandle db, const CHAR *clName,
                      CHAR srcGroup[] ) ;
INT32 lobGetRgName( sdbConnectionHandle db, const CHAR *clName,
                    CHAR dstGroup[] ) ;

SDB_EXTERN_C_END
#endif
