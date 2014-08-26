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
#include "client.hpp"

using namespace sdbclient ;


static PYOBJECT *create_cs( PYOBJECT *self, PYOBJECT *args )
{
   sdbCollectionSpace *cs = NULL ;
   NEW_CPPOBJECT( cs, sdbCollectionSpace ) ;
   if ( NULL == cs )
   {
      return NULL ;
   }

   return MAKE_PYOBJECT( cs ) ;
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

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCollectionSpace, cs ) ;
   DELETE_CPPOBJECT( cs );

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *cl_object    = NULL ;
   const CHAR *cl_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;
   sdbCollection *cl      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &cl_name, &cl_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCollectionSpace, cs ) ;
   CAST_PYOBJECT_TO_COBJECT( cl_object, sdbCollection, cl ) ;

   rc = cs->getCollection( cl_name, *cl ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   PYOBJECT *cl_object    = NULL ;
   const CHAR *cl_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;
   sdbCollection *cl      = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsO", &obj, &cl_name, &cl_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCollectionSpace, cs ) ;
   CAST_PYOBJECT_TO_COBJECT( cl_object, sdbCollection, cl ) ;

   rc = cs->createCollection( cl_name, *cl ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *create_collection_use_opt( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                    = 0 ;
   PYOBJECT *obj               = NULL ;
   PYOBJECT *cl_object         = NULL ;
   PYOBJECT *bson_option       = NULL ;
   const CHAR *cl_name         = NULL ;
   sdbCollectionSpace *cs      = NULL ;
   sdbCollection *cl           = NULL ;
   const bson::BSONObj *option = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "OsOO", &obj, &cl_name, &bson_option,
                                                          &cl_object ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCollectionSpace, cs ) ;
   CAST_PYOBJECT_TO_COBJECT( cl_object, sdbCollection, cl ) ;
   CAST_PYBSON_TO_CPPBSON( bson_option, option ) ;

   rc = cs->createCollection( cl_name, *option, *cl ) ;
   if ( rc )
   {
      goto done ;
   }

done:
   DELETE_CPPOBJECT( option ) ;
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *drop_collection( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc               = 0 ;
   PYOBJECT *obj          = NULL ;
   const CHAR *cl_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Os", &obj, &cl_name ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCollectionSpace, cs ) ;

   rc = cs->dropCollection( cl_name ) ;
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
   const CHAR *cs_name    = NULL ;
   sdbCollectionSpace *cs = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCollectionSpace, cs ) ;

   cs_name = cs->getCSName() ;

done :
   return MAKE_RETURN_INT_PYSTRING( rc, cs_name ) ;
}

/* List of functions defined in the module */
static PyMethodDef sdbcollectionspace_methods[] = {
   {"create_cs",                 create_cs,                 METH_VARARGS},
   {"release_cs",                release_cs,                METH_VARARGS},
   {"get_collection",            get_collection,            METH_VARARGS},
   {"create_collection",         create_collection,         METH_VARARGS},
   {"create_collection_use_opt", create_collection_use_opt, METH_VARARGS},
   {"drop_collection",           drop_collection,           METH_VARARGS},
   {"get_collection_space_name", get_collection_space_name, METH_VARARGS},
   {NULL, NULL}
};

CREATE_MODULE( libsdbcs, sdbcollectionspace_methods )
