
#ifndef MYSQL_SERVER
   #define MYSQL_SERVER
#endif

#include "sql_class.h"
#include "sdb_conn.h"
#include "sdb_conf.h"
#include "sdb_util.h"
#include <include/client.hpp>
#include <mysql/service_security_context.h>


sdb_conn::sdb_conn( my_thread_id _tid )
:transactionon(false),
tid(_tid)
{
}

sdb_conn::~sdb_conn()
{
}

sdbclient::sdb & sdb_conn::get_sdb()
{
   return connection ;
}

my_thread_id sdb_conn::get_tid()
{
   return tid ;
}

int sdb_conn::connect()
{
   int rc = 0 ;
   if ( !connection.isValid() )
   {
      transactionon = false ;
      MYSQL_SECURITY_CONTEXT ctx = current_thd->security_context();
      rc = connection.connect( (const CHAR **)(SDB_CONF_INST->get_coord_addrs()),
                               SDB_CONF_INST->get_coord_num(),
                               "", "" ) ; 
   }
   return rc ;
}

int sdb_conn::begin_transaction()
{
   int rc = 0 ;
   if ( !transactionon )
   {
      rc = connection.transactionBegin() ;
      if ( 0 == rc )
      {
         transactionon = true ;
      }
   }
   return rc ;
}

int sdb_conn::commit_transaction()
{
   if ( transactionon )
   {
      transactionon = false ;
      return connection.transactionCommit() ;
   }
   return 0 ;
}

int sdb_conn::rollback_transaction()
{
   if ( transactionon )
   {
      transactionon = false ;
      connection.transactionRollback() ;
   }
   return 0 ;
}


sdb_conn_ref_ptr::sdb_conn_ref_ptr( sdb_conn * connection )
{
   DBUG_ASSERT( connection != NULL ) ;
   sdb_connection = connection ;
   ref = 1 ;
}

sdb_conn_ref_ptr::~sdb_conn_ref_ptr()
{
   if ( sdb_connection )
   {
      delete sdb_connection ;
      sdb_connection = NULL ;
   }
   ref = 0 ;
}


sdb_conn_auto_ptr::sdb_conn_auto_ptr()
:ref_ptr( NULL )
{
}

sdb_conn_auto_ptr::sdb_conn_auto_ptr( sdb_conn *connection )
{
   ref_ptr = new sdb_conn_ref_ptr( connection ) ;
}

sdb_conn_auto_ptr::sdb_conn_auto_ptr( const sdb_conn_auto_ptr &other )
{
   this->ref_ptr = other.ref_ptr ;
   if ( ref_ptr )
   {
      ++(ref_ptr->ref) ;
   }
}

sdb_conn_auto_ptr::~sdb_conn_auto_ptr()
{
   if ( NULL == ref_ptr )
   {
      goto done ;
   }

   --(ref_ptr->ref) ;

   if ( 1 == ref_ptr->ref )
   {
      // there is no table-handler use sdb-instance,
      // only one in conn_list which in sdb_conn_mgr,
      // then delete it from conn_list.
      if ( NULL != ref_ptr->sdb_connection )
      {
         SDB_CONN_MGR_INST->del_sdb_conn( ref_ptr->sdb_connection->get_tid() ) ;
      }
   }
   else if ( 0 == ref_ptr->ref )
   {
      delete ref_ptr ;
      ref_ptr = NULL ;
   }
done:
   return ;
}

sdb_conn_auto_ptr & sdb_conn_auto_ptr::operator = ( sdb_conn_auto_ptr &other )
{
   this->ref_ptr = other.ref_ptr ;
   DBUG_ASSERT( ref_ptr != NULL ) ;
   DBUG_ASSERT( ref_ptr->sdb_connection != NULL ) ;
   ++(ref_ptr->ref) ;
   return *this ;
}

sdb_conn & sdb_conn_auto_ptr::operator *()
{
   return *ref_ptr->sdb_connection ;
}

sdb_conn * sdb_conn_auto_ptr::operator ->()
{
   return ref_ptr->sdb_connection ;
}


sdb_conn_mgr::sdb_conn_mgr()
{
   mysql_rwlock_init( rw_key, &rw_mutex ) ;
}

sdb_conn_mgr::~sdb_conn_mgr()
{
   mysql_rwlock_destroy( &rw_mutex ) ;
}

sdb_conn_mgr *sdb_conn_mgr::get_instance()
{
   static sdb_conn_mgr _sdb_conn_mgr ;
   return &_sdb_conn_mgr ;
}

int sdb_conn_mgr::get_sdb_conn( my_thread_id tid, sdb_conn_auto_ptr &sdb_ptr )
{
   int rc = 0 ;
   std::map<my_thread_id, sdb_conn_auto_ptr>::iterator iter ;

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

void sdb_conn_mgr::del_sdb_conn( my_thread_id tid )
{
   sdb_rw_lock_w w_lock( &rw_mutex ) ;
   conn_list.erase( tid );
}
