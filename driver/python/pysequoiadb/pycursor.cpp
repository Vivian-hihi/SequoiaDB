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

static PYOBJECT *create_cursor( PYOBJECT *self, PYOBJECT *args )
{
   sdbCursor *cursor = NULL;
   if ( !PyArg_ParseTuple(args, "") )
   {
     goto error ;
   }
   
   NEW_CPPOBJECT( cursor, sdbCursor ) ;
   if ( NULL == cursor )
   {
     goto error ;
   }
   return MAKE_PYOBJECT( cursor ) ;
error :
   return NULL;
}

static PYOBJECT *release_cursor( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   PYOBJECT *obj     = NULL ;
   sdbCursor *cursor  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  sdbCursor, cursor ) ;
   DELETE_CPPOBJECT( cursor ) ;
done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *next( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   sdbCursor *cursor   = NULL ;
   bson::BSONObj bson ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCursor, cursor ) ;
   rc = cursor->next( bson ) ;
   if ( SDB_OK != rc )
   {
     goto error ;
   }
done :
   return MAKE_RETURN_INT_PYSTRING_BYSIZE( rc, bson.objdata(), bson.objsize() ) ;
error :
   goto done ;
}

static PYOBJECT *current( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   sdbCursor *cursor   = NULL ;
   bson::BSONObj bson ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCursor, cursor ) ;
   rc = cursor->current( bson ) ;
   if ( SDB_OK != rc )
   {
     goto error ;
   }
done :
   return MAKE_RETURN_INT_PYSTRING_BYSIZE( rc, bson.objdata(), bson.objsize() ) ;
error :
   goto done ;
}

static PYOBJECT *close( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   sdbCursor *cursor   = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbCursor, cursor ) ;
   rc = cursor->close( ) ;
   if ( SDB_OK != rc )
   {
     goto error ;
   }
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

/* List of functions defined in the module */
static PyMethodDef cursor_methods[] = {
   {"create_cursor", create_cursor ,METH_VARARGS},
   {"release_cursor", release_cursor ,METH_VARARGS},
   {"next", next ,METH_VARARGS},
   {"current", current ,METH_VARARGS},
   {"close", close ,METH_VARARGS},
   {NULL, NULL}
};

CREATE_MODULE( sdbcursor, cursor_methods )
