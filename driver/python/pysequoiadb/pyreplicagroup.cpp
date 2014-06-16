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
#include <Python.h>

#include "util.hpp"
using namespace sdbclient ;

static void release_node( void *ptr )
{
   sdbNode *client = static_cast<sdbNode *>(ptr) ;
   SDB_OSS_DEL client ;
   client = NULL ;
   return;
}

static PYOBJECT *get_nodenum( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   INT32 nodestatus     = SDB_NODE_UNKNOWN ;
   INT32 nodenum        = 0 ;

   if ( !PyArg_ParseTuple( args, "Oi", &obj, &nodestatus ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   nodenum = replica_group->getNodeNum( nodestatus, &rc ) ;
done :
   return Py_BuildValue( "(i,i)", rc, nodenum ) ;
error :
   goto done ;
}

static PYOBJECT *get_detail( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   bson::BSONObj bson ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   INT32 rc = replica_group->getDetail( bson ) ;
done :
   return Py_BuildValue("(i,s#)", rc, bson.objdata(), bson.objsize() ) ;
error :
   goto done ;
}

static PYOBJECT *get_master( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   sdbNode *node        = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   node = SDB_OSS_NEW sdbNode() ;
   if ( NULL == node )
   {
      rc = SDB_OOM ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   INT32 rc = replica_group->getMaster( *node ) ;
   if  ( SDB_OK != rc )
   {
      goto error ;
   }
done :
   return Py_BuildValue( "(i,O)", rc, PyCObject_FromVoidPtr( client, release_client) ) ;
error :
   if ( NULL != node )
   {
      release_node( node ) ;
      node = NULL ;
   }
   goto done ;
}

static PYOBJECT *get_slave( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   sdbNode *node        = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   node = SDB_OSS_NEW sdbNode() ;
   if ( NULL == node )
   {
      rc = SDB_OOM ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   INT32 rc = replica_group->getSlave( *node ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
done :
   return Py_BuildValue( "(i,O)", rc, PyCObject_FromVoidPtr( client, release_client) ) ;
error :
   if ( NULL != node )
   {
      release_node( node ) ;
      node = NULL ;
   }
   goto done ;
}

static PYOBJECT *get_nodebyname( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   PYOBJECT *obj           = NULL ;
   const char *nodename    = NULL ;
   sdbNode *node           = NULL ;

   if ( !PyArg_ParseTuple( args, "Os", &obj, &nodename ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   node = SDB_OSS_NEW sdbNode() ;
   if ( NULL == node )
   {
      rc = SDB_OOM ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   rc = replica_group->getNode( nodename, *node ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
done :
   return Py_BuildValue( "(i,O)", rc, PyCObject_FromVoidPtr( client, release_client) ) ;
error :
   if ( NULL != node )
   {
      release_node( node ) ;
      node = NULL ;
   }
   goto done ;
}

static PYOBJECT *get_nodebyendpoint( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   PYOBJECT *obj           = NULL ;
   const char *hostname   = NULL ;
   const char *servicename = NULL ;
   sdbNode *node           = NULL ;

   if ( !PyArg_ParseTuple( args, "Oss", &obj, &hostname, &servicename ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   node = SDB_OSS_NEW sdbNode() ;
   if ( NULL == node )
   {
      rc = SDB_OOM ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   rc = replica_group->getNode( hostname, servicename, *node ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
done :
   return Py_BuildValue( "(i,O)", rc, PyCObject_FromVoidPtr( client, release_client) ) ;
error :
   if ( NULL != node )
   {
      release_node( node ) ;
      node = NULL ;
   }
   goto done ;
}

static INT32 pydict_to_cmap( PYOBJECT *pyobj, std::map<std::string,std::string>& cobj )
{
   INT32 rc = SDB_OK ;
   if ( !PyDict_Check( pyobj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }   
   
   PyObject *key, *keys;
   keys = PyDict_Keys( pyobj );
   for ( int i = 0; i < PyList_GET_SIZE( keys ); ++i )
   {
      key = PyList_GET_ITEM( keys, i ) ;
      const char *key_name = PyString_AsString( key );
      
      PyObject *val = PyDict_GetItemString( pyobj, key_name );
      if ( NULL == val || !PyString_Check( val ) )
      {
         rc = SDB_INVALIDARGS ;       
         goto error ;
      }
      cobj[ key_name ] = PyString_AsString( val );
   }
   
done :
   return rc ;
error :
   goto done ;
}

static PYOBJECT *create_node( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   PYOBJECT *obj           = NULL ;
   const char *nodename    = NULL ;
   const char *servicename = NULL ;
   const char *nodepath    = NULL ;
   PYOBJECT *dict          = NULL ;

   if ( !PyArg_ParseTuple( args, "OsssO", &obj, &nodename, 
                            &servicename, &nodepath, &dict ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   std::map<std::string,std::string> config ;
   rc = pydict_to_cmap( dict, config ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
   
   rc = replica_group->createNode( nodename, servicename, nodepath, config ) ;
   if ( SDB_OK != rc )
   {
      goto error ;
   }
done :
   return MAKE_RETURN_INT_VALUE( rc ) ;
error :
   goto done ;
}

static PYOBJECT *remove_node( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc                = 0 ;
   PYOBJECT *obj           = NULL ;
   PYOBJECT *pybson        = NULL ;
   const char *hostname    = NULL ;
   const char *servicename = NULL ;

   if ( !PyArg_ParseTuple( args, "Oss|O", &obj, &hostname, &servicename, pybson ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   if ( NULL == pybson )
   {
      rc = replica_group->removeNode( hostname, servicename ) ;
   }
   else
   {
      const char *bson_string = PyBytes_AsString( pybson );
      bson::BSONObj cbson( bson_string );

      rc = replica_group->removeNode( hostname, servicename, cbson ) ;
   }
done :
   return MAKE_RETURN_INT_VALUE( rc ) ;
error :
   goto done ;
}

static PYOBJECT *start( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   rc = replica_group->stop() ;
done :
   return MAKE_RETURN_INT_VALUE( rc ) ;
error :
   goto done ;
}

static PYOBJECT *stop( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   rc = replica_group->start() ;
done :
   return MAKE_RETURN_INT_VALUE( rc ) ;
error :
   goto done ;
}

static PYOBJECT *is_catalog( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbReplicaGroup *replica_group = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   rc = replica_group->isCatalog() ;
done :
   return MAKE_RETURN_INT_VALUE( rc ) ;
error :
   goto done ;
}

/* List of functions defined in the module */
static PyMethodDef group_methods[] = {
   {"get_nodenum", get_nodenum ,METH_VARARGS},
   {"get_detail", get_detail ,METH_VARARGS},
   {"get_master", get_master ,METH_VARARGS},
   {"get_slave", get_slave ,METH_VARARGS},
   {"get_nodebyname", get_nodebyname ,METH_VARARGS},
   {"get_nodebyendpoint", get_nodebyendpoint ,METH_VARARGS},
   {"create_node", create_node ,METH_VARARGS},
   {"remove_node", remove_node ,METH_VARARGS},
   {"start", start ,METH_VARARGS},
   {"stop", stop ,METH_VARARGS},
   {"is_catalog", is_catalog ,METH_VARARGS},
   {NULL, NULL}
};

CREATE_MODULE( sdbreplicagroup, group_methods )

