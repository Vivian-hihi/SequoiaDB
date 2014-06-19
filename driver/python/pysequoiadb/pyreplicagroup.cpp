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
typedef sdbReplicaGroup Group ;

static PYOBJECT *create_replicagroup( PYOBJECT *self, PYOBJECT *args )
{
   Group *replica_group = NULL;
   if ( !PyArg_ParseTuple(args, "") )
   {
     goto error ;
   }
   
   NEW_CPPOBJECT( replica_group, Group ) ;
   if ( NULL == replica_group )
   {
     goto error ;
   }
   
   return MAKE_PYOBJECT( replica_group ) ;
error :
   return NULL ;
}

static PYOBJECT *release_replicagroup( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   Group *replica_group = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj,  Group, replica_group ) ;
   DELETE_CPPOBJECT( replica_group ) ;
done:
   return MAKE_RETURN_INT( rc ) ;
}

static PYOBJECT *get_nodenum( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   INT32 nodestatus    = SDB_NODE_UNKNOWN ;
   INT32 nodenum      = 0 ;
   Group *replica_group = NULL ;

   if ( !PyArg_ParseTuple( args, "Oi", &obj, &nodestatus ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, Group, replica_group ) ;
   nodenum = replica_group->getNodeNum( (sdbNodeStatus)nodestatus, &rc ) ;
done :
   return MAKE_RETURN_INT_INT(rc, nodenum) ;
error :
   goto done ;
}

static PYOBJECT *get_detail( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   bson::BSONObj bson ;
   Group *replica_group = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, Group, replica_group ) ;
   rc = replica_group->getDetail( bson ) ;
done :
   return MAKE_RETURN_INT_PYSTRING_BYSIZE( rc, bson.objdata(), 
                                    bson.objsize() ) ;
error :
   goto done ;
}

static INT32 convert_pobj2cobj( PYOBJECT *self, PYOBJECT *args, 
                 sdbReplicaGroup *& group, sdbNode *& node)
{
   INT32 rc           = 0 ;
   PYOBJECT *group_obj   = NULL ;
   PYOBJECT *node_obj   = NULL ;

   if ( !PyArg_ParseTuple( args, "OO", &group_obj, &node_obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( group_obj, Group, group ) ;
   CAST_PYOBJECT_TO_COBJECT( node_obj, sdbNode, node ) ;
done :
   return rc;
error :
   goto done ;
}

static PYOBJECT *get_master( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   sdbNode *node      = NULL ;
   Group *replica_group = NULL ;

   rc = convert_pobj2cobj( self, args, replica_group, node) ;
   if  ( SDB_OK != rc )
   {
     goto error ;
   }

   rc = replica_group->getMaster( *node ) ;
   if  ( SDB_OK != rc )
   {
     goto error ;
   }
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *get_slave( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   sdbNode *node      = NULL ;
   Group *replica_group = NULL ;

   rc = convert_pobj2cobj( self, args, replica_group, node) ;
   if  ( SDB_OK != rc )
   {
     goto error ;
   }
   rc = replica_group->getSlave( *node ) ;
   if ( SDB_OK != rc )
   {
     goto error ;
   }
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *get_nodebyname( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *group_obj  = NULL ;
   PYOBJECT *node_obj   = NULL ;
   const char *nodename = NULL ;
   sdbNode *node      = NULL ;
   Group *replica_group = NULL ;

   if ( !PyArg_ParseTuple( args, "OOs", &group_obj, &node_obj, &nodename ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }
   
   CAST_PYOBJECT_TO_COBJECT( group_obj, Group, replica_group ) ;
   CAST_PYOBJECT_TO_COBJECT( node_obj, sdbNode, node ) ;

   rc = replica_group->getNode( nodename, *node ) ;
   if ( SDB_OK != rc )
   {
     goto error ;
   }
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *get_nodebyendpoint( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *group_obj    = NULL ;
   PYOBJECT *node_obj     = NULL ;
   const char *hostname   = NULL ;
   const char *servicename = NULL ;
   sdbNode *node         = NULL ;
   Group *replica_group   = NULL ;

   if ( !PyArg_ParseTuple( args, "OOss", &group_obj, &node_obj, 
                            &hostname, &servicename ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }
   
   CAST_PYOBJECT_TO_COBJECT( group_obj, Group, replica_group ) ;
   CAST_PYOBJECT_TO_COBJECT( node_obj, sdbNode, node ) ;
   rc = replica_group->getNode( hostname, servicename, *node ) ;
   if ( SDB_OK != rc )
   {
     goto error ;
   }
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static INT32 pydict_to_cmap( PYOBJECT *pyobj, std::map<std::string,std::string>& cobj )
{
   INT32 rc = SDB_OK ;
   PyObject *key, *keys;

   if ( !PyDict_Check( pyobj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }   
   
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
   INT32 rc            = 0 ;
   PYOBJECT *obj         = NULL ;
   const char *nodename   = NULL ;
   const char *servicename = NULL ;
   const char *nodepath   = NULL ;
   PYOBJECT *dict        = NULL ;
   std::map<std::string,std::string> config ;
   Group *replica_group   = NULL ;

   if ( !PyArg_ParseTuple( args, "OsssO", &obj, &nodename, 
                     &servicename, &nodepath, &dict ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }
   
   CAST_PYOBJECT_TO_COBJECT( obj, Group, replica_group ) ;
   
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
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *remove_node( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj         = NULL ;
   PYOBJECT *pybson      = NULL ;
   const char *hostname   = NULL ;
   const char *servicename = NULL ;
   Group *replica_group = NULL ;

   if ( !PyArg_ParseTuple( args, "Oss|O", &obj, &hostname, &servicename, pybson ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

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
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *start( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   sdbReplicaGroup *replica_group = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, sdbReplicaGroup, replica_group ) ;
   rc = replica_group->stop() ;
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *stop( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   sdbReplicaGroup *replica_group = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, Group, replica_group ) ;
   rc = replica_group->start() ;
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

static PYOBJECT *is_catalog( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc          = 0 ;
   PYOBJECT *obj      = NULL ;
   sdbReplicaGroup *replica_group = NULL ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
     rc = SDB_INVALIDARGS ;
     goto error ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, Group, replica_group ) ;
   rc = replica_group->isCatalog() ;
done :
   return MAKE_RETURN_INT( rc ) ;
error :
   goto done ;
}

/* List of functions defined in the module */
static PyMethodDef group_methods[] = {
   {"create_replicagroup", create_replicagroup ,METH_VARARGS},
   {"release_replicagroup", release_replicagroup ,METH_VARARGS},
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

