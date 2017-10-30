/* Copyright (c) 2003, 2011, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <map>
#include <mysql/psi/mysql_thread.h>
#include <include/client.hpp>

class sdb_conn
{
public:

   sdb_conn( my_thread_id _tid ) ;

   ~sdb_conn() ;

   int connect() ;

   sdbclient::sdb & get_sdb() ;

   my_thread_id get_tid() ;

   int begin_transaction() ;

   int commit_transaction() ;

   int rollback_transaction() ;

   bool is_transaction() ;

private:
   sdbclient::sdb                   connection ;
   bool                             transactionon ;
   my_thread_id                     tid ;
} ;

class sdb_conn_auto_ptr ;
class sdb_conn_ref_ptr
{
public:

   friend class sdb_conn_auto_ptr ;

protected:

   sdb_conn_ref_ptr( sdb_conn *connection ) ;

   virtual ~sdb_conn_ref_ptr() ;

protected:
   sdb_conn                         *sdb_connection ;
   long                             ref ;    // It is not need to use atomic variable
                                             // because there is only one thread access
                                             // the same sdb-instance
} ;

class sdb_conn_auto_ptr
{
public:

   sdb_conn_auto_ptr() ;

   virtual ~sdb_conn_auto_ptr() ;

   sdb_conn_auto_ptr( sdb_conn *connection ) ;

   sdb_conn_auto_ptr( const sdb_conn_auto_ptr &other ) ;

   sdb_conn_auto_ptr & operator = ( sdb_conn_auto_ptr &other ) ;

   sdb_conn& operator *() ;

   sdb_conn* operator ->() ;

private:
   sdb_conn_ref_ptr                 *ref_ptr ;
} ;

class sdb_conn_mgr
{
public:

   ~sdb_conn_mgr() ;

   static sdb_conn_mgr *get_instance() ;

   int get_sdb_conn( my_thread_id tid, sdb_conn_auto_ptr &sdb_ptr ) ;

   void del_sdb_conn( my_thread_id tid ) ;

private:

   sdb_conn_mgr() ;

   sdb_conn_mgr(const sdb_conn_mgr & rh){}

   sdb_conn_mgr & operator = (const sdb_conn_mgr & rh) { return *this ;}

private:
   std::map<my_thread_id, sdb_conn_auto_ptr>    conn_list ;
   PSI_rwlock_key                               rw_key ;
   mysql_rwlock_t                               rw_mutex ;
} ;

#define SDB_CONN_MGR_INST           sdb_conn_mgr::get_instance()