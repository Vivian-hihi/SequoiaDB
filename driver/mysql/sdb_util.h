#ifndef SDB_UTIL__H
#define SDB_UTIL__H

#include <mysql/psi/mysql_file.h>

int sdb_parse_table_name( const char * from,
                          char *db_name, int db_name_size,
                          char *table_name, int table_name_size ) ;

class sdb_rw_lock_r
{
private:
   mysql_rwlock_t*      rw_mutex ;

public:
   sdb_rw_lock_r( mysql_rwlock_t *var_lock )
      :rw_mutex(NULL)
   {
      if ( var_lock )
      {
         rw_mutex = var_lock ;
         mysql_rwlock_rdlock( rw_mutex ) ;
      }
   }

   ~sdb_rw_lock_r()
   {
      if ( rw_mutex )
      {
         mysql_rwlock_unlock( rw_mutex ) ;
      }
   }
};

class sdb_rw_lock_w
{
private:
   mysql_rwlock_t*      rw_mutex ;

public:
   sdb_rw_lock_w( mysql_rwlock_t *var_lock )
   {
      if ( var_lock )
      {
         rw_mutex = var_lock ;
         mysql_rwlock_wrlock( this->rw_mutex ) ;
      }
   }

   ~sdb_rw_lock_w()
   {
      if ( rw_mutex )
      {
         mysql_rwlock_unlock( this->rw_mutex ) ;
      }
   }
};

#endif
