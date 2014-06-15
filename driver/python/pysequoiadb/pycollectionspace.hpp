#ifndef _SDB_PYTHON_DRIVER_COLLECTION_SPACE_HPP_
#define _SDB_PYTHON_DRIVER_COLLECTION_SPACE_HPP_

static PYOBJECT *create_cs( PYOBJECT *self ) ;

static PYOBJECT *release_cs( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_collection( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *create_collection( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *create_collection_use_opt( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *drop_collection( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_collection_space_name( PYOBJECT *self, PYOBJECT *args ) ;

#endif