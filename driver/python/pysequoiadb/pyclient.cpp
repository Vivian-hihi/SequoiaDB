/*******************************************************************************

   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*******************************************************************************/
#include "util.hpp"
#include "ossVer.h"
#include "client.hpp"

using namespace sdbclient;

static PYOBJECT *create_client( PYOBJECT *self, PYOBJECT *args )
{
   sdb *client = NULL;
   NEW_CPPOBJECT( client, sdb ) ;
   if ( NULL == client )
   {
      return NULL ;
   }

   return MAKE_PYOBJECT( client ) ;
}

static PYOBJECT *release_client( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   sdb *client   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   DELETE_CPPOBJECT( client ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *connect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   sdb *client         = NULL ;
   const char *host    = NULL ;
   const char *service = NULL ;
   const char *user    = NULL ;
   const char *psw     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Ossss", &obj, &host, &service, &user, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   
   rc = client->connect( host, service, user, psw ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *disconnect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   sdb *client   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   client->disconnect() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_user( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   sdb *client           = NULL ;
   const char *user_name = NULL ;
   const char *psw       = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Oss", &obj, &user_name, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->createUsr( user_name, psw ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *remove_user( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   sdb *client           = NULL ;
   const char *user_name = NULL ;
   const char *psw       = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Oss", &obj, &user_name, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->removeUsr( user_name, psw ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_snapshot( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   INT32 snap_type                = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *cursor_obj           = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   PYOBJECT *bson_selector        = NULL ;
   PYOBJECT *bson_order_by        = NULL ;
   sdb *client                    = NULL ;
   sdbCursor *cursor              = NULL ;
   const bson::BSONObj *condition = NULL ;
   const bson::BSONObj *selector  = NULL ;
   const bson::BSONObj *order_by  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOiOOO", &obj, &cursor_obj, &snap_type,
                            &bson_condition, &bson_selector, &bson_order_by ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, order_by ) ;

   rc = client->getSnapshot( *cursor, snap_type, *condition,
                                      *selector, *order_by ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( selector ) ;
   DELETE_CPPOBJECT( order_by ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *reset_snapshot( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   sdb *client                    = NULL ;
   const bson::BSONObj *condition = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;

   client->resetSnapshot( *condition ) ;

done:
   DELETE_CPPOBJECT( condition ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_list( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   INT32 list_type                = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *cursor_obj           = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   PYOBJECT *bson_selector        = NULL ;
   PYOBJECT *bson_order_by        = NULL ;
   sdb *client                    = NULL ;
   sdbCursor *cursor              = NULL ;
   const bson::BSONObj *condition = NULL ;
   const bson::BSONObj *selector  = NULL ;
   const bson::BSONObj *order_by  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOiOOO", &obj, &cursor_obj, &list_type,
                            &bson_condition, &bson_selector, &bson_order_by ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, order_by ) ;

   rc = client->getList( *cursor, list_type, *condition, *selector, *order_by ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( selector ) ;
   DELETE_CPPOBJECT( order_by ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_collection_space( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *cs_obj       = NULL ;
   const char *cs_name    = NULL ;
   sdb *client            = NULL ;
   sdbCollectionSpace *cs = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &cs_name, &cs_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cs_obj, sdbCollectionSpace, cs ) ;

   rc = client->getCollectionSpace( cs_name, *cs ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *cl_obj       = NULL ;
   const char *cl_name    = NULL ;
   sdb *client            = NULL ;
   sdbCollection *cl      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &cl_name, &cl_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cl_obj, sdbCollection, cl ) ;

   rc = client->getCollection( cl_name, *cl ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_collection_space( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   INT32 page_size        = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *cs_obj       = NULL ;
   const char *cs_name    = NULL ;
   sdb *client            = NULL ;
   sdbCollectionSpace *cs = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsiO", &obj, &cs_name, &page_size,
                                                &cs_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cs_obj, sdbCollectionSpace, cs ) ;

   rc = client->createCollectionSpace( cs_name, page_size, *cs ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *drop_collection_space( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   const char *cs_name = NULL ;
   sdb *client         = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &cs_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->dropCollectionSpace( cs_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *list_collection_spaces( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   PYOBJECT *cursor_obj = NULL ;
   sdb *client          = NULL ;
   sdbCursor *cursor    = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &cursor_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, sdbCursor, cursor ) ;

   rc = client->listCollectionSpaces( *cursor ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *list_collections( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   PYOBJECT *cursor_obj = NULL ;
   sdb *client          = NULL ;
   sdbCursor *cursor    = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &cursor_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, sdbCursor, cursor ) ;

   rc = client->listCollections( *cursor ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *list_replica_groups( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   PYOBJECT *cursor_obj = NULL ;
   sdb *client          = NULL ;
   sdbCursor *cursor    = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &cursor_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, sdbCursor, cursor ) ;

   rc = client->listReplicaGroups( *cursor ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_replica_group_by_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *group_obj    = NULL ;
   sdb *client            = NULL ;
   sdbReplicaGroup *group = NULL ;
   const char *group_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &group_name, &group_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( group_obj, sdbReplicaGroup, group ) ;

   rc = client->getReplicaGroup( group_name, *group ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_replica_group_by_id( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   INT32 group_id         = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *group_obj    = NULL ;
   sdb *client            = NULL ;
   sdbReplicaGroup *group = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OiO", &obj, &group_id, &group_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( group_obj, sdbReplicaGroup, group ) ;

   rc = client->getReplicaGroup( group_id, *group ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_replica_group( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *group_obj    = NULL ;
   sdb *client            = NULL ;
   sdbReplicaGroup *group = NULL ;
   const char *group_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &group_name, &group_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( group_obj, sdbReplicaGroup, group ) ;
   rc = client->createReplicaGroup( group_name, *group ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *remove_replica_group( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   sdb *client            = NULL ;
   const char *group_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &group_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->removeReplicaGroup( group_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_replica_cata_group( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *bson_configure       = NULL ;
   sdb *client                    = NULL ;
   const char *host               = NULL ;
   const char *service            = NULL ;
   const char *db_path            = NULL ;
   const bson::BSONObj *configure = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsssO", &obj, &host, &service, &db_path,
                                           &bson_configure ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_configure, configure ) ;

   rc = client->createReplicaCataGroup( host, service, db_path, *configure ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( configure ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *exec_update( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc        = 0 ;
   PYOBJECT *obj   = NULL ;
   sdb *client     = NULL ;
   const char *sql = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &sql ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->execUpdate( sql ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *exec_sql( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   PYOBJECT *cursor_obj = NULL ;
   sdb *client          = NULL ;
   sdbCursor *cursor    = NULL ;
   const char *sql      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &sql, &cursor_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, sdbCursor, cursor ) ;

   rc = client->exec( sql, *cursor ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *transaction_begin( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   sdb *client   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->transactionBegin() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *transaction_commit( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   sdb *client   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->transactionCommit() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *transaction_rollback( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   sdb *client   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->transactionRollback() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *flush_configure( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                    = 0 ;
   PYOBJECT *obj               = NULL ;
   PYOBJECT *bson_option       = NULL ;
   sdb *client                 = NULL ;
   const bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, option ) ;

   rc = client->flushConfigure( *option ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( option ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_JS_procedure( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   sdb *client           = NULL ;
   const char *str_code  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &str_code ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->crtJSProcedure( str_code ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *remove_procedure( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   sdb *client           = NULL ;
   const char *spname  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &spname ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->rmProcedure( spname ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *list_procedures( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *cursor_object        = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   sdb *client                    = NULL ;
   sdbCursor *cursor              = NULL ;
   const bson::BSONObj *condition = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOO", &obj,
                            &cursor_object, &bson_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;

   rc = client->listProcedures( *cursor, *condition ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( condition ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *eval_JS( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                          = 0 ;
   SDB_SPD_RES_TYPE sdb_spd_res_type = SDB_SPD_RES_TYPE_VOID ;
   PYOBJECT *obj                     = NULL ;
   PYOBJECT *cursor_object           = NULL ;
   sdb *client                       = NULL ;
   sdbCursor *cursor                 = NULL ;
   const char *code                  = NULL ;
   const bson::BSONObj errmsg;

   if ( !PARSE_PYTHON_ARGS( args, "OOs", &obj, &cursor_object, &code ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, sdbCursor, cursor ) ;

   rc = client->evalJS( *cursor, code, &sdb_spd_res_type, errmsg ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *backup_offline( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                    = 0 ;
   PYOBJECT *obj               = NULL ;
   PYOBJECT *bson_option       = NULL ;
   sdb *client                 = NULL ;
   const bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, option ) ;

   rc = client->backupOffline( *option ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( option ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *list_backup( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *cursor_obj           = NULL ;
   PYOBJECT *bson_option          = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   PYOBJECT *bson_selector        = NULL ;
   PYOBJECT *bson_order_by        = NULL ;
   sdb *client                    = NULL ;
   sdbCursor *cursor              = NULL ;
   const bson::BSONObj *option    = NULL ;
   const bson::BSONObj *condition = NULL ;
   const bson::BSONObj *selector  = NULL ;
   const bson::BSONObj *order_by  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOOOOO", &obj, &cursor_obj, &bson_option,
                            &bson_condition, &bson_selector, &bson_order_by) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, option ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, order_by ) ;

   rc = client->listBackup( *cursor, *option, *condition, *selector, *order_by ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( option ) ;
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( selector ) ;
   DELETE_CPPOBJECT( order_by ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *remove_backup( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                    = 0 ;
   PYOBJECT *obj               = NULL ;
   PYOBJECT *bson_option       = NULL ;
   sdb *client                 = NULL ;
   const bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, option ) ;

   rc = client->removeBackup( *option ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( option ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *list_tasks( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *cursor_obj           = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   PYOBJECT *bson_selector        = NULL ;
   PYOBJECT *bson_order_by        = NULL ;
   PYOBJECT *bson_hint            = NULL ;
   sdb *client                    = NULL ;
   sdbCursor *cursor              = NULL ;
   const bson::BSONObj *condition = NULL ;
   const bson::BSONObj *selector  = NULL ;
   const bson::BSONObj *order_by  = NULL ;
   const bson::BSONObj *hint      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOOOOO", &obj, &cursor_obj, &bson_condition,
                            &bson_selector, &bson_order_by, &bson_hint) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, order_by ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, hint ) ;

   rc = client->listTasks( *cursor, *condition, *selector, *order_by, *hint ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( selector ) ;
   DELETE_CPPOBJECT( order_by ) ;
   DELETE_CPPOBJECT( hint ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *wait_task( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *task_ids_obj = NULL ;
   sdb *client            = NULL ;
   SINT64 *task_ids       = NULL ;
   SINT32 num             = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "OOi", &obj, &task_ids_obj, &num ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   task_ids = new SINT64[num] ;
   MAKE_PYLIST_TO_BUFFER( task_ids_obj, task_ids) ;
   rc = client->waitTasks( task_ids, num ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   if ( NULL != task_ids )
   {
      delete [] task_ids ;
   }
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *cancel_task( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   PYOBJECT *obj    = NULL ;
   sdb *client      = NULL ;
   SINT64 task_id   = 0 ;
   BOOLEAN is_async = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "OLi", &obj, &task_id, &is_async ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->cancelTask( task_id, is_async ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *set_session_attri( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                    = 0 ;
   PYOBJECT *obj               = NULL ;
   PYOBJECT *bson_option       = NULL ;
   sdb *client                 = NULL ;
   const bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, option ) ;

   rc = client->setSessionAttr( *option ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( option ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *close_all_cursors( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   sdb *client   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;

   rc = client->closeAllCursors() ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *is_valid( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc        = 0 ;
   INT32 result    = FALSE ;
   PYOBJECT *obj   = NULL ;
   sdb *client     = NULL ;
   BOOLEAN isvalid = FALSE;
  
   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   rc = client->isValid( &isvalid ) ;
   if ( rc )
   {
      goto done ;
   }
   
   if ( isvalid )
   {
      result = 1 ;
   }
   else
   {
      result = 0 ;
   }
done:
   return MAKE_RETURN_INT_INT( rc, result ) ;
}

static PYOBJECT *get_version( PYOBJECT *self, PYOBJECT *args )
{
   int version = 0 ;
   int sub_version = 0 ;
   int release = 0 ;
   const char *build = NULL ;

   ossGetVersion( &version, &sub_version, &release, &build ) ;

   return MAKE_RETURN_INT_INT_INT_STRING( version, sub_version, release, build ) ;
}

/* List of functions defined in the module */
static PyMethodDef client_methods[] = {
   {"create_client",             create_client,             METH_VARARGS},
   {"release_client",            release_client,            METH_VARARGS},
   {"connect",                   connect,                   METH_VARARGS},
   {"disconnect",                disconnect,                METH_VARARGS},
   {"create_user",               create_user,               METH_VARARGS},
   {"remove_user",               remove_user,               METH_VARARGS},
   {"get_snapshot",              get_snapshot,              METH_VARARGS},
   {"reset_snapshot",            reset_snapshot,            METH_VARARGS},
   {"get_list",                  get_list,                  METH_VARARGS},
   {"get_collection_space",      get_collection_space,      METH_VARARGS},
   {"get_collection",            get_collection,            METH_VARARGS},
   {"create_collection_space",   create_collection_space,   METH_VARARGS},
   {"drop_collection_space",     drop_collection_space,     METH_VARARGS},
   {"list_collection_spaces",    list_collection_spaces,    METH_VARARGS},
   {"list_collections",          list_collections,          METH_VARARGS},
   {"list_replica_groups",       list_replica_groups,       METH_VARARGS},
   {"get_replica_group_by_name", get_replica_group_by_name, METH_VARARGS},
   {"get_replica_group_by_id",   get_replica_group_by_id,   METH_VARARGS},
   {"create_replica_group",      create_replica_group,      METH_VARARGS},
   {"remove_replica_group",      remove_replica_group,      METH_VARARGS},
   {"create_replica_cata_group", create_replica_cata_group, METH_VARARGS},
   /*{"active_replica_group",     active_replica_group,       METH_VARARGS},*/
   {"exec_update",               exec_update,               METH_VARARGS},
   {"exec_sql",                  exec_sql,                  METH_VARARGS},
   {"transaction_begin",         transaction_begin,         METH_VARARGS},
   {"transaction_commit",        transaction_commit,        METH_VARARGS},
   {"transaction_rollback",      transaction_rollback,      METH_VARARGS},
   {"flush_configure",           flush_configure,           METH_VARARGS},
   {"create_JS_procedure",       create_JS_procedure,       METH_VARARGS},
   {"remove_procedure",          remove_procedure,          METH_VARARGS},
   {"list_procedures",           list_procedures,           METH_VARARGS},
   {"eval_JS",                   eval_JS,                   METH_VARARGS},
   {"backup_offline",            backup_offline,            METH_VARARGS},
   {"list_backup",               list_backup,               METH_VARARGS},
   {"remove_backup",             remove_backup,             METH_VARARGS},
   {"list_tasks",                list_tasks,                METH_VARARGS},
   {"wait_task",                 wait_task,                 METH_VARARGS},
   {"cancel_task",               cancel_task,               METH_VARARGS},
   {"set_session_attri",         set_session_attri,         METH_VARARGS},
   {"close_all_cursors",         close_all_cursors,         METH_VARARGS},
   {"is_valid",                  is_valid,                  METH_VARARGS},
   {"get_version",               get_version,               METH_VARARGS},
   {NULL, NULL}
};

CREATE_MODULE( sdbclient, client_methods )
