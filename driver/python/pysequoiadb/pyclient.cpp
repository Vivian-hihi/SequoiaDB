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
#include <client.hpp>
#include <ossMem.hpp>

#include <Python.h>
#include "util.hpp"

using namespace sdbclient;

static PYOBJECT *create_client( PYOBJECT *self/*, PYOBJECT *args */)
{
   sdb *client = SDB_OSS_NEW sdb() ;
   if ( NULL == client )
   {
      return NULL ;
   }

   return MAKE_RETURN_OBJECT( client ) ;
}

static PYOBJECT *init_connect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   INT32 port       = 0 ;
   PYOBJECT *obj    = NULL ;
   sdb *client      = NULL ;
   const char *host = NULL ;
   
   if ( !PARSE_PYTHON_ARGS( args, "Osi", &obj, &host, &port ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   client->init_connect( host, port ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *release_client( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   void *tmp     = NULL ;
   sdb *client   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client )

   SDB_OSS_DEL client ;
   client = NULL ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *connect_by_host( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   INT32 port       = 0 ;
   PYOBJECT *obj    = NULL ;
   void *tmp        = NULL ;
   sdb *client      = NULL ;
   const char *host = NULL ;
   const char *user = NULL ;
   const char *psw  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Osiss", &obj, &host, &port, &user, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client )
   rc = client.connect( host, port, user, psw ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *connect_by_service( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
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

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   
   rc = client.connect( host, service, user, psw ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *connect_by_address( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   INT32 addr_size     = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   sdb *client         = NULL ;
   const char **addr   = NULL ;
   const char *user    = NULL ;
   const char *psw     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Osss", &obj, &addr, &addr_size,
                                                 &user, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.connect( addr, addr_size, user, psw ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *disconnect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   INT32 addr_size     = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   sdb *client         = NULL ;
   const char **addr   = NULL ;
   const char *user    = NULL ;
   const char *psw     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.disconnect() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *is_connected( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   sdb *client         = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.is_connected() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_user( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   void *tmp             = NULL ;
   sdb *client           = NULL ;
   const char *user_name = NULL ;
   const char *psw       = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Oss", &obj, &user_name, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.create_user( user_name, psw ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *remove_user( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   void *tmp             = NULL ;
   sdb *client           = NULL ;
   const char *user_name = NULL ;
   const char *psw       = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Oss", &obj, &user_name, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.remove_user( user_name, psw ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_snapshot( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   INT32 *snap_type         = NULL ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cursor_obj     = NULL ;
   PYOBJECT *bson_condition = NULL ;
   PYOBJECT *bson_selector  = NULL ;
   PYOBJECT *bson_order_by  = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbCursor *cursor        = NULL ;
   bson::BSONObj *condition = NULL ;
   bson::BSONObj *selector  = NULL ;
   bson::BSONObj *order_by  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOi|OOO", &obj, &cursor_obj, &snap_type,
                            &bson_condition, &bson_selector, &bson_order_by ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, tmp, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, tmp, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, tmp, order_by ) ;

   rc = client.get_snapshot( *cursor, snap_type,
                             condition, selector, order_by ) ;
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
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *bson_condition = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   bson::BSONObj *condition = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O|O", &obj, &bson_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;

   client->reset_snapshot( condition ) ;

done:
   DELETE_CPPOBJECT( condition ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_list( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   INT32 *list_type         = NULL ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cursor_obj     = NULL ;
   PYOBJECT *bson_condition = NULL ;
   PYOBJECT *bson_selector  = NULL ;
   PYOBJECT *bson_order_by  = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbCursor *cursor        = NULL ;
   bson::BSONObj *condition = NULL ;
   bson::BSONObj *selector  = NULL ;
   bson::BSONObj *order_by  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOi|OOO", &obj, &cursor_obj, &list_type,
                            &bson_condition, &bson_selector, &bson_order_by ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, tmp, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, tmp, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, tmp, order_by ) ;

   rc = client.get_list( *cursor, snap_type, condition, selector, order_by ) ;
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

static PYOBJECT *lock( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   sdb *client         = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.lock() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *unlock( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   sdb *client         = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.unlock() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_collection_space( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   INT32 *page_size         = NULL ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cs_obj         = NULL ;
   void *tmp                = NULL ;
   const char *cs_name      = NULL ;
   sdb *client              = NULL ;
   sdbCollectionSpace *cs   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsiO", &obj, &cs_name, &page_size,
                                                &cs_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cs_obj, tmp, sdbCollectionSpace, cs ) ;

   rc = client.get_collection_space( cs_name, page_size, *cs ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_collection_space( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   INT32 *page_size         = NULL ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cs_obj         = NULL ;
   void *tmp                = NULL ;
   const char *cs_name      = NULL ;
   sdb *client              = NULL ;
   sdbCollectionSpace *cs   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsiO", &obj, &cs_name, &page_size,
                            &cs_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cs_obj, tmp, sdbCollectionSpace, cs ) ;

   rc = client.create_collection_space( cs_name, page_size, *cs ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *drop_collection_space( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   void *tmp                = NULL ;
   const char *cs_name      = NULL ;
   sdb *client              = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &cs_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.drop_collection_space( cs_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *list_collection_spaces( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cursor_obj     = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbCursor *cursor        = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &cursor_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, tmp, sdbCursor, cursor ) ;

   rc = client.list_collection_space( cursor ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_replica_group_by_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *group_obj      = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbReplicaGroup *group   = NULL ;
   const char *group_name   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &group_name, &group_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( group_obj, tmp, sdbReplicaGroup, group ) ;

   rc = client.get_replica_group_by_name( group_name, group ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_replica_group_by_id( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   INT32 group_id           = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *group_obj      = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbReplicaGroup *group   = NULL ;
   const char *group_name   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OiO", &obj, &group_id, &group_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( group_obj, tmp, sdbReplicaGroup, group ) ;

   rc = client.get_replica_group_by_id( group_id, *group ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_replica_group( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *group_obj      = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbReplicaGroup *group   = NULL ;
   const char *group_name   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &group_name, &group_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( group_obj, tmp, sdbReplicaGroup, group ) ;

   rc = client.create_replica_group( group_name, *group ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *remove_replica_group( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;

   void *tmp                = NULL ;
   sdb *client              = NULL ;
   const char *group_name   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &group_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.remove_replica_group( group_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_replica_cata_group( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *bson_configure = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   const char *host         = NULL ;
   const char *service      = NULL ;
   const char *db_path      = NULL ;
   bson::BSONObj *configure = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsssO", &obj, &host, &service, &db_path,
                                                        &bson_configure ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_configure, tmp, configure ) ;

   rc = client.get_list( host, service, db_path, configure ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( configure ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *active_replica_group( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *group_obj      = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbReplicaGroup *group   = NULL ;
   const char *group_name   = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &group_name, &group_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( group_obj, tmp, sdbReplicaGroup, group ) ;

   rc = client.active_replica_group( group_name, *group ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *exec_update( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc        = 0 ;
   PYOBJECT *obj   = NULL ;
   void *tmp       = NULL ;
   sdb *client     = NULL ;
   const char *sql = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &sql ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.exec_update( sql ) ;
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
   void *tmp            = NULL ;
   sdb *client          = NULL ;
   sdbCursor *cursor    = NULL ;
   const char *sql      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &sql, &cursor_obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, tmp, sdbCursor, cursor ) ;

   rc = client.exec_sql( sql, *cursor ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *transaction_begin( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc        = 0 ;
   INT32 addr_size = 0 ;
   PYOBJECT *obj   = NULL ;
   void *tmp       = NULL ;
   sdb *client     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.transaction_begin() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *transaction_commit( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc        = 0 ;
   INT32 addr_size = 0 ;
   PYOBJECT *obj   = NULL ;
   void *tmp       = NULL ;
   sdb *client     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.transaction_commit() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *transaction_rollback( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc        = 0 ;
   INT32 addr_size = 0 ;
   PYOBJECT *obj   = NULL ;
   void *tmp       = NULL ;
   sdb *client     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.transaction_rollback() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *flush_configure( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *bson_option = NULL ;
   void *tmp             = NULL ;
   sdb *client           = NULL ;
   bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, tmp, option ) ;

   rc = client.flush_configure( option ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( option ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *backup_offline( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *bson_option = NULL ;
   void *tmp             = NULL ;
   sdb *client           = NULL ;
   bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, tmp, option ) ;

   rc = client.backup_offline( option ) ;
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
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cursor_obj     = NULL ;
   PYOBJECT *bson_option    = NULL ;
   PYOBJECT *bson_condition = NULL ;
   PYOBJECT *bson_selector  = NULL ;
   PYOBJECT *bson_order_by  = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbCursor *cursor        = NULL ;
   bson::BSONObj *option    = NULL ;
   bson::BSONObj *condition = NULL ;
   bson::BSONObj *selector  = NULL ;
   bson::BSONObj *order_by  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOO|OOO", &obj, &cursor_obj, &bson_option,
                            &bson_condition, &bson_selector, &bson_order_by) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, tmp, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, tmp, option ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, tmp, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, tmp, order_by ) ;

   rc = client.list_backup( option, condition, selector, order_by ) ;
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
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *bson_option = NULL ;
   void *tmp             = NULL ;
   sdb *client           = NULL ;
   bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, tmp, option ) ;

   rc = client.remove_backup( option ) ;
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
   INT32 rc                 = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cursor_obj     = NULL ;
   PYOBJECT *bson_condition = NULL ;
   PYOBJECT *bson_selector  = NULL ;
   PYOBJECT *bson_order_by  = NULL ;
   PYOBJECT *bson_hint      = NULL ;
   void *tmp                = NULL ;
   sdb *client              = NULL ;
   sdbCursor *cursor        = NULL ;
   bson::BSONObj *condition = NULL ;
   bson::BSONObj *selector  = NULL ;
   bson::BSONObj *order_by  = NULL ;
   bson::BSONObj *hint    = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO|OOOO", &obj, &cursor_obj, &bson_condition,
                                  &bson_selector, &bson_order_by, &bson_hint) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_obj, tmp, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, tmp, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, tmp, order_by ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, tmp, hint ) ;

   rc = client.list_tasks( *cursor, condition, selector, order_by, hint ) ;
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
   INT32 rc         = 0 ;
   PYOBJECT *obj    = NULL ;
   void *tmp        = NULL ;
   sdb *client      = NULL ;
   SINT64 task_id   = 0 ;
   SINT32 num       = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "Oii", &obj, &task_id, &num ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.wait_task( task_id, num ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *cancel_task( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   PYOBJECT *obj    = NULL ;
   void *tmp        = NULL ;
   sdb *client      = NULL ;
   SINT64 task_id   = 0 ;
   BOOLEAN is_async = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "Oii", &obj, &task_id, &is_async ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.cancel_task( task_id, is_async ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *set_session_attri( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *bson_option = NULL ;
   void *tmp             = NULL ;
   sdb *client           = NULL ;
   bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O|O", &obj, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, tmp, option ) ;

   rc = client.set_session_attri( option ) ;
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
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   void *tmp             = NULL ;
   sdb *client           = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;

   rc = client.close_all_cursors() ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *is_valid( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   INT32 result          = 0 ;
   PYOBJECT *obj         = NULL ;
   void *tmp             = NULL ;
   sdb *client           = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Oi", &obj, &result ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdb, client ) ;
   rc = client.is_valid( result ) ;
   if ( rc )
   {
      goto done ;
   }
done:
   return MAKE_RETURN_INT_INT( rc, result ) ;
}