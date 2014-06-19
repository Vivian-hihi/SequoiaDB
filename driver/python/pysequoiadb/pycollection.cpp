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
#include "pycollection.hpp"
#include "client.hpp"

using namespace sdbclient ;

static PYOBJECT *create_cl( PYOBJECT *self, PYOBJECT *args ) 
{
   sdbCollection *cl = NULL;
   NEW_CPPOBJECT( cl, sdbCollection ) ;
   if ( NULL == cl )
   {
      return NULL ;
   }

   return MAKE_PYOBJECT( cl ) ;
}

static PYOBJECT *release_cl( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj     = NULL ;
   sdbCollection *cl = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   DELETE_CPPOBJECT( cl ) ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_count( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   SINT64 count                   = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   const bson::BSONObj *condition = NULL ;
   sdbCollection *cl              = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O|O", &obj, &bson_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;

   rc = cl->getCount( count, *condition ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT_INT( rc, count ) ;
}

static PYOBJECT *split_by_condition( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                           = 0 ;
   PYOBJECT *obj                      = NULL ;
   PYOBJECT *bson_condition           = NULL ;
   PYOBJECT *bson_end_condition       = NULL ;
   const char *src_name               = NULL ;
   const char *dst_name               = NULL ;
   sdbCollection *cl                  = NULL ;
   const bson::BSONObj *condition     = NULL ;
   const bson::BSONObj *end_condition = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OssO|O", &obj, &src_name, &dst_name,
                          &bson_condition, &bson_end_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_end_condition, end_condition ) ;

   rc = cl->split( src_name, dst_name, *condition, *end_condition ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( end_condition ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *split_by_precent( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   const char *dst_name = NULL ;
   const char *src_name = NULL ;
   sdbCollection *cl    = NULL ;
   FLOAT64 precent      = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "Ossd", &obj, &src_name, &dst_name,
                                &precent ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;

   rc = cl->split( src_name, dst_name, precent ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *split_async_by_condition( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                           = 0 ;
   PYOBJECT *obj                      = NULL ;
   PYOBJECT *bson_condition           = NULL ;
   PYOBJECT *bson_end_condition       = NULL ;
   const char *src_name               = NULL ;
   const char *dst_name               = NULL ;
   sdbCollection *cl                  = NULL ;
   const bson::BSONObj *condition     = NULL ;
   const bson::BSONObj *end_condition = NULL ;
   SINT64 task_id            = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "OssO|O", &obj, &src_name, &dst_name,
                            &bson_condition, &bson_end_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_end_condition, end_condition ) ;

   rc = cl->splitAsync( task_id, src_name, dst_name,
                          *condition, *end_condition ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( end_condition ) ;
   return MAKE_RETURN_INT_LONG( rc, task_id ) ;
}

static PYOBJECT *splite_async_by_precent( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   const char *src_name = NULL ;
   const char *dst_name = NULL ;
   sdbCollection *cl    = NULL ;
   FLOAT64 precent      = 0 ;
   SINT64 task_id       = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "Ossd", &obj, &src_name, &dst_name,
                                                &precent ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;

   rc = cl->splitAsync( src_name, dst_name, precent, task_id ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT_LONG( rc, task_id ) ;
}

static PYOBJECT *bulk_insert( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   SINT32 flags          = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *list_object = NULL ;
   sdbCollection *cl     = NULL ;

   std::vector< bson::BSONObj > vec_bson ;

   if ( !PARSE_PYTHON_ARGS( args, "OiO", &obj, &flags, &list_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   MAKE_PYLIST_TO_VECTOR( list_object, vec_bson ) ;
   rc = cl->bulkInsert( flags, vec_bson ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *insert( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                    = 0 ;
   PYOBJECT *obj               = NULL ;
   PYOBJECT *bson_object       = NULL ;
   PYOBJECT *oid_object        = NULL ;
   sdbCollection *cl           = NULL ;
   const bson::BSONObj *object = NULL ;
   bson::OID *id = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO|O", &obj, &bson_object, oid_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   if ( Py_None != oid_object )
   {
      CAST_PYOBJECT_TO_COBJECT( oid_object, bson::OID, id ) ;
   }
   CAST_PYBSON_TO_CPPBSON( bson_object, object ) ;
   rc = cl->insert( *object, id ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( object ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *update( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *bson_rule            = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   PYOBJECT *bson_hint            = NULL ;
   sdbCollection *cl              = NULL ;
   const bson::BSONObj *rule      = NULL ;
   const bson::BSONObj *condition = NULL ;
   const bson::BSONObj *hint      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO|OO", &obj, &bson_rule,
                                &bson_condition, &bson_hint ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_rule, rule ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, hint ) ;

   rc = cl->update( *rule, *condition, *hint ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( rule ) ;
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( hint ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *upsert( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                       = 0 ;
   PYOBJECT *obj                  = NULL ;
   PYOBJECT *bson_rule            = NULL ;
   PYOBJECT *bson_condition       = NULL ;
   PYOBJECT *bson_hint            = NULL ;
   sdbCollection *cl              = NULL ;
   const bson::BSONObj *rule      = NULL ;
   const bson::BSONObj *condition = NULL ;
   const bson::BSONObj *hint      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO|OO", &obj, &bson_rule,
                                &bson_condition, &bson_hint ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_rule, rule ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, hint ) ;

   rc = cl->upsert( *rule, *condition, *hint ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( rule ) ;
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( hint ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *del( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *bson_condition    = NULL ;
   PYOBJECT *bson_hint        = NULL ;
   sdbCollection *cl         = NULL ;
   const bson::BSONObj *condition    = NULL ;
   const bson::BSONObj *hint        = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O|OO", &obj, &bson_condition, &bson_hint ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, hint ) ;

   rc = cl->del( *condition, *hint ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( hint ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *query( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   INT64 num_to_skip         = 0 ;
   INT64 num_to_return        = -1 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cursor_object     = NULL ;
   PYOBJECT *bson_condition    = NULL ;
   PYOBJECT *bson_selector     = NULL ;
   PYOBJECT *bson_order_by     = NULL ;
   PYOBJECT *bson_hint        = NULL ;
   sdbCollection *cl         = NULL ;
   sdbCursor *cursor         = NULL ;
   const bson::BSONObj *condition    = NULL ;
   const bson::BSONObj *selector     = NULL ;
   const bson::BSONObj *order_by     = NULL ;
   const bson::BSONObj *hint        = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO|OOOOii", &obj, &cursor_object,
                     &bson_condition,  &bson_selector, &bson_order_by,
                     &bson_hint, &num_to_skip, &num_to_return ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, order_by ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, hint ) ;

   rc = cl->query( *cursor, *condition, *selector, *order_by, *hint,
                              num_to_skip, num_to_return ) ;
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

static PYOBJECT *create_index( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   BOOLEAN is_unique      = 0 ;
   BOOLEAN is_enforced     = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *bson_index_def = NULL ;
   sdbCollection *cl      = NULL ;
   const bson::BSONObj *index_def = NULL ;
   const char *name       = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOsii", &obj, &bson_index_def, &name,
                                 &is_unique, &is_enforced ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_index_def, index_def ) ;


   rc = cl->createIndex( *index_def, name, is_unique, is_enforced ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( index_def ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_index( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *cursor_object = NULL ;
   sdbCollection *cl      = NULL ;
   sdbCursor *cursor      = NULL ;
   const char *index_name  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOs", &obj, &cursor_object, &index_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, sdbCursor, cursor ) ;

   rc = cl->getIndexes( *cursor, index_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *drop_index( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj         = NULL ;
   sdbCollection *cl      = NULL ;
   const char *index_name  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &index_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;

   rc = cl->dropIndex( index_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_collection_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   PYOBJECT *obj      = NULL ;
   sdbCollection *cl   = NULL ;
   const char *cl_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;

   cl_name = cl->getCollectionName() ;

done:
   return MAKE_RETURN_INT_PYSTRING( rc, cl_name ) ;
}

static PYOBJECT *get_collection_space_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   PYOBJECT *obj      = NULL ;
   sdbCollection *cl   = NULL ;
   const char *cs_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;

   cs_name = cl->getCSName() ;

done:
   return MAKE_RETURN_INT_PYSTRING( rc, cs_name ) ;
}

static PYOBJECT *get_full_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc           = 0 ;
   PYOBJECT *obj       = NULL ;
   sdbCollection *cl    = NULL ;
   const char *full_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;

   full_name = cl->getFullName() ;

done:
   return MAKE_RETURN_INT_PYSTRING( rc, full_name ) ;
}

static PYOBJECT *aggregate( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *list_object   = NULL ;
   PYOBJECT *cursor_object = NULL ;
   sdbCollection *cl      = NULL ;
   sdbCursor *cursor      = NULL ;

   std::vector< bson::BSONObj > vec_bson ;

   if ( !PARSE_PYTHON_ARGS( args, "OOO", &obj, &cursor_object, &list_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, sdbCursor, cursor ) ;
   MAKE_PYLIST_TO_VECTOR( list_object, vec_bson ) ;
   rc = cl->aggregate( *cursor, vec_bson ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_query_meta( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   INT64 num_to_skip         = 0 ;
   INT64 num_to_return        = -1 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *cursor_object     = NULL ;
   PYOBJECT *bson_condition    = NULL ;
   PYOBJECT *bson_order_by     = NULL ;
   PYOBJECT *bson_hint        = NULL ;
   sdbCollection *cl         = NULL ;
   sdbCursor *cursor         = NULL ;
   const bson::BSONObj *condition    = NULL ;
   const bson::BSONObj *order_by     = NULL ;
   const bson::BSONObj *hint        = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO|OOOii", &obj, &cursor_object,
                     &bson_condition, &bson_order_by, &bson_hint,
                     &num_to_skip, &num_to_return ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, order_by ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, hint ) ;

   rc = cl->getQueryMeta( *cursor, *condition, *order_by, *hint,
                     num_to_skip, num_to_return ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( order_by ) ;
   DELETE_CPPOBJECT( hint ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *attach_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *bson_option    = NULL ;
   const char *sub_full_name = NULL ;
   sdbCollection *cl       = NULL ;
   const bson::BSONObj *option    = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &sub_full_name, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }
   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, option ) ;

   rc = cl->attachCollection( sub_full_name, *option ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( option ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *detach_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj          = NULL ;
   const char *sub_full_name = NULL ;
   sdbCollection *cl       = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &sub_full_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }
   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCollection, cl ) ;

   rc = cl->detachCollection( sub_full_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

/* List of functions defined in the module */
static PyMethodDef sdbcollection_methods[] = {
   {"create_cl",                 create_cl,                 METH_VARARGS},
   {"release_cl",                release_cl,                METH_VARARGS},
   {"get_count",                 get_count,                 METH_VARARGS},
   {"split_by_condition",        split_by_condition,        METH_VARARGS},
   {"split_by_precent",          split_by_precent,          METH_VARARGS},
   {"split_async_by_condition",  split_async_by_condition,  METH_VARARGS},
   {"splite_async_by_precent",   splite_async_by_precent,   METH_VARARGS},
   {"bulk_insert",               bulk_insert,               METH_VARARGS},
   {"insert",                    insert,                    METH_VARARGS},
   {"update",                    update,                    METH_VARARGS},
   {"upsert",                    upsert,                    METH_VARARGS},
   {"del",                       del,                       METH_VARARGS},
   {"query",                     query,                     METH_VARARGS},
   {"create_index",              create_index,              METH_VARARGS},
   {"get_index",                 get_index,                 METH_VARARGS},
   {"drop_index",                drop_index,                METH_VARARGS},
   {"get_collection_name",       get_collection_name,       METH_VARARGS},
   {"get_collection_space_name", get_collection_space_name, METH_VARARGS},
   {"get_full_name",             get_full_name,             METH_VARARGS},
   {"aggregate",                 aggregate,                 METH_VARARGS},
   {"get_query_meta",            get_query_meta,            METH_VARARGS},
   {"attach_collection",         attach_collection,         METH_VARARGS},
   {"detach_collection",         detach_collection,         METH_VARARGS},
   {NULL, NULL}
};

CREATE_MODULE( sdbcl, sdbcollection_methods )
