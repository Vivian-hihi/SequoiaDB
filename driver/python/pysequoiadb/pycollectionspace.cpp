#include <Python.h>
#include <client.hpp>

#include "util.hpp"

static PYOBJECT *create_cs( PYOBJECT *self )
{
   sdbCollectionSpace *cs = SDB_OSS_NEW sdbCollectionSpace() ;
   if ( NULL == cs )
   {
      return NULL ;
   }

   return MAKE_RETURN_OBJECT( cs ) ;
}

static PYOBJECT *release_cs( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   sdbCollectionSpace *cs = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollectionSpace, cs ) ;
   rc = cs->release_cs() ;

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *ret_object   = NULL ;
   PYOBJECT *cl_object    = NULL ;
   void *tmp              = NULL ;
   const char *cl_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;
   sdbCollection *cl      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &cl_name, &cl_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollectionSpace, cs ) ;
   CAST_PYOBJECT_TO_COBJECT( cl_object, tmp, sdbCollection, cl ) ;

   rc = cs->get_collection( cl_name, *cl ) ;
   if ( rc )
   {
      goto done ;
   }

   MAKE_PYTHON_VOID_OBJECT( cl, ret_object ) ;

done:
   return MAKE_RETURN_INT_OBJECT( rc, ret_object ) ;
}

static PYOBJECT *create_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *ret_object   = NULL ;
   PYOBJECT *cl_object    = NULL ;
   void *tmp              = NULL ;
   const char *cl_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;
   sdbCollection *cl      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &cl_name, &cl_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollectionSpace, cs ) ;
   CAST_PYOBJECT_TO_COBJECT( cl_object, tmp, sdbCollection, cl ) ;

   rc = cs->create_collection( cl_name, *cl ) ;
   if ( rc )
   {
      goto done ;
   }

   MAKE_PYTHON_VOID_OBJECT( cl, ret_object ) ;

done:
   return MAKE_RETURN_INT_OBJECT( rc, ret_object ) ;
}

static PYOBJECT *create_collection_use_opt( PYOBJECT *self, PYOBJECT *args ) ;
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *ret_object   = NULL ;
   PYOBJECT *cl_object    = NULL ;
   PYOBJECT *bson_option  = NULL ;
   void *tmp              = NULL ;
   const char *cl_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;
   sdbCollection *cl      = NULL ;
   bson::BSONObj *option  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsOO", &obj, &cl_name, &cl_object,
                                                          &bson_option ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollectionSpace, cs ) ;
   CAST_PYOBJECT_TO_COBJECT( cl_object, tmp, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, tmp, option ) ;

   rc = cs->create_collection_use_opt( cl_name, option, *cl ) ;
   if ( rc )
   {
      goto done ;
   }

   MAKE_PYTHON_VOID_OBJECT( cl, ret_object ) ;

done:
   DELETE_CPPOBJECT( option ) ;
   return MAKE_RETURN_INT_OBJECT( rc, ret_object ) ;
}

static PYOBJECT *drop_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   void *tmp              = NULL ;
   const char *cl_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &cl_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollectionSpace, cs ) ;

   rc = cs->drop_collection( cl_name ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_collection_space_name( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *ret_object   = NULL ;
   void *tmp              = NULL ;
   const char *cs_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, sdbCollectionSpace, cs ) ;

   cs_name = cs->get_collection_space_name() ;

done:
   return MAKE_RETURN_PYSTRING( cs_name ) ;
error:
   return MAKE_RETURN_INT( rc ) ;
}