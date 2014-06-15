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