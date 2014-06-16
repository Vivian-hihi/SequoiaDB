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
#ifndef _SDB_PYTHON_DRIVER_COLLECTION_SPACE_HPP_
#define _SDB_PYTHON_DRIVER_COLLECTION_SPACE_HPP_

static PYOBJECT *create_cs( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *release_cs( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_collection( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *create_collection( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *create_collection_use_opt( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *drop_collection( PYOBJECT *self, PYOBJECT *args ) ;

static PYOBJECT *get_collection_space_name( PYOBJECT *self, PYOBJECT *args ) ;

#endif