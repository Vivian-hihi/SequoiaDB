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
static PYOBJECT *create_node( PYOBJECT *self, PYOBJECT *args )
{
   sdbNode *node = NULL;
   if ( !PyArg_ParseTuple(args, "") )
   {
      return NULL ;
   }
   
   NEW_CPPOBJECT( node, sdbNode ) ;
   if ( NULL == node )
   {
      return NULL ;
   }

   return MAKE_PYOBJECT( node ) ;
}

static PYOBJECT *release_node( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc       = 0 ;
   PYOBJECT *obj  = NULL ;
   sdbNode  *node = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   DELETE_CPPOBJECT( node ) ;
done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *connect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   PYOBJECT *obj    = NULL ;
   PYOBJECT *sdbodj = NULL ;
   sdbNode *node    = NULL ;
   sdb *client      = NULL ;

   if ( !PyArg_ParseTuple( args, "OO", &obj, &sdbodj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   CAST_PYOBJECT_TO_COBJECT( sdbodj, sdb, client ) ;
   rc = node->connect( *client ) ;
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *get_status( PYOBJECT *self, PYOBJECT *args )
{
   INT32  rc        = 0 ;
   PYOBJECT *obj    = NULL ;
   INT32 nodestatus = SDB_NODE_UNKNOWN ;
   sdbNode *node    = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   nodestatus = node->getStatus() ;
done :
   return MAKE_RETURN_INT_INT( rc, nodestatus ) ;
error :
   goto done ;
}

static PYOBJECT *get_hostname( PYOBJECT *self, PYOBJECT *args )
{
   INT32  rc            = 0 ;
   PYOBJECT *obj        = NULL ;
   const char *hostname = "" ;
   sdbNode *node        = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   hostname = node->getHostName() ;
done :
   return MAKE_RETURN_INT_PYSTRING( rc, hostname ) ;
error :
   goto done ;
}

static PYOBJECT *get_servicename( PYOBJECT *self, PYOBJECT *args )
{
   INT32  rc               = 0 ;
   PYOBJECT *obj           = NULL ;
   const char *servicename = "" ;
   sdbNode *node           = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   servicename = node->getServiceName() ;
done :
   return MAKE_RETURN_INT_PYSTRING( rc, servicename ) ;
error :
   goto done ;
}

static PYOBJECT *get_nodename( PYOBJECT *self, PYOBJECT *args )
{
   INT32  rc            = 0 ;
   PYOBJECT *obj        = NULL ;
   const char *nodename = "" ;
   sdbNode *node        = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   nodename = node->getNodeName() ;
done :
   return MAKE_RETURN_INT_PYSTRING( rc, nodename ) ;
error :
   goto done ;
}

static PYOBJECT *stop( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   sdbNode *node = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   rc = node->stop() ;
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *start( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc      = 0 ;
   PYOBJECT *obj = NULL ;
   sdbNode *node = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   rc = node->start() ;
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

/* List of functions defined in the module */
static PyMethodDef node_methods[] = {
   {"create_node", create_node ,METH_VARARGS},
   {"release_node", release_node ,METH_VARARGS},
   {"connect", connect ,METH_VARARGS},
   {"get_status", get_status ,METH_VARARGS},
   {"get_hostname", get_hostname ,METH_VARARGS},
   {"get_servicename", get_servicename ,METH_VARARGS},
   {"get_nodename", get_nodename ,METH_VARARGS},
   {"stop", stop ,METH_VARARGS},
   {"start", start ,METH_VARARGS},
   {NULL, NULL}
};

CREATE_MODULE( libsdbnode, node_methods )

