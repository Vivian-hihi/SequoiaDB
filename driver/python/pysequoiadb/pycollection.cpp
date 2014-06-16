#include <Python.h>

#include "util.hpp"
#include "pycollection.hpp"

static PYOBJECT *create_cl( PYOBJECT *self ) 
{
   sdbCollection *cl = SDB_OSS_NEW sdbCollection() ;
   if ( NULL == cl )
   {
      return NULL ;
   }

   return MAKE_RETURN_OBJECT( cl ) ;
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

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   rc = cl->release_cs() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_count( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   SINT64 count             = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *bson_condition = NULL ;
   void *tmp                = NULL ;
   bson::BSONObj *condition = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OO", &obj, &bson_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;

   rc = cs->get_count( count, condition ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT_INT( rc, count ) ;
}

static PYOBJECT *split_by_condition( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                     = 0 ;
   PYOBJECT *obj                = NULL ;
   PYOBJECT *cl_object          = NULL ;
   PYOBJECT *bson_condition     = NULL ;
   PYOBJECT *bson_end_condition = NULL ;
   void *tmp                    = NULL ;
   const char *src_name         = NULL ;
   const char *dst_name         = NULL ;
   sdbCollection *cl            = NULL ;
   bson::BSONObj *condition     = NULL ;
   bson::BSONObj *end_condition = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OssO", &obj, &src_name, &dst_name,
                                  &bson_condition, &bson_end_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_end_condition, tmp, end_condition ) ;

   rc = cl->split_by_condition( src_name, dst_name, condition, end_condition ) ;
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
   PYOBJECT *cl_object  = NULL ;
   void *tmp            = NULL ;
   const char *src_name = NULL ;
   const char *dst_name = NULL ;
   sdbCollection *cl    = NULL ;
   FLOAT64 precent      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Ossd", &obj, &src_name, &dst_name,
                                          &precent ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;

   rc = cl->split_by_precent( src_name, dst_name, precent ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *split_async_by_condition( PYOBJECT *self, PYOBJECT *args ) ;
{
   INT32 rc                     = 0 ;
   PYOBJECT *obj                = NULL ;
   PYOBJECT *cl_object          = NULL ;
   PYOBJECT *bson_condition     = NULL ;
   PYOBJECT *bson_end_condition = NULL ;
   void *tmp                    = NULL ;
   const char *src_name         = NULL ;
   const char *dst_name         = NULL ;
   sdbCollection *cl            = NULL ;
   bson::BSONObj *condition     = NULL ;
   bson::BSONObj *end_condition = NULL ;
   SINT64 task_id               = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "OssO", &obj, &src_name, &dst_name,
                                  &bson_condition, &bson_end_condition ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_end_condition, tmp, end_condition ) ;

   rc = cl->split_async_by_condition( task_id, src_name, dst_name,
                                      condition, end_condition ) ;
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
   PYOBJECT *cl_object  = NULL ;
   void *tmp            = NULL ;
   const char *src_name = NULL ;
   const char *dst_name = NULL ;
   sdbCollection *cl    = NULL ;
   FLOAT64 precent      = NULL ;
   SINT64 task_id       = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "Ossd", &obj, &src_name, &dst_name,
                                                           &precent ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;

   rc = cl->splite_async_by_precent( src_name, dst_name, precent, task_id ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT_LONG( rc, task_id ) ;
}

static PYOBJECT *bulk_insert( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   SINT32 flags         = 0 ;
   INT32  list_size     = 0 ;
   PYOBJECT *obj        = NULL ;
   PYOBJECT *cl_object  = NULL ;
   PYOBJECT *list_object= NULL ;
   void *tmp            = NULL ;
   sdbCollection *cl    = NULL ;

   std::vector< bson::BSONObj > vec_bson ;

   if ( !PARSE_PYTHON_ARGS( args, "OiO", &obj, &flags, &list_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   MAKE_PYLIST_TO_VECTOR( list_object, list_size, tmp, vec_bson ) ;
   rc = cl->bulk_insert( flags, vec_bson ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *insert( PYOBJECT *self, PYOBJECT *args )
{

}

static PYOBJECT *update( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                     = 0 ;
   PYOBJECT *obj                = NULL ;
   PYOBJECT *bson_rule          = NULL ;
   PYOBJECT *bson_condition     = NULL ;
   PYOBJECT *bson_hint          = NULL ;
   void *tmp                    = NULL ;
   sdbCollection *cl            = NULL ;
   bson::BSONObj *rule          = NULL ;
   bson::BSONObj *condition     = NULL ;
   bson::BSONObj *hint          = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOOO", &obj, &bson_rule,
                                          &bson_condition, &bson_hint ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_rule, tmp, rule ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, tmp, hint ) ;

   rc = cl->update( rule, condition, hint ) ;
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
   INT32 rc                     = 0 ;
   PYOBJECT *obj                = NULL ;
   PYOBJECT *bson_rule          = NULL ;
   PYOBJECT *bson_condition     = NULL ;
   PYOBJECT *bson_hint          = NULL ;
   void *tmp                    = NULL ;
   sdbCollection *cl            = NULL ;
   bson::BSONObj *rule          = NULL ;
   bson::BSONObj *condition     = NULL ;
   bson::BSONObj *hint          = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOOO", &obj, &bson_rule,
                                          &bson_condition, &bson_hint ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_rule, tmp, rule ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, tmp, hint ) ;

   rc = cl->upsert( rule, condition, hint ) ;
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
   INT32 rc                     = 0 ;
   PYOBJECT *obj                = NULL ;
   PYOBJECT *bson_condition     = NULL ;
   PYOBJECT *bson_hint          = NULL ;
   void *tmp                    = NULL ;
   sdbCollection *cl            = NULL ;
   bson::BSONObj *condition     = NULL ;
   bson::BSONObj *hint          = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOO", &obj, &bson_condition, &bson_hint ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, tmp, hint ) ;

   rc = cl->del( rule, condition, hint ) ;
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
   INT32 rc                     = 0 ;
   INT64 num_to_skip            = 0 ;
   INT64 num_to_return          = -1 ;
   PYOBJECT *obj                = NULL ;
   PYOBJECT *ret_object         = NULL ;
   PYOBJECT *cursor_object      = NULL ;
   PYOBJECT *bson_condition     = NULL ;
   PYOBJECT *bson_selector      = NULL ;
   PYOBJECT *bson_order_by      = NULL ;
   PYOBJECT *bson_hint          = NULL ;
   void *tmp                    = NULL ;
   sdbCollection *cl            = NULL ;
   sdbCursor *cursor            = NULL ;
   bson::BSONObj *condition     = NULL ;
   bson::BSONObj *selector      = NULL ;
   bson::BSONObj *order_by      = NULL ;
   bson::BSONObj *hint          = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOOOOOii", &obj, &cursor_object,
                            &bson_condition,  &bson_selector, &bson_order_by,
                            &bson_hint, &num_to_skip, &num_to_return ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, tmp, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_selector, tmp, selector ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, tmp, order_by ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, tmp, hint ) ;

   rc = cl->query( *cursor, condition, selector, order_by, hint,
                                       num_to_skip, &num_to_return ) ;
   if ( rc )
   {
      goto done ;
   }

   MAKE_PYTHON_VOID_OBJECT( cursor, ret_object ) ;

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( selector ) ;
   DELETE_CPPOBJECT( order_by ) ;
   DELETE_CPPOBJECT( hint ) ;
   return MAKE_RETURN_INT_OBJECT( rc ) ;
}

static PYOBJECT *create_index( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                 = 0 ;
   BOOLEAN is_unique        = 0 ;
   BOOLEAN is_enforced      = 0 ;
   PYOBJECT *obj            = NULL ;
   PYOBJECT *bson_index_def = NULL ;
   void *tmp                = NULL ;
   sdbCollection *cl        = NULL ;
   bson::BSONObj *index_def = NULL ;
   const char *name         = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOsii", &obj, &bson_index_def, &name,
                                           &is_unique, &is_enforced ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_index_def, tmp, index_def ) ;

   rc = cl->create_index( index_def, name, is_unique, is_enforced ) ;
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
   INT32 rc                = 0 ;
   PYOBJECT *obj           = NULL ;
   PYOBJECT *ret_object    = NULL ;
   PYOBJECT *cursor_object = NULL ;
   void *tmp               = NULL ;
   sdbCollection *cl       = NULL ;
   sdbCursor *cursor       = NULL ;
   const char *index_name  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOs", &obj, &cursor_object, &index_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, tmp, sdbCursor, cursor ) ;

   rc = cl->get_index( *cursor, index_name ) ;
   if ( rc )
   {
      goto done ;
   }

   MAKE_PYTHON_VOID_OBJECT( cursor, ret_object ) ;

done:
   return MAKE_RETURN_INT_OBJECT( rc,  ) ;
}

static PYOBJECT *drop_index( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   PYOBJECT *obj           = NULL ;
   void *tmp               = NULL ;
   sdbCollection *cl       = NULL ;
   sdbCursor *cursor       = NULL ;
   const char *index_name  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &index_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;

   rc = cl->drop_index( index_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_collection_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   sdbCollection *cl   = NULL ;
   const char *cl_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;

   cl_name = cl->get_collection_name() ;

done:
   return MAKE_RETURN_INT_PYSTRING( rc, cl_name ) ;
}

static PYOBJECT *get_collection_space_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   sdbCollection *cl   = NULL ;
   const char *cs_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;

   cs_name = cl->get_collection_name() ;

done:
   return MAKE_RETURN_INT_PYSTRING( rc, cs_name ) ;
}

static PYOBJECT *get_full_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc              = 0 ;
   PYOBJECT *obj         = NULL ;
   void *tmp             = NULL ;
   sdbCollection *cl     = NULL ;
   const char *full_name = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;

   full_name = cl->get_full_name() ;

done:
   return MAKE_RETURN_INT_PYSTRING( rc, full_name ) ;
}

static PYOBJECT *aggregate( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   PYOBJECT *obj           = NULL ;
   PYOBJECT *ret_object    = NULL ;
   PYOBJECT *cl_object     = NULL ;
   PYOBJECT *list_object   = NULL ;
   PYOBJECT *cursor_object = NULL ;
   void *tmp               = NULL ;
   sdbCollection *cl       = NULL ;
   sdbCursor *cursor       = NULL ;

   std::vector< bson::BSONObj > vec_bson ;

   if ( !PARSE_PYTHON_ARGS( args, "OOO", &obj, &cursor_object, &list_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, tmp, sdbCursor, cursor ) ;
   MAKE_PYLIST_TO_VECTOR( list_object, list_size, tmp, vec_bson ) ;
   rc = cl->aggregate( *cursor, vec_bson ) ;
   if ( rc )
   {
      goto done ;
   }

   MAKE_PYTHON_VOID_OBJECT( cursor, ret_object ) ;

done:
   return MAKE_RETURN_INT_OBJECT( rc, ret_object ) ;
}

static PYOBJECT *get_query_meta( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                     = 0 ;
   INT64 num_to_skip            = 0 ;
   INT64 num_to_return          = 0 ;
   PYOBJECT *obj                = NULL ;
   PYOBJECT *ret_object         = NULL ;
   PYOBJECT *cursor_object      = NULL ;
   PYOBJECT *bson_condition     = NULL ;
   PYOBJECT *bson_order_by      = NULL ;
   PYOBJECT *bson_hint          = NULL ;
   void *tmp                    = NULL ;
   sdbCollection *cl            = NULL ;
   sdbCursor *cursor            = NULL ;
   bson::BSONObj *condition     = NULL ;
   bson::BSONObj *order_by      = NULL ;
   bson::BSONObj *hint          = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OOOOOOii", &obj, &cursor_object,
                            &bson_condition, &bson_order_by, &bson_hint,
                            &num_to_skip, &num_to_return ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYOBJECT_TO_COBJECT( cursor_object, tmp, sdbCursor, cursor ) ;
   CAST_PYBSON_TO_CPPBSON( bson_condition, tmp, condition ) ;
   CAST_PYBSON_TO_CPPBSON( bson_order_by, tmp, order_by ) ;
   CAST_PYBSON_TO_CPPBSON( bson_hint, tmp, hint ) ;

   rc = cl->get_query_meta( *cursor, condition, selector, order_by, hint,
                            num_to_skip, &num_to_return ) ;
   if ( rc )
   {
      goto done ;
   }

   MAKE_PYTHON_VOID_OBJECT( cursor, ret_object ) ;

done:
   DELETE_CPPOBJECT( condition ) ;
   DELETE_CPPOBJECT( order_by ) ;
   DELETE_CPPOBJECT( hint ) ;
   return MAKE_RETURN_INT_OBJECT( rc ) ;
}

static PYOBJECT *attach_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                  = 0 ;
   PYOBJECT *obj             = NULL ;
   PYOBJECT *ret_object      = NULL ;
   PYOBJECT *bson_option     = NULL ;
   void *tmp                 = NULL ;
   const char *sub_full_name = NULL ;
   sdbCollection *cl         = NULL ;
   bson::BSONObj *option     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &sub_full_name, &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }
   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, tmp, option ) ;

   rc = cs->attach_collection( sub_full_name, option ) ;
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
   INT32 rc                  = 0 ;
   PYOBJECT *obj             = NULL ;
   void *tmp                 = NULL ;
   const char *sub_full_name = NULL ;
   sdbCollection *cl         = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &sub_full_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }
   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollection, cl ) ;

   rc = cs->detach_collection( sub_full_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}