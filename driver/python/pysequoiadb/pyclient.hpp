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
#ifndef _SDB_PYTHON_DRIVER_CLIENT_HPP_
#define _SDB_PYTHON_DRIVER_CLIENT_HPP_

/**
 *@brief get a object reference to sdb.
 *
 **/
static PYOBJECT *create_client( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *init_connect( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *release_client( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *connect_by_host( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *connect_by_service( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *connect_by_address( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *disconnect( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *is_connected( PYOBJECT *self, PYOBJECT *args ) ;

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

static PYOBJECT *list_collection_spaces( PYOBJECT *self, PYOBJECT *args ) ;

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

static PYOBJECT *remove_backup( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *list_tasks( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *wait_task( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *cancel_task( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *set_session_attri( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *close_all_cursors( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *is_valid( PYOBJECT *self, PYOBJECT *args ) ;

#endif