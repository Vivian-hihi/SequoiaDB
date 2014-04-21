#ifndef SDB_FDW_H__
#define SDB_FDW_H__

#include "client.h"
#include "fmgr.h"
#include "catalog/pg_foreign_server.h"
#include "catalog/pg_foreign_table.h"

#define OPTION_NAME_ADDRESS         "address"
#define OPTION_NAME_SERVICE         "service"
#define OPTION_NAME_USER            "user"
#define OPTION_NAME_PASSWORD        "password"
#define OPTION_NAME_COLLECTIONSPACE "collectionspace"
#define OPTION_NAME_COLLECTION      "collection"

#define DEFAULT_HOSTNAME            "localhost"
#define DEFAULT_SERVICENAME         "30010"
#define DEFAULT_USERNAME            ""
#define DEFAULT_PASSWORDNAME        ""

#define INITIAL_ARRAY_CAPACITY         8
#define SDB_TUPLE_COST_MULTIPLIER      5

#define POSTGRES_TO_UNIX_EPOCH_DAYS (POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE)
#define POSTGRES_TO_UNIX_EPOCH_USECS (POSTGRES_TO_UNIX_EPOCH_DAYS * USECS_PER_DAY)
struct SdbInputOption
{
   const CHAR *optionName ;
   Oid   optionContextID ;
} ;
typedef struct SdbInputOption SdbInputOption ;

static const SdbInputOption SdbInputOptionList[] =
{
   { OPTION_NAME_ADDRESS,         ForeignServerRelationId },
   { OPTION_NAME_SERVICE,         ForeignServerRelationId },
   { OPTION_NAME_USER,            ForeignServerRelationId },
   { OPTION_NAME_PASSWORD,        ForeignServerRelationId },
   { OPTION_NAME_COLLECTIONSPACE, ForeignTableRelationId },
   { OPTION_NAME_COLLECTION,      ForeignTableRelationId }
} ;

struct SdbInputOptions
{
   CHAR *address ;
   CHAR *service ;
   CHAR *user ;
   CHAR *password ;
   CHAR *collectionspace ;
   CHAR *collection ;
} ;
typedef struct SdbInputOptions SdbInputOptions ;

/* SdbColumnMapping is for mapping between bson's field name and PG's column
 * name
 */
struct SdbColumnMapping
{
   CHAR columnName [ NAMEDATALEN ] ;
   UINT32 columnIndex ;
   Oid columnTypeId ;
   INT32 columnTypeMod ;
   Oid columnArrayTypeId ;
} ;
typedef struct SdbColumnMapping SdbColumnMapping ;

/* SdbExeState represents the runtime state for sdb cursor
 */
struct SdbExecState
{
   struct HTAB *columnMappingHash ; /* map bson fields to columns */
   sdbCursorHandle hCursor ;
   sdbConnectionHandle hConnection ;
   sdbCollectionHandle hCollection ;
   bson *queryDocument ; /* query request */
} ;
typedef struct SdbExecState SdbExecState ;


extern Datum sdb_fdw_handler(PG_FUNCTION_ARGS);
extern Datum sdb_fdw_validator(PG_FUNCTION_ARGS);
#endif
