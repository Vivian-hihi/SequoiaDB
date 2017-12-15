#include "sdb_adaptor.h"
#include "sdb_err_code.h"
#include "sdb_util.h"
#include "sdb_conn.h"
#include "sdb_conn_ptr.h"

sdb_adaptor::sdb_adaptor()
{
   pthread_rwlock_init( &rw_mutex, NULL ) ;
}

sdb_adaptor::~sdb_adaptor()
{
   DBUG_ASSERT( conn_list.size() == 0 ) ;
   pthread_rwlock_destroy( &rw_mutex ) ;
}

sdb_adaptor *sdb_adaptor::get_instance()
{
   static sdb_adaptor _sdb_adaptor ;
   return &_sdb_adaptor ;
}

int sdb_adaptor::get_sdb_conn( my_thread_id tid, sdb_conn_auto_ptr &sdb_ptr )
{
   int rc = 0 ;
   std::map<my_thread_id, sdb_conn_auto_ptr>::iterator iter ;

   sdb_ptr.clear() ;
   {
   sdb_rw_lock_r r_lock( &rw_mutex ) ;
   iter = conn_list.find( tid ) ;
   if ( conn_list.end() != iter )
   {
      sdb_ptr = iter->second ;
      goto done ;
   }
   }

   {
   sdb_conn_auto_ptr tmp_conn( new sdb_conn( tid ) ) ;
   sdb_ptr = tmp_conn ;
   rc = sdb_ptr->connect() ;
   sdb_rw_lock_w w_lock( &rw_mutex ) ;
   conn_list.insert( std::pair<my_thread_id,sdb_conn_auto_ptr>(tid,sdb_ptr) ) ;
   }
done:
   return rc ;
}

void sdb_adaptor::del_sdb_conn( my_thread_id tid )
{
   sdb_rw_lock_w w_lock( &rw_mutex ) ;
   std::map<my_thread_id, sdb_conn_auto_ptr>::iterator iter ;
   iter = conn_list.find( tid ) ;
   if ( conn_list.end() == iter
        || iter->second.ref() > 1
        || !iter->second->is_idle() )
   {
      goto done ;
   }
   conn_list.erase( iter );
done:
   return ;
}

int sdb_adaptor::get_sdb_cl( my_thread_id tid, char *cs_name,
                             char *cl_name, sdb_cl_auto_ptr &collection,
                             bool create )
{
   int rc = SDB_ERR_OK ;
   sdb_conn_auto_ptr connection ;

   rc = get_sdb_conn( tid, connection ) ;
   if ( rc != SDB_ERR_OK )
   {
      goto error ;
   }

   rc = connection->get_cl( cs_name, cl_name, collection, create ) ;
   if ( rc != SDB_ERR_OK )
   {
      goto error ;
   }

done:
   return rc ;
error:
   goto done ;
}

int sdb_adaptor::create_sdb_cl( my_thread_id tid, char *cs_name,
                                char *cl_name, sdb_cl_auto_ptr &collection )
{
   int rc = SDB_ERR_OK ;
   sdb_conn_auto_ptr connection ;

   rc = get_sdb_conn( tid, connection ) ;
   if ( rc != SDB_ERR_OK )
   {
      goto error ;
   }

   rc = connection->create_cl( cs_name, cl_name, collection ) ;
   if ( rc != SDB_ERR_OK )
   {
      goto error ;
   }

done:
   return rc ;
error:
   goto done ;
}

