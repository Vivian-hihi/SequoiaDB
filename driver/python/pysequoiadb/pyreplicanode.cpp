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

static PYOBJECT *connect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   PYOBJECT *sdbodj     = NULL ;

   if ( !PyArg_ParseTuple( args, "OO", &obj, &sdbodj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbNode *node = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   sdb *client   = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdb, client ) ;
   nodenum = node->connect( *client ) ;
done :
   return MAKE_RETURN_INT_VALUE( rc ) ;
error :
   goto done ;
}

static PYOBJECT *get_status( PYOBJECT *self, PYOBJECT *args )
{
   INT32  rc              = 0 ;
   PYOBJECT *obj          = NULL ;
   INT32 nodestatus       = SDB_NODE_UNKNOWN ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbNode *node = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   nodestatus = node->getStatus() ;
done :
   return Py_BuildValue( "(i,i)", rc, nodestatus ) ;
error :
   goto done ;
}

static PYOBJECT *get_hostname( PYOBJECT *self, PYOBJECT *args )
{
   INT32  rc              = 0 ;
   PYOBJECT *obj          = NULL ;
   const char *hostname = "" ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbNode *node = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   hostname = node->getHostName() ;
done :
   return Py_BuildValue( "(i,s)", rc, hostname ) ;
error :
   goto done ;
}

static PYOBJECT *get_servicename( PYOBJECT *self, PYOBJECT *args )
{
   INT32  rc               = 0 ;
   PYOBJECT *obj           = NULL ;
   const char *servicename = "" ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbNode *node = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   servicename = node->getServiceName() ;
done :
   return Py_BuildValue( "(i,s)", rc, servicename ) ;
error :
   goto error ;
}

static PYOBJECT *get_nodename( PYOBJECT *self, PYOBJECT *args )
{
   INT32  rc              = 0 ;
   PYOBJECT *obj          = NULL ;
   const char *nodename   = "" ;

   if ( !PyArg_ParseTuple( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto error ;
   }

   sdbNode *node = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   nodename = node->getNodeName() ;
done :
   return Py_BuildValue( "(i,s)", rc, nodename ) ;
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

   sdbNode *node = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   rc = node->stop() ;
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

   sdbNode *node = NULL ;
   CAST_PYOBJECT_TO_COBJECT( obj, sdbNode, node ) ;
   rc = node->start() ;
done :
   return MAKE_RETURN_INT_VALUE( rc ) ;
error :
   goto done ;
}

