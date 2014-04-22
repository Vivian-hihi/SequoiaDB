#include "postgres.h"
#include "sdb_fdw.h"

#include "access/htup_details.h"
#include "access/reloptions.h"
#include "access/xact.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h"
#include "commands/explain.h"
#include "commands/vacuum.h"
#include "foreign/fdwapi.h"
#include "foreign/foreign.h"
#include "nodes/makefuncs.h"
#include "nodes/relation.h"
#include "optimizer/cost.h"
#include "optimizer/pathnode.h"
#include "optimizer/plancat.h"
#include "optimizer/planmain.h"
#include "optimizer/restrictinfo.h"
#include "optimizer/var.h"
#include "parser/parsetree.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/date.h"
#include "utils/hsearch.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "utils/numeric.h"
#include "utils/rel.h"
#include "utils/timestamp.h"

#define SDB_COLLECTIONSPACE_NAME_LEN 128
#define SDB_COLLECTION_NAME_LEN 128

static void SdbGetForeignRelSize ( PlannerInfo *root,
                                   RelOptInfo *baserel,
                                   Oid foreignTableId ) ;

static void SdbGetForeignPaths ( PlannerInfo *root,
                                 RelOptInfo *baserel,
                                 Oid foreignTableId ) ;

static ForeignScan *SdbGetForeignPlan ( PlannerInfo *root,
                                        RelOptInfo *baserel,
                                        Oid foreignTableId,
                                        ForeignPath *bestPath,
                                        List *targetList,
                                        List *restrictionClauses ) ;

static void SdbBeginForeignScan ( ForeignScanState *scanState,
                                  INT32 executorFlags ) ;

static TupleTableSlot *SdbIterateForeignScan ( ForeignScanState *scanState ) ;

static void SdbRescanForeignScan ( ForeignScanState *scanState ) ;

static void SdbEndForeignScan ( ForeignScanState *scanState ) ;

static void SdbExplainForeignScan ( ForeignScanState *scanState,
                                    ExplainState *explainState ) ;

static bool SdbAnalyzeForeignTable ( Relation relation,
                                     AcquireSampleRowsFunc *acquireSampleRowsFunc,
                                     BlockNumber *totalPageCount ) ;

static INT32 SdbIsForeignRelUpdatable ( Relation rel ) ;

static void SdbAddForeignUpdateTargets ( Query *parsetree,
                                         RangeTblEntry *target_rte,
                                         Relation target_relation ) ;

static List *SdbPlanForeignModify ( PlannerInfo *root,
                                    ModifyTable *plan,
                                    Index resultRelation,
                                    INT32 subplan_index ) ;
/* module initialization */
void _PG_init () ;

/* transaction management */
static void SdbFdwXactCallback ( XactEvent event, void *arg ) ;

#if PG_VERSION_NUM>90300
BeginForeignModify_function SdbBeginForeignModify = NULL ;
ExecForeignInsert_function SdbExecForeignInsert = NULL ;
ExecForeignUpdate_function SdbExecForeignUpdate = NULL ;
ExecForeignDelete_function SdbExecForeignDelete = NULL ;
EndForeignModify_function SdbEndForeignModify = NULL ;
ExplainForeignModify_function SdbExplainForeignModify = NULL ;
#endif

/* register handler and validator */
PG_MODULE_MAGIC ;
PG_FUNCTION_INFO_V1 ( sdb_fdw_handler ) ;
PG_FUNCTION_INFO_V1 ( sdb_fdw_validator ) ;

/* declare for sdb_fdw_handler */
Datum sdb_fdw_handler ( PG_FUNCTION_ARGS )
{
   FdwRoutine *fdwRoutine              = makeNode ( FdwRoutine ) ;
   /* Functions for scanning foreign tables */
   fdwRoutine->GetForeignRelSize       = SdbGetForeignRelSize ;
   fdwRoutine->GetForeignPaths         = SdbGetForeignPaths ;
   fdwRoutine->GetForeignPlan          = SdbGetForeignPlan ;
   fdwRoutine->BeginForeignScan        = SdbBeginForeignScan ;
   fdwRoutine->IterateForeignScan      = SdbIterateForeignScan ;
   fdwRoutine->ReScanForeignScan       = SdbRescanForeignScan ;
   fdwRoutine->EndForeignScan          = SdbEndForeignScan ;
#if PG_VERSION_NUM>90300
   /* Remaining functions are optional */
   fdwRoutine->AddForeignUpdateTargets = SdbAddForeignUpdateTargets ;
   fdwRoutine->PlanForeignModify       = SdbPlanForeignModify ;
   fdwRoutine->BeginForeignModify      = SdbBeginForeignModify ;
   fdwRoutine->ExecForeignInsert       = SdbExecForeignInsert ;
   fdwRoutine->ExecForeignUpdate       = SdbExecForeignUpdate ;
   fdwRoutine->ExecForeignDelete       = SdbExecForeignDelete ;
   fdwRoutine->EndForeignModify        = SdbEndForeignModify ;
   fdwRoutine->IsForeignRelUpdatable   = SdbIsForeignRelUpdatable ;
#endif
   /* Support functions for EXPLAIN */
   fdwRoutine->ExplainForeignScan      = SdbExplainForeignScan ;
#if PG_VERSION_NUM>90300
   fdwRoutine->ExplainForeignModify    = SdbExplainForeignModify ;
#endif
   fdwRoutine->AnalyzeForeignTable     = SdbAnalyzeForeignTable ;
   PG_RETURN_POINTER ( fdwRoutine ) ;
}

/* connection pool */
static SdbConnectionPool *sdbGetConnectionPool ()
{
   static SdbConnectionPool connPool ;
   return &connPool ;
}

static void sdbInitConnectionPool ()
{
   SdbConnection *pNewMem  = NULL ;
   SdbConnectionPool *pool = sdbGetConnectionPool() ;
   memset ( pool, 0, sizeof ( SdbConnectionPool ) ) ;
   pNewMem = (SdbConnection*)malloc ( sizeof(SdbConnection) *
                                      INITIAL_ARRAY_CAPACITY ) ;
   if ( !pNewMem )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_OUT_OF_MEMORY ),
                         errmsg ( "Unable to allocate connection pool" ),
                         errhint ( "Make sure the memory pool or ulimit is "
                                   "properly configured" ) ) ) ;
      goto error ;
   }
   pool->connList = pNewMem ;
   pool->poolSize = INITIAL_ARRAY_CAPACITY ;
error :
   return ;
}

static void sdbUninitConnectionPool ()
{
   INT32 count             = 0 ;
   SdbConnectionPool *pool = sdbGetConnectionPool() ;
   for ( count = 0; count < pool->numConnections; ++count )
   {
      SdbConnection *conn = &pool->connList[count] ;
      if ( conn->connName )
      {
         free ( conn->connName ) ;
      }
      sdbDisconnect ( conn->hConnection ) ;
      sdbReleaseConnection ( conn->hConnection ) ;
   }
   free ( pool->connList ) ;
}

/* sdbSerializeDocument serializes the sdbbson document to a constant
 * Note this function just copies the pointer of documents' data,
 * therefore the caller should NOT destroy the object
 */
static Const *sdbSerializeDocument ( sdbbson *document )
{
   Const *serializedDocument = NULL ;
   Datum documentDatum       = 0 ;
   const CHAR *documentData  = sdbbson_data ( document ) ;
   INT32 documentSize        = sdbbson_buffer_size ( document ) ;
   documentDatum             = CStringGetDatum ( documentData ) ;
   serializedDocument        = makeConst ( CSTRINGOID, -1,
                                           InvalidOid, documentSize,
                                           documentDatum, FALSE, FALSE ) ;
   return serializedDocument ;
}

/* sdbDeserializeDocument deserializes a constant into sdbbson document
 */
static void sdbDeserializeDocument ( Const *constant,
                                     sdbbson *document )
{
   Datum documentDatum = constant->constvalue ;
   CHAR *documentData = DatumGetCString ( documentDatum ) ;
   sdbbson_init_size ( document, 0 ) ;
   sdbbson_init_finished_data ( document, documentData ) ;
   return ;
}

/* sdbOperatorName converts PG comparison operator to Sdb
 */
static const CHAR *sdbOperatorName ( const CHAR *operatorName )
{
   const CHAR *pResult                  = NULL ;
   const INT32 totalNames               = 6 ;
   static const CHAR *nameMappings[][2] = {
      { "<", "$lt" },
      { "<=", "$lte" },
      { ">", "$gt" },
      { ">=", "$gte" },
      { "=", "$et" },
      { "<>", "$ne" } } ;
   INT32 i = 0 ;
   for ( i = 0; i < totalNames; ++i )
   {
      if ( strncmp ( nameMappings[i][0],
                     operatorName, NAMEDATALEN ) == 0 )
      {
         pResult = nameMappings[i][1] ;
         break ;
      }
   }
   return pResult ;
}

/* sdbFindArgumentOfType iterate the given argument list, looks for an argument
 * with the given type and returns the argument if found
 */
static Expr *sdbFindArgumentOfType ( List *argumentList, NodeTag argumentType )
{
   Expr *foundArgument = NULL ;
   ListCell *argumentCell = NULL ;
   foreach ( argumentCell, argumentList )
   {
      Expr *argument = (Expr *) lfirst ( argumentCell ) ;
      if ( nodeTag ( argument ) == argumentType )
      {
         foundArgument = argument ;
         break ;
      }
   }
   return foundArgument ;
}

/* sdbUniqueColumnList group all expression with same column name together
 */
static List *sdbUniqueColumnList ( List *operatorList )
{
   List *uniqueColumnList = NIL ;
   ListCell *operatorCell = NULL ;
   foreach ( operatorCell, operatorList )
   {
      OpExpr *operator = (OpExpr *) lfirst ( operatorCell ) ;
      List *argumentList = operator->args ;
      Var *column = (Var *) sdbFindArgumentOfType(argumentList, T_Var) ;
      uniqueColumnList = list_append_unique ( uniqueColumnList, column ) ;
   }
   return uniqueColumnList ;
}

/* sdbAppendConstantValue appends to query document with key and value */
static void sdbAppendConstantValue ( sdbbson *queryDocument,
                                     const CHAR *keyName,
                                     Const *constant )
{
   Datum constantValue  = constant->constvalue ;
   Oid constantTypeId   = constant->consttype ;
   BOOLEAN constantNull = constant->constisnull ;
   if ( constantNull )
   {
      /* this matches null and not exists for both table and index scan */
      sdbbson_append_int ( queryDocument, "$isnull", 1 ) ;
      goto done ;
   }
   switch ( constantTypeId )
   {
   case INT2OID :
   {
      INT16 value = DatumGetInt16 ( constantValue ) ;
      sdbbson_append_int ( queryDocument, keyName, ( INT32 ) value ) ;
      break ;
   }
   case INT4OID :
   {
      INT32 value = DatumGetInt32 ( constantValue ) ;
      sdbbson_append_int ( queryDocument, keyName, value ) ;
      break ;
   }
   case INT8OID :
   {
      INT64 value = DatumGetInt64 ( constantValue ) ;
      sdbbson_append_long ( queryDocument, keyName, value ) ;
      break ;
   }
   case FLOAT4OID :
   {
      FLOAT32 value = DatumGetFloat4 ( constantValue ) ;
      sdbbson_append_double ( queryDocument, keyName, (FLOAT64)value ) ;
      break ;
   }
   case FLOAT8OID :
   {
      FLOAT64 value = DatumGetFloat8 ( constantValue ) ;
      sdbbson_append_double ( queryDocument, keyName, value ) ;
      break ;
   }
   case NUMERICOID :
   {
      Datum valueDatum = DirectFunctionCall1 ( numeric_float8, constantValue ) ;
      FLOAT64 value    = DatumGetFloat8 ( valueDatum ) ;
      sdbbson_append_double ( queryDocument, keyName, value ) ;
      break ;
   }
   case BOOLOID :
   {
      BOOLEAN value = DatumGetBool ( constantValue ) ;
      sdbbson_append_bool ( queryDocument, keyName, value ) ;
      break ;
   }
   case BPCHAROID :
   case VARCHAROID :
   case TEXTOID :
   {
      CHAR *outputString    = NULL ;
      Oid outputFunctionId  = InvalidOid ;
      bool typeVarLength    = false ;
      getTypeOutputInfo ( constantTypeId, &outputFunctionId, &typeVarLength ) ;
      outputString = OidOutputFunctionCall ( outputFunctionId, constantValue ) ;
      sdbbson_append_string ( queryDocument, keyName, outputString ) ;
      break ;
   }
   case NAMEOID :
   {
      CHAR *outputString    = NULL ;
      Oid outputFunctionId  = InvalidOid ;
      bool typeVarLength    = false ;
      sdbbson_oid_t sdbbsonObjectId ;
      memset ( sdbbsonObjectId.bytes, 0, sizeof ( sdbbsonObjectId.bytes ) ) ;
      getTypeOutputInfo ( constantTypeId, &outputFunctionId, &typeVarLength ) ;
      outputString = OidOutputFunctionCall ( outputFunctionId, constantValue ) ;
      sdbbson_oid_from_string ( &sdbbsonObjectId, outputString ) ;
      sdbbson_append_oid ( queryDocument, keyName, &sdbbsonObjectId ) ;
      break ;
   }
   case DATEOID :
   {
      Datum valueDatum = DirectFunctionCall1 ( date_timestamp, constantValue ) ;
      Timestamp valueTimestamp = DatumGetTimestamp ( valueDatum ) ;
      INT64 valueMicroSecs = valueTimestamp + POSTGRES_TO_UNIX_EPOCH_USECS ;
      INT64 valueMilliSecs = valueMicroSecs / 1000 ;
      sdbbson_append_date ( queryDocument, keyName, valueMilliSecs ) ;
      break ;
   }
   case TIMESTAMPOID :
   case TIMESTAMPTZOID :
   {
      Timestamp valueTimestamp = DatumGetTimestamp ( constantValue ) ;
      INT64 valueMicroSecs = valueTimestamp + POSTGRES_TO_UNIX_EPOCH_USECS ;
      INT64 valueMilliSecs = valueMicroSecs / 1000 ;
      sdbbson_append_date ( queryDocument, keyName, valueMilliSecs ) ;
      break ;
   }
   default :
   {
      /* we do not support other data types */
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_INVALID_DATA_TYPE ),
                         errmsg ( "Cannot convert constant value to BSON" ),
                         errhint ( "Constant value data type: %u",
                                   constantTypeId ) ) ) ;
      break ;
   }
   }
done :
   return ;
}

/* sdbApplicableOpExpressionList iterate all operators and push the predicates
 * that's able to handled by sdb into sdb
 */
static List *sdbApplicableOpExpressionList ( RelOptInfo *baserel )
{
   List *opExpressionList     = NIL ;
   List *restrictInfoList     = baserel->baserestrictinfo ;
   ListCell *restrictInfoCell = NULL ;

   foreach ( restrictInfoCell, restrictInfoList )
   {
      RestrictInfo *restrictInfo = (RestrictInfo *) lfirst(restrictInfoCell);
      Expr *expression           = restrictInfo->clause ;
      NodeTag expressionType     = 0 ;
      OpExpr *opExpression       = NULL ;
      CHAR *operatorName         = NULL ;
      const CHAR *sdbOpName      = NULL ;
      List *argumentList         = NIL ;
      Var *column                = NULL ;
      Const *constant            = NULL ;
      BOOLEAN constantIsArray    = FALSE ;

      /* we only support operator expressions */
      expressionType = nodeTag ( expression ) ;
      if ( T_OpExpr != expressionType )
      {
         continue ;
      }
      opExpression = (OpExpr*) expression ;
      operatorName = get_opname ( opExpression->opno ) ;
      /* we only support > >= < <= <> = */
      sdbOpName    = sdbOperatorName ( operatorName ) ;
      if ( !sdbOpName )
      {
         continue ;
      }
      /* we only support simple binary operators */
      argumentList = opExpression->args ;
      column = (Var*)sdbFindArgumentOfType ( argumentList, T_Var ) ;
      constant = (Const*)sdbFindArgumentOfType ( argumentList, T_Const ) ;
      /* we skip array */
      if ( NULL != constant )
      {
         Oid constantArrayTypeId = get_element_type ( constant->consttype ) ;
         if ( constantArrayTypeId != InvalidOid )
         {
            constantIsArray = TRUE ;
         }
      }
      if ( NULL != column && NULL != constant && !constantIsArray )
      {
         opExpressionList = lappend ( opExpressionList, opExpression ) ;
      }
   }
   return opExpressionList ;
}

/* sdbColumnOperatorList finds all expressions for the given column
 */
static List *sdbColumnOperatorList ( Var *column, List *operatorList )
{
   List *columnOperatorList = NIL ;
   ListCell *operatorCell   = NULL ;
   foreach ( operatorCell, operatorList )
   {
      OpExpr *operator   = (OpExpr*) lfirst ( operatorCell ) ;
      List *argumentList = operator->args ;
      Var *foundColumn   = (Var*)sdbFindArgumentOfType ( argumentList, T_Var ) ;
      if ( equal ( column, foundColumn ) )
      {
         columnOperatorList = lappend ( columnOperatorList, operator ) ;
      }
   }
   return columnOperatorList ;
}

/* sdbColumnList takes the planner's information and extract all columns that
 * may be used in projections, joins, and filter clauses, de-duplicate those
 * columns and returns them in a new list
 */
static List *sdbColumnList ( RelOptInfo *baserel )
{
   List *columnList           = NIL ;
   List *neededColumnList     = NIL ;
   AttrNumber columnIndex     = 1 ;
   AttrNumber columnCount     = baserel->max_attr ;
   List *targetColumnList     = baserel->reltargetlist ;
   List *restrictInfoList     = baserel->baserestrictinfo ;
   ListCell *restrictInfoCell = NULL ;
   /* first add the columns used in joins and projections */
   neededColumnList = list_copy ( targetColumnList ) ;
   /* then walk over all restriction clauses, and pull up any used columns */
   foreach ( restrictInfoCell, restrictInfoList )
   {
      RestrictInfo *restrictInfo = (RestrictInfo *) lfirst(restrictInfoCell) ;
      Node *restrictClause       = (Node *) restrictInfo->clause ;
      List *clauseColumnList     = NIL ;
      /* recursively pull up any columns used in the restriction clauses */
      clauseColumnList = pull_var_clause ( restrictClause ,
                                           PVC_RECURSE_AGGREGATES,
                                           PVC_RECURSE_PLACEHOLDERS ) ;
      neededColumnList = list_union ( neededColumnList, clauseColumnList ) ;
   }
   /* walk over all column definitions and deduplicate column list */
   for (columnIndex = 1; columnIndex <= columnCount; columnIndex++ )
   {
      ListCell *neededColumnCell = NULL ;
      Var *column = NULL ;
      foreach ( neededColumnCell, neededColumnList )
      {
         Var *neededColumn = (Var *) lfirst(neededColumnCell) ;
         if ( neededColumn->varattno == columnIndex )
         {
            column = neededColumn ;
            break ;
         }
      }
      if ( NULL != column )
      {
         columnList = lappend ( columnList, column ) ;
      }
   }
   return columnList ;
}

/* sdbBuildQuery build sdbbson query from opExpressionList */
static INT32 sdbBuildQuery ( Oid relationId,
                             List *opExpressionList,
                             sdbbson *queryDocument )
{
   INT32 rc                       = SDB_OK ;
   List *columnList               = NIL ;
   ListCell *columnCell           = NULL ;

   /* first we need to group all expression with same column name together */
   columnList = sdbUniqueColumnList ( opExpressionList ) ;

   /* go through each element in column list and pick those columns from
    * expression list
    */
   foreach ( columnCell, columnList )
   {
      Var *column                  = (Var*) lfirst ( columnCell ) ;
      Oid columnId                 = InvalidOid ;
      CHAR *columnName             = NULL ;
      List *columnOperatorList     = NIL ;
      ListCell *columnOperatorCell = NULL ;
      columnId                     = column->varattno ;
      columnName                   = get_relid_attribute_name ( relationId,
                                                                columnId ) ;
      /* find all expression for the column */
      columnOperatorList = sdbColumnOperatorList ( column, opExpressionList ) ;
      /* for each expression, start a sub-document */
      sdbbson_append_start_object ( queryDocument, columnName ) ;
      /* add element into subdocument */
      foreach ( columnOperatorCell, columnOperatorList )
      {
         OpExpr *columnOperator   = (OpExpr *) lfirst ( columnOperatorCell ) ;
         const CHAR *operatorName = NULL ;
         const CHAR *sdbOpName    = NULL ;
         List *argumentList = columnOperator->args ;
         /* pickup all constant */
         Const *constant = (Const*)sdbFindArgumentOfType ( argumentList,
                                                           T_Const ) ;
         operatorName = get_opname ( columnOperator->opno ) ;
         sdbOpName = sdbOperatorName ( operatorName ) ;
         sdbAppendConstantValue ( queryDocument, sdbOpName, constant ) ;
         /* TODO: variable comparison may also needed */
      }
      sdbbson_append_finish_object ( queryDocument ) ;
   }
   sdbbson_finish ( queryDocument ) ;
   return rc ;
}

/* Iterate SdbInputOptionList and return all possible option name for
 * the given context id
 */
static StringInfo sdbListOptions ( const Oid currentContextID )
{
   StringInfo resultString = makeStringInfo () ;
   INT32 firstPrinted      = 0 ;
   INT32 i                 = 0 ;
   for ( i = 0; i < sizeof(SdbInputOptionList); ++i )
   {
      const SdbInputOption *inputOption = &(SdbInputOptionList[i]) ;
      /* if currentContextID matches the context, then let's append */
      if ( currentContextID == inputOption->optionContextID )
      {
         /* append ", " if it's not the first element */
         if ( firstPrinted )
         {
            appendStringInfoString ( resultString, ", " ) ;
         }
         appendStringInfoString ( resultString, inputOption->optionName ) ;
         firstPrinted = 1 ;
      }
   }
   return resultString ;
}

/* sdbGetOptionValue retrieve options info from PG catalog */
static CHAR *sdbGetOptionValue ( Oid foreignTableId, const CHAR *optionName )
{
   ForeignTable *ft     = NULL ;
   ForeignServer *fs    = NULL ;
   List *optionList     = NIL ;
   ListCell *optionCell = NULL ;
   CHAR *optionValue    = NULL ;
   /* retreive foreign table and server in order to get options */
   ft  = GetForeignTable ( foreignTableId ) ;
   fs = GetForeignServer ( ft->serverid ) ;
   /* add server and table options into list */
   optionList    = list_concat ( optionList, ft->options ) ;
   optionList    = list_concat ( optionList, fs->options ) ;
   /* iterate each option and match the option name */
   foreach ( optionCell, optionList )
   {
      DefElem *optionDef = (DefElem *) lfirst ( optionCell ) ;
      if ( strncmp ( optionDef->defname, optionName, NAMEDATALEN ) == 0 )
      {
         optionValue = defGetString ( optionDef ) ;
         goto done ;
      }
   }
done :
   return optionValue ;
}


/* getOptions retreive connection options from Oid */
static void sdbGetOptions ( Oid foreignTableId, SdbInputOptions *options )
{
   CHAR *addressName         = NULL ;
   CHAR *serviceName         = NULL ;
   CHAR *userName            = NULL ;
   CHAR *passwordName        = NULL ;
   CHAR *collectionspaceName = NULL ;
   CHAR *collectionName      = NULL ;
   if ( NULL == options )
      goto done ;
   /* address name */
   addressName = sdbGetOptionValue ( foreignTableId,
                                     OPTION_NAME_ADDRESS ) ;
   if ( NULL == addressName )
   {
      addressName = pstrdup ( DEFAULT_HOSTNAME ) ;
   }

   /* service name */
   serviceName = sdbGetOptionValue ( foreignTableId,
                                     OPTION_NAME_SERVICE ) ;
   if ( NULL == serviceName )
   {
      serviceName = pstrdup ( DEFAULT_SERVICENAME ) ;
   }

   /* user name */
   userName = sdbGetOptionValue ( foreignTableId,
                                  OPTION_NAME_USER ) ;
   if ( NULL == userName )
   {
      userName = pstrdup ( DEFAULT_USERNAME ) ;
   }

   /* password name */
   passwordName = sdbGetOptionValue ( foreignTableId,
                                      OPTION_NAME_PASSWORD ) ;
   if ( NULL == passwordName )
   {
      passwordName = pstrdup ( DEFAULT_PASSWORDNAME ) ;
   }

   /* collectionspace name */
   collectionspaceName = sdbGetOptionValue ( foreignTableId,
                                             OPTION_NAME_COLLECTIONSPACE ) ;
   if ( NULL == collectionspaceName )
   {
      /* if collectionspace name is not provided, let's use the schema name
       * of the table
       */
      collectionspaceName = get_namespace_name (
            get_rel_namespace ( foreignTableId ) ) ;
   }

   /* collection name */
   collectionName = sdbGetOptionValue ( foreignTableId,
                                        OPTION_NAME_COLLECTION ) ;
   if ( NULL == collectionName )
   {
      /* if collection name is not provided, let's use the table name */
      collectionName = get_rel_name ( foreignTableId ) ;
   }

   /* fill up the result structure */
   options->address         = addressName ;
   options->service         = serviceName ;
   options->user            = userName ;
   options->password        = passwordName ;
   options->collectionspace = collectionspaceName ;
   options->collection      = collectionName ;
done :
   return ;
}

/* find a connection from pool, if the connection does not exist, let's create
 * one
 */
static INT32 sdbGetConnection ( Oid foreignTableId,
                                SdbConnection **connect )
{
   INT32 rc                        = SDB_OK ;
   INT32 count                     = 0 ;
   StringInfo connName             = makeStringInfo () ;
   sdbConnectionHandle hConnection = SDB_INVALID_HANDLE ;
   SdbConnectionPool *pool         = NULL ;
   SdbInputOptions options ;
   sdbGetOptions ( foreignTableId, &options ) ;

   /* connection string is address + service + user + password */
   appendStringInfo ( connName, "%s:%s:%s:%s", options.address,
                      options.service, options.user, options.password ) ;
   /* iterate all connections in pool */
   pool = sdbGetConnectionPool() ;
   for ( count = 0; count < pool->numConnections; ++count )
   {
      if ( strcmp ( pool->connList[count].connName, connName->data ) == 0 )
      {
         *connect = &pool->connList[count] ;
         goto done ;
      }
   }
   /* when we get here, we don't have the connection so let's create one */
   rc = sdbConnect ( options.address,
                     options.service,
                     options.user,
                     options.password,
                     &hConnection ) ;
   if ( rc )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_UNABLE_TO_ESTABLISH_CONNECTION ),
                         errmsg ( "unable to establish connection to \"%s:%s\""
                                  ", rc = %d",
                                  options.address, options.service, rc ),
                         errhint ( "Make sure remote service is running "
                                   "and username/password are valid" ) ) ) ;
      goto error ;
   }
   /* add connection into pool */
addpool:
   if ( pool->poolSize > pool->numConnections )
   {
      *connect = &pool->connList[pool->numConnections] ;
      (*connect)->connName = strdup ( connName->data ) ;
      (*connect)->hConnection = hConnection ;
      (*connect)->transLevel = 0 ;
      pool->numConnections ++ ;
   }
   else
   {
      /* allocate new slots */
      SdbConnection *pNewMem = pool->connList ;
      INT32 poolSize = pool->poolSize ;
      poolSize = poolSize << 1 ;
      pNewMem = (SdbConnection*)realloc ( pNewMem,
            sizeof(SdbConnection) * poolSize ) ;
      if ( !pNewMem )
      {
         ereport ( ERROR, ( errcode ( ERRCODE_FDW_OUT_OF_MEMORY ),
                            errmsg ( "Unable to allocate connection pool" ),
                            errhint ( "Make sure the memory pool or ulimit is "
                                      "properly configured" ) ) ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      pool->connList = pNewMem ;
      pool->poolSize = poolSize ;
      goto addpool ;
   }
done :
   return rc ;
error :
   goto done ;
}

/* validator validates for:
 * foreign data wrapper
 * server
 * user mapping
 * foreign table
 */
Datum sdb_fdw_validator ( PG_FUNCTION_ARGS )
{
   Datum optionArray    = PG_GETARG_DATUM ( 0 ) ;
   Oid optionContextId  = PG_GETARG_OID ( 1 ) ;
   List *optionList     = untransformRelOptions ( optionArray ) ;
   ListCell *optionCell = NULL ;

   /* iterate each element in option */
   foreach ( optionCell, optionList )
   {
      DefElem *option  = ( DefElem * ) lfirst ( optionCell ) ;
      CHAR *optionName = option->defname ;
      INT32 i = 0 ;
      /* find out which option it is */
      for ( i = 0;
            i < sizeof ( SdbInputOptionList ) / sizeof( SdbInputOption );
            ++i )
      {
         const SdbInputOption *inputOption = &(SdbInputOptionList[i]) ;
         /* compare type and name */
         if ( optionContextId == inputOption->optionContextID &&
              strncmp ( optionName, inputOption->optionName,
                        NAMEDATALEN ) == 0 )
         {
            /* if we find the option, let's jump out */
            break ;
         }
      }
      /* if we don't find any match */
      if ( sizeof( SdbInputOptionList ) / sizeof( SdbInputOption ) == i )
      {
         StringInfo optionNames = sdbListOptions ( optionContextId ) ;
         ereport ( ERROR, ( errcode ( ERRCODE_FDW_INVALID_OPTION_NAME ),
                            errmsg ( "invalid option \"%s\"", optionName ),
                            errhint ( "Valid options in this context are: %s",
                                      optionNames->data ) ) ) ;
      }
      /* make sure the port is integer */
      if ( strncmp ( optionName, OPTION_NAME_SERVICE, NAMEDATALEN ) == 0 )
      {
         CHAR *optionValue = defGetString ( option ) ;
         INT32 portNumber  = pg_atoi ( optionValue, sizeof(INT32), 0 ) ;
         (void) portNumber ;
      }
   }
   PG_RETURN_VOID () ;
}

/* sdbColumnMappingHash creates a hash table to map sdbbson fields into PG column
 * index
 */
static HTAB *sdbColumnMappingHash ( Oid foreignTableId,
                                    List *columnList )
{
   ListCell *columnCell     = NULL ;
   const long hashTableSize = 2048 ;
   HTAB *columnMappingHash  = NULL ;
   /* create hash table */
   HASHCTL hashInfo ;
   memset ( &hashInfo, 0, sizeof(hashInfo) ) ;
   hashInfo.keysize = NAMEDATALEN ;
   hashInfo.entrysize = sizeof(SdbColumnMapping) ;
   hashInfo.hash = string_hash ;
   hashInfo.hcxt = CurrentMemoryContext ;

   /* create hash table */
   columnMappingHash = hash_create ( "Column Mapping Hash",
                                     hashTableSize,
                                     &hashInfo,
                                     (HASH_ELEM | HASH_FUNCTION | HASH_CONTEXT )
                                    ) ;
   if ( !columnMappingHash )
   {
      goto error ;
   }
   /* iterate each column */
   foreach ( columnCell, columnList )
   {
      Var *column                     = (Var*) lfirst ( columnCell ) ;
      AttrNumber columnId             = column->varattno ;
      SdbColumnMapping *columnMapping = NULL ;
      CHAR *columnName                = NULL ;
      bool handleFound                = false ;
      void *hashKey                   = NULL ;

      columnName = get_relid_attribute_name ( foreignTableId, columnId ) ;
      hashKey = (void *) columnName ;

      /* validate each column only appears once, HASH_ENTER means we want to
       * enter the entry into hash table if not found
       */
      columnMapping = (SdbColumnMapping*)hash_search ( columnMappingHash,
                                                       hashKey,
                                                       HASH_ENTER,
                                                       &handleFound ) ;
      if ( !columnMapping )
      {
         goto error ;
      }
      columnMapping->columnIndex       = columnId - 1 ;
      columnMapping->columnTypeId      = column->vartype ;
      columnMapping->columnTypeMod     = column->vartypmod ;
      columnMapping->columnArrayTypeId = get_element_type ( column->vartype ) ;
   }
done :
   return columnMappingHash ;
error :
   goto done ;
}

/* sdbColumnTypesCompatible checks if a sdbbson type is compatible with PG type */
static BOOLEAN sdbColumnTypesCompatible ( sdbbson_type sdbbsonType, Oid columnTypeId )
{
   BOOLEAN compatibleType = FALSE ;
   switch ( columnTypeId )
   {
   case BOOLOID :
      if ( BSON_BOOL == sdbbsonType )
      {
         compatibleType = TRUE ;
      }
      /* do not break here since we also want to compatible with int/long/double
       * for boolean type
       */
   case INT2OID :
   case INT4OID :
   case INT8OID :
   case FLOAT4OID :
   case FLOAT8OID :
   case NUMERICOID :
   {
      if ( BSON_INT == sdbbsonType || BSON_LONG == sdbbsonType ||
           BSON_DOUBLE == sdbbsonType )
      {
         compatibleType = TRUE ;
      }
      break ;
   }
   case BPCHAROID :
   case VARCHAROID :
   case TEXTOID :
   {
      if ( BSON_STRING == sdbbsonType )
         compatibleType = TRUE ;
      break ;
   }
   case NAMEOID :
   {
      if ( BSON_OID == sdbbsonType )
         compatibleType = TRUE ;
      break ;
   }
   case DATEOID :
   case TIMESTAMPOID :
   case TIMESTAMPTZOID :
   {
      if ( BSON_DATE == sdbbsonType )
         compatibleType = TRUE ;
      break ;
   }
   default :
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_INVALID_DATA_TYPE ),
                         errmsg ( "cannot convert sdbbson type to column type" ),
                         errhint ( "Column type: %u",
                                   (UINT32)columnTypeId ) ) ) ;
      break ;
   }
   }
   return compatibleType ;
}

/* sdbColumnValue converts sdbbson value into PG datum
 */
static Datum sdbColumnValue ( sdbbson_iterator *sdbbsonIterator, Oid columnTypeId,
                              INT32 columnTypeMod )
{
   Datum columnValue = 0 ;
   switch ( columnTypeId )
   {
   case INT2OID :
   {
      INT16 value = (INT16) sdbbson_iterator_int ( sdbbsonIterator ) ;
      columnValue = Int16GetDatum ( value ) ;
      break ;
   }
   case INT4OID :
   {
      INT32 value = sdbbson_iterator_int ( sdbbsonIterator ) ;
      columnValue = Int32GetDatum ( value ) ;
      break ;
   }
   case INT8OID :
   {
      INT64 value = sdbbson_iterator_long ( sdbbsonIterator ) ;
      columnValue = Int64GetDatum ( value ) ;
      break ;
   }
   case FLOAT4OID :
   {
      FLOAT32 value = (FLOAT32)sdbbson_iterator_double ( sdbbsonIterator ) ;
      columnValue = Float4GetDatum ( value ) ;
      break ;
   }
   case FLOAT8OID :
   {
      FLOAT64 value = sdbbson_iterator_double ( sdbbsonIterator ) ;
      columnValue = Float8GetDatum ( value ) ;
      break ;
   }
   case NUMERICOID :
   {
      FLOAT64 value = sdbbson_iterator_double ( sdbbsonIterator ) ;
      Datum valueDatum = Float8GetDatum ( value ) ;
      columnValue = DirectFunctionCall1 ( float8_numeric, valueDatum ) ;
      break ;
   }
   case BOOLOID :
   {
      BOOLEAN value = sdbbson_iterator_bool ( sdbbsonIterator ) ;
      columnValue = BoolGetDatum ( value ) ;
      break ;
   }
   case BPCHAROID :
   {
      const CHAR *value = sdbbson_iterator_string ( sdbbsonIterator ) ;
      Datum valueDatum = CStringGetDatum ( value ) ;
      columnValue = DirectFunctionCall3 ( bpcharin, valueDatum,
                                          ObjectIdGetDatum ( InvalidOid ),
                                          Int32GetDatum ( columnTypeMod ) ) ;
      break ;
   }
   case VARCHAROID :
   {
      const CHAR *value = sdbbson_iterator_string ( sdbbsonIterator ) ;
      Datum valueDatum = CStringGetDatum ( value ) ;
      columnValue = DirectFunctionCall3 ( varcharin, valueDatum,
                                          ObjectIdGetDatum ( InvalidOid ),
                                          Int32GetDatum ( columnTypeMod ) ) ;
      break ;
   }
   case TEXTOID :
   {
      const CHAR *value = sdbbson_iterator_string ( sdbbsonIterator ) ;
      columnValue       = CStringGetTextDatum ( value ) ;
      break ;
   }
   case NAMEOID :
   {
      CHAR value [ NAMEDATALEN ] = {0} ;
      Datum valueDatum = 0 ;
      sdbbson_oid_t *sdbbsonObjectId = sdbbson_iterator_oid ( sdbbsonIterator ) ;
      sdbbson_oid_to_string ( sdbbsonObjectId, value ) ;
      valueDatum               = CStringGetDatum ( value ) ;
      columnValue = DirectFunctionCall3 ( namein, valueDatum,
                                          ObjectIdGetDatum ( InvalidOid ),
                                          Int32GetDatum ( columnTypeMod ) ) ;
      break ;
   }
   case DATEOID :
   {
      INT64 valueMillis    = sdbbson_iterator_date ( sdbbsonIterator ) ;
      INT64 timestamp      = ( valueMillis * 1000L ) - POSTGRES_TO_UNIX_EPOCH_USECS ;
      Datum timestampDatum = TimestampGetDatum ( timestamp ) ;
      columnValue = DirectFunctionCall1 ( timestamp_date, timestampDatum ) ;
      break ;
   }
   case TIMESTAMPOID :
   case TIMESTAMPTZOID :
   {
      INT64 valueMillis = sdbbson_iterator_date ( sdbbsonIterator ) ;
      INT64 timestamp   = ( valueMillis * 1000L ) -
                          POSTGRES_TO_UNIX_EPOCH_USECS ;
      columnValue       = TimestampGetDatum ( timestamp ) ;
      break ;
   }
   default :
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_INVALID_DATA_TYPE ),
                         errmsg ( "cannot convert sdbbson type to column type" ),
                         errhint ( "Column type: %u",
                                   (UINT32)columnTypeId ) ) ) ;
      break ;
   }
   }
   return columnValue ;
}

/* sdbFreeScanState closes the cursor, connection and collection to SequoiaDB
 */
static void sdbFreeScanState ( SdbExecState *executionState )
{
   if ( !executionState )
      goto done ;
   if ( executionState->queryDocument )
      sdbbson_dispose ( executionState->queryDocument ) ;
   if ( SDB_INVALID_HANDLE != executionState->hCursor )
      sdbReleaseCursor ( executionState->hCursor ) ;
   if ( SDB_INVALID_HANDLE != executionState->hCollection )
      sdbReleaseCollection ( executionState->hCollection ) ;
   /* do not free collection since it's in pool */
done :
   return ;
}

/* sdbColumnValueArray build array
 */
static Datum sdbColumnValueArray ( sdbbson_iterator *sdbbsonIterator,
                                   Oid valueTypeId )
{
   UINT32 arrayCapacity          = INITIAL_ARRAY_CAPACITY ;
   UINT32 arrayGrowthFactor      = 2 ;
   UINT32 arrayIndex             = 0 ;
   ArrayType *columnValueObject  = NULL ;
   Datum columnValueDatum        =  0 ;
   bool typeByValue              = false ;
   CHAR typeAlignment            = 0 ;
   INT16 typeLength              = 0 ;
   sdbbson_iterator sdbbsonSubIterator = { NULL, 0 } ;
   Datum *columnValueArray       = NULL ;
   sdbbson_iterator_subiterator ( sdbbsonIterator, &sdbbsonSubIterator ) ;
   columnValueArray = (Datum*)palloc0 ( INITIAL_ARRAY_CAPACITY *
                                        sizeof(Datum)) ;
   if ( !columnValueArray )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_OUT_OF_MEMORY ),
                         errmsg ( "Unable to allocate array memory" ),
                         errhint ( "Make sure the memory pool or ulimit is "
                                   "properly configured" ) ) ) ;
      goto error ;
   }
   /* go through each element in array */
   while ( sdbbson_iterator_next ( &sdbbsonSubIterator ) )
   {
      sdbbson_type sdbbsonType = sdbbson_iterator_type ( &sdbbsonSubIterator ) ;
      BOOLEAN compatibleTypes = FALSE ;
      compatibleTypes = sdbColumnTypesCompatible ( sdbbsonType, valueTypeId ) ;
      /* skip the element if it's not compatible */
      if ( BSON_NULL == sdbbsonType || !compatibleTypes )
      {
         continue ;
      }
      if ( arrayIndex >= arrayCapacity )
      {
         arrayCapacity *= arrayGrowthFactor ;
         columnValueArray = repalloc ( columnValueArray,
                                       arrayCapacity * sizeof(Datum) ) ;
      }
      /* use default type modifier to convert column value */
      columnValueArray[arrayIndex] = sdbColumnValue ( &sdbbsonSubIterator,
                                                      valueTypeId, 0 ) ;
      ++arrayIndex ;
   }
done :
   get_typlenbyvalalign ( valueTypeId, &typeLength, &typeByValue,
                          &typeAlignment ) ;
   columnValueObject = construct_array ( columnValueArray, arrayIndex,
                                         valueTypeId, typeLength,
                                         typeByValue, typeAlignment ) ;
   columnValueDatum = PointerGetDatum ( columnValueObject ) ;
   return columnValueDatum ;
error :
   goto done ;
}

/* sdbFillTupleSlot go through sdbbson document and the hash table, try to build
 * the tuple for PG
 * documentKey: if we want to find things in nested sdbbson object
 */
static void sdbFillTupleSlot ( const sdbbson *sdbbsonDocument,
                               const CHAR *documentKey,
                               HTAB *columnMappingHash,
                               Datum *columnValues,
                               bool *columnNulls )
{
   sdbbson_iterator sdbbsonIterator = { NULL, 0 } ;
   sdbbson_iterator_init ( &sdbbsonIterator, sdbbsonDocument ) ;

   /* for each element in sdbbson object */
   while ( sdbbson_iterator_next ( &sdbbsonIterator ) )
   {
      const CHAR *sdbbsonKey = sdbbson_iterator_key ( &sdbbsonIterator ) ;
      sdbbson_type sdbbsonType = sdbbson_iterator_type ( &sdbbsonIterator ) ;
      SdbColumnMapping *columnMapping = NULL ;
      Oid columnTypeId                = InvalidOid ;
      Oid columnArrayTypeId           = InvalidOid ;
      BOOLEAN compatibleTypes         = FALSE ;
      bool handleFound                = false ;
      const CHAR *sdbbsonFullKey         = NULL ;
      void *hashKey                   = NULL ;

      /*
       * if we have object
       * {
       *    a: {
       *       A: 1,
       *       B: 2
       *    },
       *    b: "hello"
       * }
       * first round we have documentKey=NULL, then sdbbsonKey=a
       * next we have nested object, then recursively we call sdbFillTupleSlot
       * so we have
       * documentKey=a, sdbbsonKey=A, then we have sdbbsonFullKey=a.A
       */
      if ( documentKey )
      {
         /* if we want to find entries in nested object, we should use this one
          */
         StringInfo sdbbsonFullKeyString = makeStringInfo () ;
         appendStringInfo ( sdbbsonFullKeyString, "%s.%s", documentKey,
                            sdbbsonKey ) ;
         sdbbsonFullKey = sdbbsonFullKeyString->data ;
      }
      else
      {
         sdbbsonFullKey = sdbbsonKey ;
      }
      /* recurse into nested objects */
      if ( BSON_OBJECT == sdbbsonType )
      {
         sdbbson subObject ;
         sdbbson_iterator_subobject ( &sdbbsonIterator, &subObject ) ;
         sdbFillTupleSlot ( &subObject, sdbbsonFullKey,
                            columnMappingHash, columnValues, columnNulls ) ;
         continue ;
      }
      /* match columns for sdbbson key */
      hashKey = (void*)sdbbsonFullKey ;
      columnMapping = (SdbColumnMapping*)hash_search ( columnMappingHash,
                                                       hashKey,
                                                       HASH_FIND,
                                                       &handleFound ) ;
      /* if we cannot find the column, or if the sdbbson type is null, let's just
       * leave it as null
       */
      if ( NULL == columnMapping || BSON_NULL == sdbbsonType )
      {
         continue ;
      }

      /* check if columns have compatible types */
      columnTypeId = columnMapping->columnTypeId ;
      columnArrayTypeId = columnMapping->columnArrayTypeId ;
      if ( OidIsValid ( columnArrayTypeId ) && sdbbsonType == BSON_ARRAY )
      {
         compatibleTypes = TRUE ;
      }
      else
      {
         compatibleTypes = sdbColumnTypesCompatible ( sdbbsonType, columnTypeId ) ;
      }

      /* if types are incompatible, leave this column null */
      if ( !compatibleTypes )
      {
         continue ;
      }
      /* fill in corresponding column values and null flag */
      if ( OidIsValid ( columnArrayTypeId ) )
      {
         INT32 columnIndex = columnMapping->columnIndex ;
         columnValues[columnIndex] = sdbColumnValueArray ( &sdbbsonIterator,
                                                           columnArrayTypeId ) ;
         columnNulls[columnIndex] = false ;
      }
      else
      {
         INT32 columnIndex = columnMapping->columnIndex ;
         Oid columnTypeMod = columnMapping->columnTypeMod ;
         columnValues[columnIndex] = sdbColumnValue ( &sdbbsonIterator,
                                                      columnTypeId,
                                                      columnTypeMod ) ;
         columnNulls[columnIndex] = false  ;
      }
   }
}

/* sdbRowsCount get count of records in the given collection */
static INT32 sdbRowsCount ( Oid foreignTableId, SINT64 *count )
{
   INT32 rc                        = SDB_OK ;
   sdbCollectionHandle hCollection = SDB_INVALID_HANDLE ;
   SdbConnection *connection       = NULL ;
   SdbInputOptions options ;
   StringInfo fullCollectionName = makeStringInfo () ;

   sdbGetOptions ( foreignTableId, &options ) ;
   /* attempt to connect to remote database */
   rc = sdbGetConnection ( foreignTableId, &connection ) ;
   if ( rc )
   {
      goto error ;
   }
   /* get collection */
   appendStringInfoString ( fullCollectionName, options.collectionspace ) ;
   appendStringInfoString ( fullCollectionName, "." ) ;
   appendStringInfoString ( fullCollectionName, options.collection ) ;
   rc = sdbGetCollection ( connection->hConnection,
                           fullCollectionName->data,
                           &hCollection ) ;
   if ( rc )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_ERROR ),
                         errmsg ( "Unable to get collection \"%s\", rc = %d",
                                  fullCollectionName->data, rc ),
                         errhint ( "Make sure the collectionspace and "
                                   "collection exist on the remote database" )
                        ) ) ;
      goto error ;
   }
   /* get count */
   rc = sdbGetCount ( hCollection, NULL, count ) ;
   if ( rc )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_ERROR ),
                         errmsg ( "Unable to get count for \"%s\", rc = %d",
                                  fullCollectionName->data, rc ),
                         errhint ( "Make sure the collectionspace and "
                                   "collection exist on the remote database" )
                        ) ) ;
      goto error ;
   }
done :
   if ( SDB_INVALID_HANDLE != hCollection )
      sdbReleaseCollection ( hCollection ) ;
   return rc ;
error :
   goto done ;
}

/* Get the estimation size for Sdb foreign table
 * This function is also responsible to establish connection to remote table
 */
static void SdbGetForeignRelSize ( PlannerInfo *root,
                                   RelOptInfo *baserel,
                                   Oid foreignTableId )
{
   INT32 rc        = SDB_OK ;
   INT64 rowsCount = -1 ;

   rc = sdbRowsCount ( foreignTableId, &rowsCount ) ;
   if ( rc )
      goto error ;
   if ( rowsCount >= 0 )
   {
      List *rowClauseList = baserel->baserestrictinfo ;
      FLOAT64 rowSelectivity = clauselist_selectivity ( root, rowClauseList,
                                                        0, JOIN_INNER, NULL ) ;
      FLOAT64 outputRowCount = clamp_row_est ( rowsCount * rowSelectivity ) ;
      baserel->rows = outputRowCount ;
   }
   else
   {
      ereport ( DEBUG1, ( errmsg ( "Unable to retrieve the row count "
                                   "for collection" ),
                          errhint ( "Falling back to default estimates in "
                                    "planning" ) ) ) ;
   }
done :
   return  ;
error :
   goto done ;
}

/* SdbGetForeignPaths creates the scan path to execute query.
 * Sdb may decide to use underlying index, but the caller procedures do not
 * know whether it will happen or not.
 * Therefore, we simply create a table scan path
 * Estimation includes:
 * 1) row count
 * 2) disk cost
 * 3) cpu cost
 * 4) startup cost
 */
static void SdbGetForeignPaths ( PlannerInfo *root,
                                 RelOptInfo *baserel,
                                 Oid foreignTableId )
{
   INT32 rc                 = SDB_OK ;
   INT64 rowsCount          = 0 ;
   BlockNumber pageCount    = 0 ;
   INT32 rowWidth           = 0 ;
   FLOAT64 selectivity      = 0.0 ;
   FLOAT64 inputRowCount    = 0.0 ;
   FLOAT64 foreignTableSize = 0.0 ;
   List *opExpressionList   = NIL ;
   Path *foreignPath        = NULL ;

   /* cost estimation */
   FLOAT64 totalDiskCost    = 0.0 ;
   FLOAT64 totalCPUCost     = 0.0 ;
   FLOAT64 totalStartupCost = 0.0 ;
   FLOAT64 totalCost        = 0.0 ;

   rc = sdbRowsCount ( foreignTableId, &rowsCount ) ;
   if ( rc )
      goto error ;
   if ( rowsCount >= 0 )
   {
      /* rows estimation after applying predicates */
      opExpressionList = sdbApplicableOpExpressionList ( baserel ) ;
      selectivity      = clauselist_selectivity ( root, opExpressionList,
                                                  0, JOIN_INNER, NULL ) ;
      inputRowCount    = clamp_row_est ( rowsCount * selectivity ) ;

      /* disk cost estimation */
      rowWidth         = get_relation_data_width ( foreignTableId,
                                                   baserel->attr_widths ) ;
      foreignTableSize = rowWidth * rowsCount ;
      pageCount = ( BlockNumber ) rint ( foreignTableSize / BLCKSZ ) ;
      totalDiskCost = seq_page_cost * pageCount ;

      /* cpu cost estimation, it includes the record process time + return time
       * cpu_tuple_cost: time to process each row
       * SDB_TUPLE_COST_MULTIPLIER * cpu_tuple_cost: time to return a row
       * baserel->baserestrictcost.per_tuple: time to process a row in PG
       */
      totalCPUCost = cpu_tuple_cost * rowsCount +
                     ( SDB_TUPLE_COST_MULTIPLIER * cpu_tuple_cost +
                       baserel->baserestrictcost.per_tuple ) * inputRowCount ;

      /* startup cost estimation, which includes query execution startup time
       */
      totalStartupCost = baserel->baserestrictcost.startup ;

      /* total cost includes totalDiskCost + totalCPUCost + totalStartupCost
       */
      totalCost = totalStartupCost + totalCPUCost + totalDiskCost ;
   }
   else
   {
      ereport ( DEBUG1, ( errmsg ( "Unable to retreive row count for "
                                   "collection" ),
                          errhint ( "Falling back to default estimates in "
                                    "planning" ) ) ) ;
   }
done :
   /* create foreign path node */
   foreignPath = (Path*) create_foreignscan_path ( root, baserel, baserel->rows,
                                                   totalStartupCost, totalCost,
                                                   NIL, /* no pathkeys */
                                                   NULL, /* no outer rel */
                                                   NIL ) ; /* no fdw_private */
   /* add foreign path into path */
   add_path ( baserel, foreignPath ) ;
   return ;
error :
   goto done ;
}

/* SdbGetForeignPlan creates a foreign scan plan node for Sdb collection */
static ForeignScan *SdbGetForeignPlan ( PlannerInfo *root,
                                        RelOptInfo *baserel,
                                        Oid foreignTableId,
                                        ForeignPath *bestPath,
                                        List *targetList,
                                        List *restrictionClauses )
{
   INT32 rc                  = SDB_OK ;
   Index scanRangeTableIndex = baserel->relid ;
   ForeignScan *foreignScan  = NULL ;
   List *foreignPrivateList  = NIL ;
   List *opExpressionList    = NIL ;
   Const *queryBuffer        = NULL ;
   List *columnList          = NIL ;
   sdbbson queryDocument ;
   sdbbson_init ( &queryDocument ) ;
   /* We keep all restriction clauses at PG ide to re-check */
   restrictionClauses = extract_actual_clauses ( restrictionClauses, FALSE ) ;
   /* construct the query sdbbson document */
   opExpressionList = sdbApplicableOpExpressionList ( baserel ) ;
   rc = sdbBuildQuery ( foreignTableId, opExpressionList, &queryDocument ) ;
   if ( rc )
   {
      goto error ;
   }
   queryBuffer = sdbSerializeDocument ( &queryDocument ) ;
   /* copy document list */
   columnList = sdbColumnList ( baserel ) ;
   /* construct foreign plan with query predicates and column list */
   foreignPrivateList = list_make2 ( queryBuffer, columnList ) ;
done :
   /* create the foreign scan node */
   foreignScan =  make_foreignscan ( targetList, restrictionClauses,
                                     scanRangeTableIndex,
                                     NIL, /* no expressions to evaluate */
                                     foreignPrivateList ) ;
   /* object should NOT be destroyed since SerializeDocument does not make
    * memory copy
    * So sdbbson_destroy is only called when rc != SDB_OK ( that means no
    * SerializeDocument is called )
    */
   if ( rc )
   {
      sdbbson_destroy ( &queryDocument ) ;
   }
   return foreignScan ;
error :
   goto done ;
}

/* SdbExplainForeignScan produces output for EXPLAIN command
 */
static void SdbExplainForeignScan ( ForeignScanState *scanState,
                                    ExplainState *explainState )
{
   SdbInputOptions options ;
   StringInfo namespaceName = makeStringInfo () ;
   Oid foreignTableId = RelationGetRelid ( scanState->ss.ss_currentRelation ) ;
   sdbGetOptions ( foreignTableId, &options ) ;
   appendStringInfo ( namespaceName, "%s.%s",
                      options.collectionspace, options.collection ) ;
   ExplainPropertyText ( "Foreign Namespace", namespaceName->data,
                         explainState ) ;
}

/* SdbBeginForeignScan connects to sdb server and open a cursor to perform scan
 */
static void SdbBeginForeignScan ( ForeignScanState *scanState,
                                  INT32 executorFlags )
{
   SdbInputOptions options ;
   sdbbson *queryDocument             = NULL ;
   INT32 rc                        = SDB_OK ;
   sdbConnectionHandle hConnection = SDB_INVALID_HANDLE ;
   SdbConnection *connection       = NULL ;
   sdbCursorHandle hCursor         = SDB_INVALID_HANDLE ;
   sdbCollectionHandle hCollection = SDB_INVALID_HANDLE ;
   Oid foreignTableId              = InvalidOid ;
   ForeignScan *foreignScan        = NULL ;
   List *foreignPrivateList        = NIL ;
   Const *queryBuffer              = NULL ;
   List *columnList                = NIL ;
   HTAB *columnMappingHash         = NULL ;
   StringInfo namespaceName        = NULL ;
   SdbExecState *executionState    = NULL ;
   /* do not begin real scan if it's explain only */
   if ( executorFlags & EXEC_FLAG_EXPLAIN_ONLY )
   {
      goto done ;
   }
   queryDocument = sdbbson_create () ;
   if ( !queryDocument )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_OUT_OF_MEMORY ),
                         errmsg ( "Unable to allocate sdbbson object memory" ),
                         errhint ( "Make sure the memory pool or ulimit is "
                                   "properly configured" ) ) ) ;
      goto error ;
   }
   sdbbson_init ( queryDocument ) ;
   /* retreive target information */
   foreignTableId = RelationGetRelid ( scanState->ss.ss_currentRelation ) ;
   sdbGetOptions ( foreignTableId, &options ) ;
   /* attempt to connect to remote box */
   rc = sdbGetConnection ( foreignTableId, &connection ) ;
   if ( rc )
   {
      goto error ;
   }
   /* deserialize query document, and create column info hash */
   foreignScan = (ForeignScan *) scanState->ss.ps.plan ;
   foreignPrivateList = foreignScan->fdw_private ;
   queryBuffer = (Const *) linitial ( foreignPrivateList ) ;
   sdbDeserializeDocument ( queryBuffer, queryDocument ) ;
   columnList = (List *) lsecond ( foreignPrivateList ) ;
   /* build hash map */
   columnMappingHash = sdbColumnMappingHash ( foreignTableId, columnList ) ;
   /* attempt to query */
   namespaceName = makeStringInfo() ;
   appendStringInfo ( namespaceName, "%s.%s", options.collectionspace,
                      options.collection ) ;
   /* fire up query */
   rc = sdbGetCollection ( connection->hConnection, namespaceName->data,
                           &hCollection ) ;
   if ( rc )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_ERROR ),
                         errmsg ( "unable to get collection for \"%s.%s\""
                                  ", rc = %d",
                                  options.collectionspace,
                                  options.collection, rc ),
                         errhint ( "Make sure collection exists on remote "
                                   "SequoiaDB database" ) ) ) ;
      goto error ;
   }
   rc = sdbQuery ( hCollection, queryDocument, NULL, NULL, NULL, 0, -1,
                   &hCursor ) ;
   if ( rc )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_ERROR ),
                         errmsg ( "unable to query collection for \"%s.%s\""
                                  ", rc = %d",
                                  options.collectionspace,
                                  options.collection, rc ),
                         errhint ( "Make sure collection exists on remote "
                                   "SequoiaDB database" ) ) ) ;
      goto error ;
   }
   /* allocate new execution state */
   executionState = (SdbExecState*)palloc0 ( sizeof ( SdbExecState ) ) ;
   if ( !executionState )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_OUT_OF_MEMORY ),
                         errmsg ( "Unable to allocate execution state memory" ),
                         errhint ( "Make sure the memory pool or ulimit is "
                                   "properly configured" ) ) ) ;
      goto error ;
   }
   executionState->planType          = SDB_PLAN_SCAN ;
   executionState->columnMappingHash = columnMappingHash ;
   executionState->hConnection       = hConnection ;
   executionState->hCursor           = hCursor ;
   executionState->hCollection       = hCollection ;
   executionState->queryDocument     = queryDocument ;
   scanState->fdw_state = (void*) executionState ;
done :
   /* note we should not release cursor and connection */
   return ;
error :
   if ( queryDocument )
   {
      sdbbson_dispose ( queryDocument ) ;
   }
   goto done ;
}

/* SdbIterateForeignScan reads the next record from SequoiaDB and converts into
 * PostgreSQL tuple
 */
static TupleTableSlot * SdbIterateForeignScan ( ForeignScanState *scanState )
{
   sdbbson recordObj ;
   INT32 rc                     = SDB_OK ;
   SdbExecState *executionState = (SdbExecState*)scanState->fdw_state ;
   TupleTableSlot *tupleSlot    = scanState->ss.ss_ScanTupleSlot ;
   TupleDesc tupleDescriptor    = tupleSlot->tts_tupleDescriptor ;
   Datum *columnValues          = tupleSlot->tts_values ;
   bool *columnNulls            = tupleSlot->tts_isnull ;
   INT32 columnCount            = tupleDescriptor->natts ;
   const CHAR *sdbbsonDocumentKey  = NULL ;

   sdbbson_init ( &recordObj ) ;
   /* if there's nothing more to fetch, we return empty slot to represent
    * there's no more data to read
    */
   ExecClearTuple ( tupleSlot ) ;
   /* initialize all values */
   memset ( columnValues, 0, columnCount * sizeof ( Datum ) ) ;
   memset ( columnNulls, TRUE, columnCount * sizeof(BOOLEAN) ) ;

   /* cursor read next */
   rc = sdbNext ( executionState->hCursor, &recordObj ) ;
   if ( rc )
   {
      if ( SDB_DMS_EOC != rc )
      {
         sdbFreeScanState ( executionState ) ;
         /* if other error happened, let's report them */
         ereport ( ERROR, ( errcode ( ERRCODE_FDW_ERROR ),
                            errmsg ( "unable to fetch next record"
                                     ", rc = %d", rc ),
                            errhint ( "Make sure collection exists on remote "
                                      "SequoiaDB database, and the server "
                                      "is still up and running" ) ) ) ;
         goto error ;
      }
      /* if we get EOC, let's just goto done to return empty tupleSlot */
      goto done ;
   }
   sdbFillTupleSlot ( &recordObj, sdbbsonDocumentKey,
                      executionState->columnMappingHash,
                      columnValues, columnNulls ) ;
   ExecStoreVirtualTuple ( tupleSlot ) ;
done :
   sdbbson_destroy ( &recordObj ) ;
   return tupleSlot ;
error :
   goto done ;
}

/* SdbRescanForeignScan rescans the foreign table
 */
static void SdbRescanForeignScan ( ForeignScanState *scanState )
{
   INT32 rc                     = SDB_OK ;
   SdbExecState *executionState = ( SdbExecState * ) scanState->fdw_state ;
   if ( !executionState )
      goto error ;
   /* kill the cursor and start up a new one */
   sdbReleaseCursor ( executionState->hCursor ) ;
   rc = sdbQuery ( executionState->hCollection, executionState->queryDocument,
                   NULL, NULL, NULL, 0, -1,
                   &executionState->hCursor ) ;
   if ( rc )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_ERROR ),
                         errmsg ( "unable to rescan collection"
                                  ", rc = %d", rc ),
                         errhint ( "Make sure collection exists on remote "
                                   "SequoiaDB database" ) ) ) ;
      goto error ;
   }
done :
   return ;
error :
   goto done ;
}

/* SdbEndForeignScan finish scanning the foreign table
 */
static void SdbEndForeignScan ( ForeignScanState *scanState )
{
   SdbExecState *executionState = ( SdbExecState * ) scanState->fdw_state ;
   if ( executionState )
   {
      sdbFreeScanState ( executionState ) ;
   }
}

/* sdbAcquireSampleRows acquires a random sample of rows from the foreign table.
 */
static INT32 sdbAcquireSampleRows ( Relation relation, INT32 errorLevel,
                                    HeapTuple *sampleRows, INT32 targetRowCount,
                                    FLOAT64 *totalRowCount,
                                    FLOAT64 *totalDeadRowCount )
{
   INT32 rc                          = SDB_OK ;
   INT32 sampleRowCount              = 0 ;
   INT64 rowCount                    = 0 ;
   INT64 rowCountToSkip              = -1 ;
   FLOAT64 randomState               = 0 ;
   Datum *columnValues               = NULL ;
   bool *columnNulls                 = NULL ;
   Oid foreignTableId                = InvalidOid ;
   TupleDesc tupleDescriptor         = NULL ;
   Form_pg_attribute *attributesPtr  = NULL ;
   AttrNumber columnCount            = 0 ;
   AttrNumber columnId               = 0 ;
   HTAB *columnMappingHash           = NULL ;
   sdbCursorHandle hCursor           = SDB_INVALID_HANDLE ;
   List *columnList                  = NIL ;
   Const *queryBuffer                = NULL ;
   ForeignScanState *scanState       = NULL ;
   List *foreignPrivateList          = NIL ;
   ForeignScan *foreignScan          = NULL ;
   SdbExecState *executionState      = NULL ;
   CHAR *relationName                = NULL ;
   INT32 executorFlags               = 0 ;
   MemoryContext oldContext          = CurrentMemoryContext ;
   MemoryContext tupleContext        = NULL ;
   sdbbson queryDocument ;

   sdbbson_init ( &queryDocument ) ;

   /* create columns in the relation */
   tupleDescriptor = RelationGetDescr ( relation ) ;
   columnCount = tupleDescriptor->natts ;
   attributesPtr = tupleDescriptor->attrs ;

   for ( columnId = 1; columnId <= columnCount; ++columnId )
   {
      Var *column       = (Var*)palloc0(sizeof(Var)) ;
      if ( !column )
      {
         ereport ( ERROR, ( errcode ( ERRCODE_FDW_OUT_OF_MEMORY ),
                            errmsg ( "Unable to allocate Var memory" ),
                            errhint ( "Make sure the memory pool or ulimit is "
                                      "properly configured" ) ) ) ;
         goto error ;
      }
      column->varattno  = columnId ;
      column->vartype   = attributesPtr[columnId-1]->atttypid ;
      column->vartypmod = attributesPtr[columnId-1]->atttypmod ;
      columnList        = lappend ( columnList, column ) ;
   }
   /* create state structure */
   scanState = makeNode ( ForeignScanState ) ;
   scanState->ss.ss_currentRelation = relation ;
   foreignTableId = RelationGetRelid ( relation ) ;
   rc = sdbBuildQuery ( foreignTableId, NIL, &queryDocument ) ;
   if ( rc )
   {
      goto error ;
   }
   queryBuffer = sdbSerializeDocument ( &queryDocument ) ;

   /* construct foreign plan */
   foreignPrivateList       = list_make2 ( queryBuffer, columnList ) ;
   foreignScan              = makeNode ( ForeignScan ) ;
   foreignScan->fdw_private = foreignPrivateList ;
   scanState->ss.ps.plan = (Plan *)foreignScan ;
   SdbBeginForeignScan ( scanState, executorFlags ) ;
   executionState = (SdbExecState*)scanState->fdw_state ;
   hCursor = executionState->hCursor ;
   columnMappingHash = executionState->columnMappingHash ;

   tupleContext = AllocSetContextCreate ( CurrentMemoryContext,
                                          "sdb_fdw temp context",
                                          ALLOCSET_DEFAULT_MINSIZE,
                                          ALLOCSET_DEFAULT_INITSIZE,
                                          ALLOCSET_DEFAULT_MAXSIZE ) ;
   /* prepare for sampling rows */
   randomState  = anl_init_selection_state ( targetRowCount ) ;
   columnValues = (Datum *)palloc0 ( columnCount * sizeof(Datum) ) ;
   columnNulls  = (bool*)palloc0 ( columnCount * sizeof(bool) ) ;
   if ( !columnValues || !columnNulls )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_OUT_OF_MEMORY ),
                         errmsg ( "Unable to allocate Var memory" ),
                         errhint ( "Make sure the memory pool or ulimit is "
                                   "properly configured" ) ) ) ;
      goto error ;
   }
   while ( TRUE )
   {
      sdbbson recordObj ;
      sdbbson_init ( &recordObj ) ;
      /* check for any break or terminate events */
      vacuum_delay_point() ;
      /* init all values for this row to null */
      memset ( columnValues, 0, columnCount * sizeof(Datum) ) ;
      memset ( columnNulls, true, columnCount * sizeof(bool) ) ;
      rc = sdbNext ( hCursor, &recordObj ) ;
      if ( rc )
      {
         sdbbson_destroy ( &recordObj ) ;
         if ( SDB_DMS_EOC != rc )
         {
            sdbFreeScanState ( executionState ) ;
            /* if other error happened, let's report them */
            ereport ( ERROR, ( errcode ( ERRCODE_FDW_ERROR ),
                               errmsg ( "unable to fetch next record"
                                        ", rc = %d", rc ),
                               errhint ( "Make sure collection exists on remote"
                                         " SequoiaDB database, and the server "
                                         "is still up and running" ) ) ) ;
            goto error ;
         }
         break ;
      }
      /* if we can read the next record */
      MemoryContextReset ( tupleContext ) ;
      MemoryContextSwitchTo ( tupleContext ) ;
      sdbFillTupleSlot ( &recordObj, NULL, columnMappingHash,
                         columnValues, columnNulls ) ;
      MemoryContextSwitchTo ( oldContext ) ;

      if ( sampleRowCount < targetRowCount )
      {
         sampleRows[sampleRowCount++] = heap_form_tuple ( tupleDescriptor,
                                                          columnValues,
                                                          columnNulls ) ;
      }
      else
      {
         if ( rowCountToSkip < 0 )
         {
            rowCountToSkip = anl_get_next_S ( rowCount, targetRowCount,
                                              &randomState ) ;
         }
         if ( rowCountToSkip <= 0 )
         {
            /* process the row if we skipped enough records */
            INT32 rowIndex = (INT32)(targetRowCount * anl_random_fract() ) ;
            heap_freetuple ( sampleRows[rowIndex] ) ;
            sampleRows[rowIndex] = heap_form_tuple ( tupleDescriptor,
                                                     columnValues,
                                                     columnNulls ) ;
         }
         rowCountToSkip -= 1 ;
      }
      rowCount += 1 ;
      sdbbson_destroy ( &recordObj ) ;
   }
   sdbFreeScanState ( executionState ) ;
done :
   MemoryContextDelete ( tupleContext ) ;
   pfree ( columnValues ) ;
   pfree ( columnNulls ) ;
   sdbbson_destroy ( &queryDocument ) ;

   /* get some result */
   relationName = RelationGetRelationName ( relation ) ;
   ereport ( errorLevel, (errmsg ( "\"%s\": collection contains %lld rows; "
                                   "%d rows in sample",
                                   relationName, rowCount, sampleRowCount ))) ;
   (*totalRowCount) = rowCount ;
   (*totalDeadRowCount) = 0 ;
   return sampleRowCount ;
error :
   goto done ;
}

/* SdbAnalyzeForeignTable collects statistics for the given foreign table
 */
static bool SdbAnalyzeForeignTable (
      Relation relation,
      AcquireSampleRowsFunc *acquireSampleRowsFunc,
      BlockNumber *totalPageCount )
{
   INT32 rc                 = SDB_OK ;
   BlockNumber pageCount    = 0 ;
   INT32 attributeCount     = 0 ;
   INT32 *attributeWidths   = NULL ;
   Oid foreignTableId       = InvalidOid ;
   INT32 documentWidth      = 0;
   INT64 rowsCount          = 0 ;
   FLOAT64 foreignTableSize = 0 ;

   foreignTableId = RelationGetRelid ( relation ) ;
   rc = sdbRowsCount ( foreignTableId, &rowsCount ) ;
   if ( rc )
   {
      goto error ;
   }
   if ( rowsCount >= 0 )
   {
      attributeCount  = RelationGetNumberOfAttributes ( relation ) ;
      attributeWidths = (INT32*)palloc0((attributeCount + 1 )*sizeof(INT32)) ;
      if ( !attributeWidths )
      {
         ereport ( ERROR, ( errcode ( ERRCODE_FDW_OUT_OF_MEMORY ),
                            errmsg ( "Unable to allocate attr array memory" ),
                            errhint ( "Make sure the memory pool or ulimit is "
                                      "properly configured" ) ) ) ;
         goto error ;
      }
      documentWidth = get_relation_data_width ( foreignTableId,
                                                attributeWidths ) ;
      foreignTableSize = rowsCount * documentWidth ;
      pageCount = (BlockNumber) rint ( foreignTableSize / BLCKSZ) ;
   }
   else
   {
      ereport ( ERROR, ( errmsg ( "Unable to retreive rows count" ),
                         errhint ( "Unable to collect stats about foreign "
                                   "table" ) ) ) ;
   }
done :
   (*totalPageCount) = pageCount ;
   (*acquireSampleRowsFunc) = sdbAcquireSampleRows ;
   return TRUE ;
error :
   goto done ;
}

#if PG_VERSION_NUM>90300

static INT32 SdbIsForeignRelUpdatable ( Relation rel )
{
   return ( 1 << CMD_INSERT ) | ( 1 << CMD_UPDATE ) |
          ( 1 << CMD_DELETE ) ;
}

static void SdbAddForeignUpdateTargets ( Query *parsetree,
                                         RangeTblEntry *target_rte,
                                         Relation target_relation )
{
/*   Var         *var = NULL ;
   TargetEntry *tle = NULL ;
   var = makeVar ( parsetree->resultRelation,
                   SelfItemPointerAttributeNumber,
                   TIDOID,
                   -1,
                   InvalidOid,
                   0 ) ;
   tle = makeTargetEntry ( (Expr*) var,
                           list_length ( parsetree->targetList ) + 1,
                           pstrdup ( "rowid" ),
                           true ) ;
   parsetree->targetList = lappend ( parsetree->targetList, tle ) ;*/
}

static List *SdbPlanForeignModify ( PlannerInfo *root,
                                    ModifyTable *plan,
                                    Index resultRelation,
                                    INT32 subplan_index )
{
   List                *result       = NIL ;
   CmdType              operation ;
   RangeTblEntry       *rte          = NULL ;
   SdbExecState        *state        = NULL ;
   ForeignTable        *foreignTable = NULL ;
   BOOLEAN              is_table     = FALSE ;
   ListCell            *elem         = NULL ;
   List                *plan_values  = NULL ;
   SdbInputOptions      options ;
   StringInfo connectionString ;

   /* sanity check, update only applies for base table */
   rte          = planner_rt_fetch ( resultRelation, root ) ;
   foreignTable = GetForeignTable ( rte->relid ) ;
   operation    = plan->operation ;
   foreach ( elem, foreignTable->options )
   {
      DefElem *option = (DefElem *)lfirst(elem) ;
      if ( strcmp ( option->defname, "table" ) == 0 )
      {
         is_table = TRUE ;
         break ;
      }
   }
   if ( !is_table )
   {
      ereport ( ERROR, ( errcode ( ERRCODE_FDW_UNABLE_TO_CREATE_EXECUTION ),
                         errmsg ( "cannot modify foreign table "
                                  "\"%s\" which is based on a query",
                                  get_rel_name ( rte->relid ) ) ) ) ;
      goto error ;
   }

   /* pickup all columns that's try to modify */
done :
   return result ;
error :
   goto done ;
}
#endif

/* Transaction related */

static void SdbFdwXactCallback ( XactEvent event, void *arg )
{
   INT32 count             = 0 ;
   SdbConnectionPool *pool = sdbGetConnectionPool() ;
   for ( count = 0; count < pool->numConnections; ++count )
   {
      /* if there's no transaction started on this connection, let's continue */
      if ( 0 == pool->connList[count].transLevel )
         continue ;
      /* attempt to commit/abort the transaction, ignore return code */
      switch ( event )
      {
      case XACT_EVENT_COMMIT :
         sdbTransactionCommit ( pool->connList[count].hConnection ) ;
         pool->connList[count].transLevel = 0 ;
         break ;
      case XACT_EVENT_ABORT :
         sdbTransactionRollback ( pool->connList[count].hConnection ) ;
         pool->connList[count].transLevel = 0 ;
         break ;
      default :
         break ;
      }
   }
}

void _PG_init ()
{
   sdbInitConnectionPool () ;
   RegisterXactCallback ( SdbFdwXactCallback, NULL ) ;
}

void _PG_fini ()
{
   sdbUninitConnectionPool () ;
}
