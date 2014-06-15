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

   return MAKE_RETURN_OBJECT( client, release) ;
}

static PYOBJECT *init_connect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   PYOBJECT *obj    = NULL ;
   const char *host = NULL ;
   int port         = 0 ;

   if ( !PARSE_PYTHON_ARGS( args, "Osi", &obj, &host, &port ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

done:
   return MAKE_RETURN_INT_VALUE( rc ) ;
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

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, client, sdb )

   SDB_OSS_DEL client ;
   client = NULL ;

done:
   return MAKE_RETURN_INT_VALUE( rc ) ;
}

static PYOBJECT *connect_by_host( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc         = 0 ;
   int port         = 0 ;
   PYOBJECT *obj    = NULL ;
   void *tmp        = NULL ;
   const char *host = NULL ;
   const char *user = NULL ;
   const char *psw  = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Osiss", &obj, &host, &port, &user, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, client, sdb )

   rc = client.connect( host, port, user, psw ) ;

done:
   return MAKE_RETURN_INT_VALUE( rc ) ;
}

static PYOBJECT *connect_by_service( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   const char *host    = NULL ;
   const char *service = NULL ;
   const char *user    = NULL ;
   const char *psw     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Ossss", &obj, &host, &service, &user, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, client, sdb ) ;
   
   rc = client.connect( host, service, user, psw ) ;

done:
   return MAKE_RETURN_INT_VALUE( rc ) ;
}

static PYOBJECT *connect_by_address( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   INT32 addr_size     = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   const char **addr   = NULL ;
   const char *user    = NULL ;
   const char *psw     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "Osiss", &obj, &addr, &addr_size, &user, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, client, sdb ) ;

   rc = client.connect( addr, addr_size, user, psw ) ;

done:
   return MAKE_RETURN_INT_VALUE( rc ) ;
}

static PYOBJECT *disconnect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc            = 0 ;
   INT32 addr_size     = 0 ;
   PYOBJECT *obj       = NULL ;
   void *tmp           = NULL ;
   const char **addr   = NULL ;
   const char *user    = NULL ;
   const char *psw     = NULL ;

   if ( !PARSE_PYTHON_ARGS( args, "O", &obj ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   CAST_PYOBJECT_TO_COBJECT( obj, tmp, client, sdb ) ;

   rc = client.connect( addr, addr_size, user, psw ) ;

done:
   return MAKE_RETURN_INT_VALUE( rc ) ;
}

static PYOBJECT *create_user( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *remove_user( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_snapshot( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *reset_snapshot( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_list( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *lock( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *unlock( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_collection_space( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *create_collection_space( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *drop_collection_space( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *list_collection_space( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_replica_group_by_name( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_replica_group_by_id( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *create_replica_group( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *remove_replica_group( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *create_replica_cata_group( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *active_replica_group( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *exec_update( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *exec_sql( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *transaction_begin( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *transaction_commit( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *transaction_rollback( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *flush_configure( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *backup_offline( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *list_backup( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *list_task( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *wait_task( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *cancel_task( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *set_session_attri( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *close_all_cursors( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *is_valid( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *connect( PYOBJECT *self, PYOBJECT *args )
{
   INT32 rc             = 0 ;
   PYOBJECT *obj        = NULL ;
   const char *host     = NULL ;
   int port             = 0 ;
   const char *user     = NULL ;
   const char *psw      = NULL ;

   if ( ! PyArg_ParseTuple( args, "Osiss", &obj, &host, &port, &user, &psw ) )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   VOID *tmp = PyCObject_AsVoidPtr( obj ) ;
   sdb *client = static_cast< sdb * >( tmp ) ;
   if ( NULL == client )
   {
      rc = SDB_INVALIDARGS ;
      goto done ;
   }

   rc = cur->connect( host, port, user, psw ) ;
done:
   return MAKE_RETURN_INT_VALUE( rc ) ;
}

