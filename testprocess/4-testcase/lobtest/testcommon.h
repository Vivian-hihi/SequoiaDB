/******************************************************************************
*@Name : common.h
*@Description: Common functions declaration for testcase programs
*******************************************************************************/

#ifndef COMMON_H__
#define COMMON_H__
#include <stdbool.h>
#include "client.h"
#include "lib/md5.h"
//#include <gtest/gtest.h>

#define HOST                  "localhost"
#define SERVER                "60000"
#define USER                  ""
#define PASSWD                ""

#define SDB_MD5_DIGEST_LENGTH 16      // the length for md5
// [COMMCSNAME:"C_DriverTest_GTest_CS"] is global CS in C Driver Test
#define COMMCSNAME            "C_DriverTest_GTest_CS"

/*
*@ define error number for LOB test
*/
#define LOB_ERROR_CONNECT_SDB  -1      /*<Failed to Connect Sdb>*/
#define LOB_ERROR_CREATE_CS   -2      /*<Failed to Create CS>*/
#define LOB_ERROR_CREATE_CL   -3      /*<Failed to Create CL>*/
#define LOB_ERROR_GET_CS      -4      /*<Failed to Get CS>*/
#define LOB_ERROR_DROP_CL     -5      /*<Failed to Get CL>*/
#define LOB_ERROR_GET_PID     -6      /*<Failed to Get CL>*/



#define CHECK_MSG(fmt, args ...) printf("%s[%d]:"fmt,__FILE__,__LINE__,##args)

struct Lob
{
   sdbCollectionHandle CL ;
   bson_oid_t OID ;
   CHAR *Buffer ;
   bool inspect ;
   SINT64 size ;
} ;

struct basicOp
{
   sdbCollectionHandle Cl ;
   bson_oid_t Oid[2] ;
   CHAR *writeLobBuf ;
   CHAR *readLobBuf ;
   UINT32 sekSize ;
} ;

SDB_EXTERN_C_START
/* display syntax error */
void displaySyntax ( CHAR *pCommand );

/* create record list */
void createRecordList ( bson *objlist, INT32 listSize ) ;
/* get name have pid, add by xiaojun */
void getUniqueName( const CHAR *modName, CHAR getName[] ) ;

/* write some string into file*/
//void createLobFile( UINT32 kilo, const CHAR *fileName, CHAR *buffer ) ;

/*md5 file*/
int md5Code( const char *src, char *code, int size ) ;

/*<create collection>*/
//INT32 tCreateCollection( sdbConnectionHandle *db, sdbCSHandle *cs,
//                         const CHAR *prefixName, sdbCollectionHandle *cl ) ;
INT32 tCreateCollection( sdbConnectionHandle *db, sdbCSHandle *cs,
                         const CHAR *prefixName, sdbCollectionHandle *cl,
                         CHAR *clName ) ;

/*<Thread function>*/
void *tOpenLob( void *LOB ) ;
void *tCloseLob( void *LOB ) ;
void *tRemoveLob( void *LOB ) ;
void *tWriteLob( void *LOB ) ;
void *tReadLob( void *LOB ) ;
void *tGetLobSize( void *LOB ) ;
void *tSeekReadLob( void *LOB ) ;
void testLobBasicOperation( void *basicOpLob ) ;
void genLobData( CHAR *lobWriteBuf, int size ) ;
static INT32 putData( UINT32 putSize, CHAR *buffer ) ;


SDB_EXTERN_C_END

#endif
