#ifndef CLIENT_INTERNAL_H__
#define CLIENT_INTERNAL_H__

#include "core.h"
#include "client.h"
#define SDB_HANDLE_TYPE_INVALID      0
#define SDB_HANDLE_TYPE_CONNECTION   1
#define SDB_HANDLE_TYPE_COLLECTION   2
#define SDB_HANDLE_TYPE_CS           3
#define SDB_HANDLE_TYPE_CURSOR       4
#define SDB_HANDLE_TYPE_REPLICAGROUP 5
#define SDB_HANDLE_TYPE_REPLICANODE  6

struct _Node
{
   ossValuePtr data ;
   struct _Node *next ;
} ;
typedef struct _Node Node ;

struct _sdbConnectionStruct
{
   // generic variables, to validate which type does this handle belongs to
   INT32 _handleType ;
   SOCKET _sock ;
   CHAR *_pSendBuffer ;
   INT32 _sendBufferSize ;
   CHAR *_pReceiveBuffer ;
   INT32 _receiveBufferSize ;
   BOOLEAN _endianConvert ;

   Node *_cursors ;
} ;
typedef struct _sdbConnectionStruct sdbConnectionStruct ;

#define CLIENT_RG_NAMESZ         127
#define CLIENT_MAX_HOSTNAME      255
#define CLIENT_MAX_SERVICENAME   63
struct _sdbRGStruct
{
   // generic variables, to validate which type does this handle belongs to
   INT32 _handleType ;
   sdbConnectionHandle _connection ;
   SOCKET _sock ;
   INT32 _offset ;
   CHAR *_pSendBuffer ;
   INT32 _sendBufferSize ;
   CHAR *_pReceiveBuffer ;
   INT32 _receiveBufferSize ;
   BOOLEAN _endianConvert ;

   // replication group specific variables
   CHAR  _replicaGroupName [ CLIENT_RG_NAMESZ+1 ] ;
   BOOLEAN _isCatalog ;
} ;
typedef struct _sdbRGStruct sdbRGStruct ;

#define SDB_REPLICA_NODE_INVALID_NODEID -1
struct _sdbRNStruct
{
   // generic variables, to validate which type does this handle belongs to
   INT32 _handleType ;
   sdbConnectionHandle _connection ;
   SOCKET _sock ;
   INT32 _offset ;
   CHAR *_pSendBuffer ;
   INT32 _sendBufferSize ;
   CHAR *_pReceiveBuffer ;
   INT32 _receiveBufferSize ;
   BOOLEAN _endianConvert ;

   // replication node specific variables
   CHAR _hostName [ CLIENT_MAX_HOSTNAME + 1 ] ;
   CHAR _serviceName [ CLIENT_MAX_SERVICENAME + 1 ] ;
   CHAR _nodeName [ CLIENT_MAX_HOSTNAME + CLIENT_MAX_SERVICENAME + 2 ] ;
   INT32 _nodeID ;
} ;
typedef struct _sdbRNStruct sdbRNStruct ;

#define CLIENT_COLLECTION_NAMESZ 127
#define CLIENT_CS_NAMESZ         127
struct _sdbCSStruct
{
   // generic variables, to validate which type does this handle belongs to
   INT32 _handleType ;
   sdbConnectionHandle _connection ;
   SOCKET _sock ;
   INT32 _offset ;
   CHAR *_pSendBuffer ;
   INT32 _sendBufferSize ;
   CHAR *_pReceiveBuffer ;
   INT32 _receiveBufferSize ;
   BOOLEAN _endianConvert ;

   // collection space specific variables
   CHAR _CSName [ CLIENT_CS_NAMESZ + 1 ] ;
} ;
typedef struct _sdbCSStruct sdbCSStruct ;

struct _sdbCollectionStruct
{
   // generic variables, to validate which type does this handle belongs to
   INT32 _handleType ;
   sdbConnectionHandle _connection ;
   SOCKET _sock ;
   INT32 _offset ;
   CHAR *_pSendBuffer ;
   INT32 _sendBufferSize ;
   CHAR *_pReceiveBuffer ;
   INT32 _receiveBufferSize ;
   BOOLEAN _endianConvert ;

   // collection specific variables
   CHAR _collectionName [ CLIENT_COLLECTION_NAMESZ + 1 ] ;
   CHAR _CSName [ CLIENT_CS_NAMESZ + 1 ] ;
   CHAR _collectionFullName [ CLIENT_CS_NAMESZ + CLIENT_COLLECTION_NAMESZ + 2 ];
} ;
typedef struct _sdbCollectionStruct sdbCollectionStruct ;

struct _sdbCursorStruct
{
   // generic variables, to validate which type does this handle belongs to
   INT32 _handleType ;
   sdbConnectionHandle _connection ;
   SOCKET _sock ;
   INT32 _offset ;
   CHAR *_pSendBuffer ;
   INT32 _sendBufferSize ;
   CHAR *_pReceiveBuffer ;
   INT32 _receiveBufferSize ;
   BOOLEAN _isClosed ;
   BOOLEAN _endianConvert ;

   // cursor specific variables
   SINT64 _contextID ;
   SINT64 _totalRead ;
//   bson *_modifiedCurrent ;
//   BOOLEAN _isDeleteCurrent ;
   CHAR _collectionFullName [ CLIENT_CS_NAMESZ + CLIENT_COLLECTION_NAMESZ + 2 ];
} ;
typedef struct _sdbCursorStruct sdbCursorStruct ;

#endif
