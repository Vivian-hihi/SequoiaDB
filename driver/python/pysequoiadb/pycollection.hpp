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
#ifndef _SDB_PYTHON_DRIVER_COLLECTION_HPP_
#define _SDB_PYTHON_DRIVER_COLLECTION_HPP_

static PYOBJECT *create_cl( PYOBJECT *self ) ;

static PYOBJECT *release_cl( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_count( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *split_by_condition( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *split_by_precent( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *split_async_by_condition( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *splite_async_by_precent( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *bulk_insert( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *insert( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *update( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *upsert( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *del( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *query( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *create_index( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_index( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *drop_index( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_collection_name( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_collection_space_name( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_full_name( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *aggregate( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_query_meta( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *attach_collection( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *detach_collection( PYOBJECT *self, PYOBJECT *args ) ;

#endif